// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "RndrRenderer.h"
#include <hydramoonray/NullRenderer.h>
#include <hydramoonray/RenderDelegate.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#if __has_include(<pxr/imaging/hd/rendererCreateArgs.h>)
#include <pxr/imaging/hd/rendererCreateArgs.h>
#define HDMOONRAY_HAS_RENDERER_CREATE_ARGS 1
#else
#define HDMOONRAY_HAS_RENDERER_CREATE_ARGS 0
#endif

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE // this does not work unless inside the pxr namespace

class HdMoonrayRendererDebugPlugin final : public pxr::HdRendererPlugin {
public:
    HdMoonrayRendererDebugPlugin() {}

    pxr::HdRenderDelegate *CreateRenderDelegate() override {
        return new hdMoonray::RenderDelegate(new hdMoonray::RndrRenderer(0));
    }

    pxr::HdRenderDelegate *CreateRenderDelegate(pxr::HdRenderSettingsMap const& settings) override {
        // "disableRender" and "threads" settings can only be specified at creation time
        // via this constructor
        auto it = settings.find(pxr::TfToken("disableRender"));
        if (it != settings.end()) {
            if (it->second.Get<bool>()) {
                auto rd = new hdMoonray::RenderDelegate(new hdMoonray::NullRenderer(),settings);
                rd->setDisableRender(true);
                return rd;
            }
        }

        uint32_t threads = 0;
        it = settings.find(pxr::TfToken("threads"));
        if (it != settings.end()) {
            threads = it->second.Get<int>();
        }
        return new hdMoonray::RenderDelegate(new hdMoonray::RndrRenderer(threads),settings);
    }

    void DeleteRenderDelegate(pxr::HdRenderDelegate *renderDelegate) override {
        delete renderDelegate;
    }

#if HDMOONRAY_HAS_RENDERER_CREATE_ARGS
    // Newer USD (e.g. 25.11) makes this overload pure virtual and deprecates
    // the bool one below.
    bool IsSupported(pxr::HdRendererCreateArgs const&,
                     std::string* reasonWhyNot = nullptr) const override {
        return true;
    }
#endif
#if PXR_VERSION >= 2302
    bool IsSupported(bool gpuEnabled = true) const override {
        return true;
    }
#else
bool IsSupported() const override {
        return true;
    }
#endif

private:
    // uncopyable
    HdMoonrayRendererDebugPlugin(const HdMoonrayRendererDebugPlugin&)             = delete;
    HdMoonrayRendererDebugPlugin &operator =(const HdMoonrayRendererDebugPlugin&) = delete;
};

TF_REGISTRY_FUNCTION(TfType)
{
    pxr::HdRendererPluginRegistry::Define<HdMoonrayRendererDebugPlugin>();
}

PXR_NAMESPACE_CLOSE_SCOPE
