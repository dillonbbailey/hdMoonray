// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "pxr/pxr.h"
#include "pxr/usdImaging/usdImaging/api.h"
#include "pxr/usdImaging/usdImaging/lightAdapter.h"

PXR_NAMESPACE_OPEN_SCOPE


class UsdPrim;

class MoonrayMeshLightAdapter : public UsdImagingLightAdapter {
public:
    typedef UsdImagingLightAdapter BaseAdapter;

    MoonrayMeshLightAdapter()
        : UsdImagingLightAdapter()
    {}
    
    USDIMAGING_API
    virtual ~MoonrayMeshLightAdapter();
    
    USDIMAGING_API
    virtual SdfPath Populate(UsdPrim const& prim,
                     UsdImagingIndexProxy* index,
                     UsdImagingInstancerContext const* instancerContext = NULL);
    USDIMAGING_API
    virtual bool IsSupported(UsdImagingIndexProxy const* index) const;

    // in 0.20.11+ we can re-implement Get in the adapter to return
    // the path value of "rel geometry"
    virtual VtValue Get(UsdPrim const& prim,
                        SdfPath const& cachePath,
                        TfToken const& key,
                        UsdTimeCode time, 
                        VtIntArray*
) const;
#if PXR_VERSION >= 2408
    // Scene index support (USD 25.x scene-index imaging, e.g. Maya's Hydra
    // viewport). UsdImagingLightAdapter supplies the light data; this adds the
    // sprim type and exposes "rel inputs:geometry" as a path data source.
    TfTokenVector GetImagingSubprims(UsdPrim const& prim) override;
    TfToken GetImagingSubprimType(UsdPrim const& prim, TfToken const& subprim) override;
    HdContainerDataSourceHandle GetImagingSubprimData(
        UsdPrim const& prim, TfToken const& subprim,
        const UsdImagingDataSourceStageGlobals& stageGlobals) override;
    HdDataSourceLocatorSet InvalidateImagingSubprim(
        UsdPrim const& prim, TfToken const& subprim, TfTokenVector const& properties,
        UsdImagingPropertyInvalidationType invalidationType) override;
#endif

protected:
    virtual void _RemovePrim(SdfPath const& cachePath,
                             UsdImagingIndexProxy* index) final;

};

PXR_NAMESPACE_CLOSE_SCOPE
