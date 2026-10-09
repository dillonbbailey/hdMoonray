// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "RenderSettings.h"
#include "RenderDelegate.h"
#include "ValueConverter.h"
#include "Utils.h"

#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneVariables.h>
#include <scene_rdl2/render/logging/logging.h>

#include <cctype>
#include <set>

// If adding or changing the descriptors, the file
// ../houdini/soho/parameters/HdMoonrayRendererPlugin_Viewport.ds must be updated to match

using namespace pxr;
namespace {

TF_DEFINE_PRIVATE_TOKENS(Tokens,
    (debug)
    (info)
    (logLevel)
    (rdlOutput)
    (disableLighting)
    (doubleSided)
    (decodeNormals)
    (enableMotionBlur)
    (pruneWillow)
    (pruneFurDeform)
    (pruneCurveDeform)
    (pruneVolume)
    (pruneWrapDeform)
    (forcePolygon)
    (executionMode)
);

// MoonRay scene variables offered as viewport settings - the set in the
// Houdini viewport parameters (HdMoonrayRendererPlugin_Viewport.ds). Their
// descriptors use camelCase keys ("pixelSamples"), as the other settings do:
// mayaHydra makes a Maya attribute and an option-box control per descriptor and
// labels it with the key. apply() also accepts the USD render settings form
// "moonray:sceneVariable:<name>" and Houdini's "sceneVariable_<name>".
const std::pair<const char*, const char*> sSceneVariables[] = {
    {"sampling_mode",            "Sampling Mode"},
    {"pixel_samples",            "Pixel Samples"},
    {"min_adaptive_samples",     "Min Adaptive Samples"},
    {"max_adaptive_samples",     "Max Adaptive Samples"},
    {"target_adaptive_error",    "Target Adaptive Error"},
    {"sample_clamping_depth",    "Sample Clamping Depth"},
    {"sample_clamping_value",    "Sample Clamping Value"},
    {"max_depth",                "Max Depth"},
    {"max_diffuse_depth",        "Max Diffuse Depth"},
    {"max_glossy_depth",         "Max Glossy Depth"},
    {"max_mirror_depth",         "Max Mirror Depth"},
    {"max_hair_depth",           "Max Hair Depth"},
    {"max_presence_depth",       "Max Presence Depth"},
    {"max_volume_depth",         "Max Volume Depth"},
    {"texture_cache_size",       "Texture Cache Size"},
    {"enable_presence_shadows",  "Enable Presence Shadows"},
};

// "pixel_samples" -> "pixelSamples"
TfToken
settingKey(const std::string& sceneVariable)
{
    std::string key;
    bool upper = false;
    for (char c : sceneVariable) {
        if (c == '_') { upper = true; continue; }
        key += upper ? char(std::toupper(c)) : c;
        upper = false;
    }
    return TfToken(key);
}

}

namespace hdMoonray {

using scene_rdl2::logging::Logger;

void
RenderSettings::addDescriptors(HdRenderSettingDescriptorList& descriptorList) const
{
    static HdRenderSettingDescriptorList descriptors = {

        { "Show Debug Messages",  Tokens->debug,               VtValue(getEnv("HDMOONRAY_DEBUG", false)) },
        { "Show Info Messages",   Tokens->info,                VtValue(getEnv("HDMOONRAY_INFO", false)) },
        { "Rdla output",          Tokens->rdlOutput,           VtValue(getEnv("HDMOONRAY_RDLA_OUTPUT","")) },
        { "Disable Lighting",     Tokens->disableLighting,     VtValue(getEnv("HDMOONRAY_DISABLE_LIGHTING", false)) },
        { "DoubleSided",          Tokens->doubleSided,         VtValue(getEnv("HDMOONRAY_DOUBLESIDED", false)) },
        { "Decode Normals",       Tokens->decodeNormals,       VtValue(getEnv("HDMOONRAY_DOUBLESIDED", false)) },
        { "Enable Motion Blur",   Tokens->enableMotionBlur,    VtValue(getEnv("HDMOONRAY_ENABLE_MOTION_BLUR", true)) },
        { "Prune Willow",         Tokens->pruneWillow,         VtValue(getEnv("HDMOONRAY_PRUNE_WILLOW", false)) },
        { "Prune FurDeform",      Tokens->pruneFurDeform,      VtValue(getEnv("HDMOONRAY_PRUNE_FURDEFORM", false)) },
        { "Prune Volumes",        Tokens->pruneVolume,         VtValue(getEnv("HDMOONRAY_PRUNE_VOLUME", false)) },
        { "Prune WrapDeform",     Tokens->pruneWrapDeform,     VtValue(getEnv("HDMOONRAY_PRUNE_WRAPDEFORM", false)) },
        { "Prune CurveDeform",    Tokens->pruneCurveDeform,    VtValue(getEnv("HDMOONRAY_PRUNE_CURVEDEFORM", false)) },
        { "Force Polygon",        Tokens->forcePolygon,        VtValue(getEnv("HDMOONRAY_FORCE_POLYGON", false)) },
        { "Execution Mode",       Tokens->executionMode,       VtValue(getEnv("HDMOONRAY_EXEC_MODE", "auto")) },
    };
    for (const auto& desc : descriptors) {
        descriptorList.push_back(desc);
    }

    // The scene variables above; types and defaults from MoonRay's SceneVariables
    // class, so an untouched setting changes nothing.
    using scene_rdl2::rdl2::Attribute;
    const scene_rdl2::rdl2::SceneClass& sceneClass =
        mDelegate.acquireSceneContext().getSceneVariables().getSceneClass();
    for (const auto& [name, label] : sSceneVariables) {
        const Attribute* attr = nullptr;
        try {
            attr = sceneClass.getAttribute(name);
        } catch (const std::exception&) {
            continue; // not in this MoonRay version
        }
        VtValue value;
        switch (attr->getType()) {
        case scene_rdl2::rdl2::TYPE_BOOL:
            value = VtValue(bool(attr->getDefaultValue<scene_rdl2::rdl2::Bool>())); break;
        case scene_rdl2::rdl2::TYPE_INT:
            value = VtValue(int(attr->getDefaultValue<scene_rdl2::rdl2::Int>())); break;
        case scene_rdl2::rdl2::TYPE_FLOAT:
            value = VtValue(float(attr->getDefaultValue<scene_rdl2::rdl2::Float>())); break;
        case scene_rdl2::rdl2::TYPE_STRING:
            value = VtValue(attr->getDefaultValue<scene_rdl2::rdl2::String>()); break;
        default:
            continue;
        }
        // Enums are plain ints in UIs built from descriptors: name the values.
        std::string text = label;
        if (attr->isEnumerable()) {
            std::string values;
            for (auto it = attr->beginEnumValues(); it != attr->endEnumValues(); ++it) {
                values += (values.empty() ? "" : ", ") + std::to_string(it->first) + " " + it->second;
            }
            text += " (" + values + ")";
        }
        descriptorList.push_back({text, settingKey(name), value});
    }
}
VtValue
RenderSettings::getRenderSetting(const TfToken& key) const
{
    return mDelegate.GetRenderSetting(key);
}

void RenderSettings::apply()
{
    bool info = get<bool>(Tokens->info);
    if (info) {
        Logger::setInfoLevel();
    }

    bool dbg = get<bool>(Tokens->debug);
    if (dbg) {
        Logger::setDebugLevel();
    }

    static const TfToken houdiniInteractive("houdini:interactive");
    VtValue val = mDelegate.GetRenderSetting(houdiniInteractive);
    mDelegate.setIsHoudini(not val.IsEmpty());

    // ---------------------------------------------------------------------------------
    // support render settings "moonray:sceneVariable:<name>" and "moonray:sceneVariable_<name>" for any
    // scene variable. The second form is used in the Houdini .ds file because it
    // doesn't like the colon character.

    // These are excluded...
    static const std::set<std::string> sDontWrite = {
        "camera", "motion_steps", "enable_motion_blur", "layer", "image_width", "image_height"
    };

    static const std::set<std::string> sViewportSceneVariables = [] {
        std::set<std::string> names;
        for (const auto& entry : sSceneVariables) names.insert(entry.first);
        return names;
    }();

    scene_rdl2::rdl2::SceneVariables& sv = mDelegate.acquireSceneContext().getSceneVariables();
    {
        UpdateGuard guard(sv);
        const SceneClass& sceneClass = sv.getSceneClass();
        for (auto it = sceneClass.beginAttributes(); it != sceneClass.endAttributes(); ++it) {

            const std::string& attrName = (*it)->getName();
            if (sDontWrite.count(attrName)) continue;

            TfToken key = TfToken("moonray:sceneVariable:" + attrName);
            VtValue val = mDelegate.GetRenderSetting(key);
            if (val.IsEmpty()) {
                key = TfToken("sceneVariable_" + attrName);
                val = mDelegate.GetRenderSetting(key);
            }
            if (val.IsEmpty() && sViewportSceneVariables.count(attrName)) {
                val = mDelegate.GetRenderSetting(settingKey(attrName));
            }
            if (not val.IsEmpty()) {
                ValueConverter::setAttribute(&sv, *it, val);
            }
        }

        // apply any overrides from the render options
        sv.set(sv.sDebugKey, dbg);
        sv.set(sv.sInfoKey, info);

    }
    mDelegate.setRdlOutput(get<std::string>(Tokens->rdlOutput));
    mDelegate.setDisableLighting(get<bool>(Tokens->disableLighting));
    mDelegate.setDoubleSided(get<bool>(Tokens->doubleSided));
    mDelegate.setDecodeNormals(get<bool>(Tokens->decodeNormals));
    mDelegate.setEnableMotionBlur(get<bool>(Tokens->enableMotionBlur));
    mDelegate.setPruneProcedural("WillowGeometry_v3", get<bool>(Tokens->pruneWillow));
    mDelegate.setPruneProcedural("FurDeformGeometry", get<bool>(Tokens->pruneFurDeform));
    mDelegate.setPruneProcedural("CurveDeformGeometry", get<bool>(Tokens->pruneCurveDeform));
    mDelegate.setPruneProcedural("WrapDeformGeometry", get<bool>(Tokens->pruneWrapDeform));
    mDelegate.setPruneVolume(get<bool>(Tokens->pruneVolume));
    mDelegate.setForcePolygon(get<bool>(Tokens->forcePolygon));
    setDeepIdAttributeName();

}


std::string
RenderSettings::getExecutionMode() const
{
    VtValue val = mDelegate.GetRenderSetting(Tokens->executionMode);
    if (not val.IsEmpty()) {
        return val.Get<std::string>();
    } else {
        return "auto";
    }
}

void
RenderSettings::setDeepIdAttributeName(){
    TfToken key = TfToken("moonray:sceneVariable:deep_id_attribute_names");
    VtValue val = mDelegate.GetRenderSetting(key);
    if (val.IsHolding<pxr::VtArray<std::string>>()) {
        pxr::VtArray<std::string> names = val.UncheckedGet<pxr::VtArray<std::string>>();
        mDelegate.setDeepIdAttrName(names.front());
    }
}

} // namespace hdMoonray
