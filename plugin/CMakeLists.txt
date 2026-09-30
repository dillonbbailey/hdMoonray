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
