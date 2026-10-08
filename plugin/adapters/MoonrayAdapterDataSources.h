// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Scene index support shared by the MoonRay light adapters (USD 24.08+).
//
// With scene-index imaging, hdMoonray's Light / LightFilter sprims read their
// parameters with HdSceneDelegate::GetLightParamValue(), which the scene index
// adapter answers from the prim's "light" container. USD's light data sources
// only put UsdLux schema inputs there, so MoonRay's own "moonray:*" attributes
// (moonray:class, moonray:intensity, ...) and the targets of the relationships
// the delegate needs were missing. This builds a "light" container holding both,
// to overlay on the adapter's base data.

#include <pxr/pxr.h>

#if PXR_VERSION >= 2408

#include <pxr/base/tf/stringUtils.h>
#include <pxr/imaging/hd/lightSchema.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/relationship.h>
#include <pxr/usdImaging/usdImaging/dataSourceAttribute.h>
#include <pxr/usdImaging/usdImaging/dataSourceStageGlobals.h>

PXR_NAMESPACE_OPEN_SCOPE

namespace moonray_adapters {

// relationships: (name, whether the delegate expects all targets or the first).
inline HdContainerDataSourceHandle
overlayMoonrayLightParams(const HdContainerDataSourceHandle& base,
                          const UsdPrim& prim,
                          const UsdImagingDataSourceStageGlobals& stageGlobals,
                          const std::vector<std::pair<TfToken, bool>>& relationships)
{
    TfTokenVector names;
    std::vector<HdDataSourceBaseHandle> values;

    for (const UsdAttribute& attr : prim.GetAttributes()) {
        if (TfStringStartsWith(attr.GetName().GetString(), "moonray:") && attr.HasAuthoredValue()) {
            names.push_back(attr.GetName());
            values.push_back(UsdImagingDataSourceAttributeNew(attr, stageGlobals));
        }
    }
    for (const auto& [name, allTargets] : relationships) {
        UsdRelationship rel = prim.GetRelationship(name);
        SdfPathVector targets;
        if (!rel || !rel.GetForwardedTargets(&targets) || targets.empty()) {
            continue;
        }
        names.push_back(name);
        if (allTargets) {
            values.push_back(HdRetainedTypedSampledDataSource<SdfPathVector>::New(targets));
        } else {
            values.push_back(HdRetainedTypedSampledDataSource<SdfPath>::New(targets.front()));
        }
    }
    if (names.empty()) {
        return base;
    }
    return HdOverlayContainerDataSource::New(
        HdRetainedContainerDataSource::New(
            HdLightSchema::GetSchemaToken(),
            HdRetainedContainerDataSource::New(names.size(), names.data(), values.data())),
        base);
}

// Locators to invalidate when moonray:* attributes or the relationships change.
inline HdDataSourceLocatorSet
moonrayLightParamLocators(const TfTokenVector& properties,
                          const std::vector<std::pair<TfToken, bool>>& relationships)
{
    HdDataSourceLocatorSet result;
    for (const TfToken& property : properties) {
        bool ours = TfStringStartsWith(property.GetString(), "moonray:");
        for (const auto& rel : relationships) {
            ours = ours || property == rel.first;
        }
        if (ours) {
            result.insert(HdLightSchema::GetDefaultLocator().Append(property));
        }
    }
    return result;
}

} // namespace moonray_adapters

PXR_NAMESPACE_CLOSE_SCOPE

#endif
