# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json
        DESTINATION plugin/pxr)

add_subdirectory(hd_moonray)
if(HDMOONRAY_BUILD_DEBUG_PLUGIN)
    add_subdirectory(hd_moonray_debug)
endif()
if(HDMOONRAY_BUILD_HOUDINI)
    add_subdirectory(houdini)
endif()
add_subdirectory(adapters)

# Codeless USD schema for the MoonrayMeshLight / MoonrayLightFilter prim types:
# newer USD only finds an adapter for a prim type that is a registered schema.
# Derived from USD 25.11's usdLux schema, so only installed for newer USD.
if(PXR_VERSION VERSION_GREATER_EQUAL 2408)
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/schemas/plugInfo.json
        ${CMAKE_CURRENT_SOURCE_DIR}/schemas/generatedSchema.usda
        DESTINATION plugin/pxr/hdMoonraySchemas)
endif()
