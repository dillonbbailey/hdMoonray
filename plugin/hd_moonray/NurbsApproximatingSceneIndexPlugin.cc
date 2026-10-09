// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// Converts NURBS curves and patches (UsdGeomNurbsCurves, NurbsPatch) to basis
// curves and meshes for the Moonray render delegate, which has no native rprims
// for them. USD's legacy imaging (UsdImagingDelegate) did this itself; with
// scene-index based imaging (Maya's Hydra viewport) nothing does, and such prims
// render nothing. Registered for the "Moonray" renderer like
// ImplicitSurfaceSceneIndexPlugin.cc.

#include <pxr/imaging/hd/sceneIndexPlugin.h>
#include <pxr/imaging/hd/sceneIndexPluginRegistry.h>
#include <pxr/imaging/hdsi/nurbsApproximatingSceneIndex.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdMoonrayNurbsApproximatingSceneIndexPlugin : public HdSceneIndexPlugin {
public:
    HdMoonrayNurbsApproximatingSceneIndexPlugin() = default;

protected:
    HdSceneIndexBaseRefPtr _AppendSceneIndex(const HdSceneIndexBaseRefPtr& inputScene,
                                             const HdContainerDataSourceHandle& inputArgs) override
    {
        return HdsiNurbsApproximatingSceneIndex::New(inputScene);
    }
};

TF_REGISTRY_FUNCTION(TfType)
{
    HdSceneIndexPluginRegistry::Define<HdMoonrayNurbsApproximatingSceneIndexPlugin>();
}

TF_REGISTRY_FUNCTION(HdSceneIndexPlugin)
{
    const HdSceneIndexPluginRegistry::InsertionPhase insertionPhase = 0;
    HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
        "Moonray",
        TfToken("HdMoonrayNurbsApproximatingSceneIndexPlugin"),
        nullptr, // no input args
        insertionPhase,
        HdSceneIndexPluginRegistry::InsertionOrderAtStart);
}

PXR_NAMESPACE_CLOSE_SCOPE
