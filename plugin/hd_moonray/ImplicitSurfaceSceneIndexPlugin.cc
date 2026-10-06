// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

// Converts implicit surfaces (UsdGeomSphere, Cube, Cone, Cylinder, Capsule,
// Plane) to meshes for the Moonray render delegate, which has no native rprims
// for them. With scene-index based imaging (the default in newer USD, and how
// Maya's Hydra viewport works) nothing else does this, and such prims render
// nothing. Registered for the "Moonray" renderer (plugInfo.json
// "loadWithRenderer" + RegisterSceneIndexForRenderer below), the same way other
// delegates (e.g. Arnold, hdPrman) do it.

#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/sceneIndexPlugin.h>
#include <pxr/imaging/hd/sceneIndexPluginRegistry.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hdsi/implicitSurfaceSceneIndex.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdMoonrayImplicitSurfaceSceneIndexPlugin : public HdSceneIndexPlugin {
public:
    HdMoonrayImplicitSurfaceSceneIndexPlugin() = default;

protected:
    HdSceneIndexBaseRefPtr _AppendSceneIndex(const HdSceneIndexBaseRefPtr& inputScene,
                                             const HdContainerDataSourceHandle& inputArgs) override
    {
        HdDataSourceBaseHandle const toMesh =
            HdRetainedTypedSampledDataSource<TfToken>::New(HdsiImplicitSurfaceSceneIndexTokens->toMesh);

        HdContainerDataSourceHandle const args = HdRetainedContainerDataSource::New(
            HdPrimTypeTokens->sphere, toMesh,
            HdPrimTypeTokens->cube, toMesh,
            HdPrimTypeTokens->cone, toMesh,
            HdPrimTypeTokens->cylinder, toMesh,
            HdPrimTypeTokens->capsule, toMesh,
            HdPrimTypeTokens->plane, toMesh);

        return HdsiImplicitSurfaceSceneIndex::New(inputScene, args);
    }
};

TF_REGISTRY_FUNCTION(TfType)
{
    HdSceneIndexPluginRegistry::Define<HdMoonrayImplicitSurfaceSceneIndexPlugin>();
}

// "loadWithRenderer" in plugInfo.json only makes the registry load this library
// when the Moonray renderer is used; the scene index must also be registered for
// the renderer here, or it is never appended.
TF_REGISTRY_FUNCTION(HdSceneIndexPlugin)
{
    const HdSceneIndexPluginRegistry::InsertionPhase insertionPhase = 0;
    HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
        "Moonray",
        TfToken("HdMoonrayImplicitSurfaceSceneIndexPlugin"),
        nullptr, // no input args
        insertionPhase,
        HdSceneIndexPluginRegistry::InsertionOrderAtStart);
}

PXR_NAMESPACE_CLOSE_SCOPE
