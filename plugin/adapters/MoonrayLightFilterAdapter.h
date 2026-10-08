// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "pxr/pxr.h"

#include "pxr/usdImaging/usdImaging/lightFilterAdapter.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE


class UsdPrim;

class MoonrayLightFilterAdapter : public UsdImagingLightFilterAdapter {
public:
    typedef UsdImagingLightFilterAdapter BaseAdapter;

    MoonrayLightFilterAdapter()
        : UsdImagingLightFilterAdapter()
    {
    }

    virtual ~MoonrayLightFilterAdapter();

    virtual VtValue Get(UsdPrim const& prim,
                        SdfPath const& cachePath,
                        TfToken const& key,
                        UsdTimeCode time, 
                        VtIntArray* outIndices) const;

#if PXR_VERSION >= 2408
    // Scene index support: UsdImagingLightFilterAdapter supplies the filter;
    // this exposes the "rel" properties the delegate reads with Get() as
    // top-level path data sources.
    HdContainerDataSourceHandle GetImagingSubprimData(
        UsdPrim const& prim, TfToken const& subprim,
        const UsdImagingDataSourceStageGlobals& stageGlobals) override;
    HdDataSourceLocatorSet InvalidateImagingSubprim(
        UsdPrim const& prim, TfToken const& subprim, TfTokenVector const& properties,
        UsdImagingPropertyInvalidationType invalidationType) override;
#endif
};

PXR_NAMESPACE_CLOSE_SCOPE
