# Copyright 2023-2024 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

set(component hd_moonray)

set(installIncludeDir ${PACKAGE_NAME}/${component})
set(exportGroup ${PROJECT_NAME}Targets)

add_library(${component} SHARED "")
add_library(${PROJECT_NAME}::${component} ALIAS ${component})

# no "lib" prefix on the plugin lib
set_target_properties(${component} PROPERTIES PREFIX "")

target_sources(${component}
    PRIVATE
        ArrasRenderer.cc
        ArrasSettings.cc
        RendererPlugin.cc
)

# With scene-index based imaging (the default in newer USD, and Maya's Hydra
# viewport) implicit surfaces (sphere, cube, ...) and NURBS need converting to
# meshes / basis curves for Moonray. Registered through plugInfo.json for the
# "Moonray" renderer.
set(HDMOONRAY_EXTRA_PLUGIN_TYPES "")
if(TARGET hdsi AND PXR_VERSION VERSION_GREATER_EQUAL 2408)
    target_sources(${component} PRIVATE
        ImplicitSurfaceSceneIndexPlugin.cc
        NurbsApproximatingSceneIndexPlugin.cc)
    target_link_libraries(${component} PUBLIC hdsi)
    string(CONCAT HDMOONRAY_EXTRA_PLUGIN_TYPES
        ",\n                    \"HdMoonrayImplicitSurfaceSceneIndexPlugin\": {\n"
        "                        \"bases\": [\"HdSceneIndexPlugin\"],\n"
        "                        \"loadWithRenderer\": \"Moonray\",\n"
        "                        \"priority\": 0,\n"
        "                        \"displayName\": \"Moonray implicit surfaces to meshes\"\n"
        "                    }"
        ",\n                    \"HdMoonrayNurbsApproximatingSceneIndexPlugin\": {\n"
        "                        \"bases\": [\"HdSceneIndexPlugin\"],\n"
        "                        \"loadWithRenderer\": \"Moonray\",\n"
        "                        \"priority\": 0,\n"
        "                        \"displayName\": \"Moonray NURBS to basis curves and meshes\"\n"
        "                    }")
endif()

target_include_directories(${component}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (NOT IsDarwinPlatform)
    set(PlatformSpecificLibs atomic)
endif()

target_link_libraries(${component}
    PUBLIC
        HdMoonray::hydramoonray
        ArrasCore::arras4_log
        ArrasCore::client_api
        ArrasCore::sdk
        ArrasCore::core_messages
        ArrasCore::message_api
        McrtDataio::client_receiver
        McrtMessages::mcrt_messages
        TBB::tbb
        hd   # pxr imaging
        usdImaging
        Python::Module
        ${PlatformSpecificLibs}
)

# If at Dreamworks add a SConscript stub file so others can use this library.
SConscript_Stub(${component})

# Set standard compile/link options
HdMoonray_cxx_compile_definitions(${component})
HdMoonray_cxx_compile_features(${component})
HdMoonray_cxx_compile_options(${component})
HdMoonray_link_options(${component})

# Configure plugInfo.json file
set(plugInfoTemplate ${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json.in)
set(plugInfoFile ${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json)
set(LIBRARY_PATH ../../${component}${CMAKE_SHARED_LIBRARY_SUFFIX})
configure_file(${plugInfoTemplate} ${plugInfoFile})

# -------------------------------------
# Install the target and the export set
# -------------------------------------
include(GNUInstallDirs)

# install the plugin lib
# .so is installed under plugin, so that it can
# use the RUNPATH ../lib64
install(TARGETS ${component}
    COMPONENT ${component}
    EXPORT ${exportGroup}
    LIBRARY
        DESTINATION plugin
        NAMELINK_SKIP
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${installIncludeDir}
)

# install main plugInfo
INSTALL(FILES ${plugInfoFile}
    DESTINATION plugin/pxr/${component}
)

