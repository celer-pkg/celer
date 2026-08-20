# Lightmap baking, which depends on Embree, is supported on:
#   Windows on x86_64
#   macOS on x86_64 and arm64
#   Linux on x86_64 and arm64
# because it's restricted to what Embree builds on in practice.
if((WIN32 AND (TEST_architecture_arch STREQUAL x86_64))
        OR (MACOS AND ((TEST_architecture_arch STREQUAL x86_64) OR (TEST_architecture_arch STREQUAL arm64)))
        OR (LINUX AND ((TEST_architecture_arch STREQUAL x86_64) OR (TEST_architecture_arch STREQUAL arm64))))
    add_subdirectory(embree)
    qt_install_3rdparty_library_wrap_config_extra_file(BundledEmbree)
    set(QT_QUICK3D_HAS_EMBREE 1 PARENT_SCOPE)
endif()

if (QT_FEATURE_quick3dxr_openxr AND NOT QT_FEATURE_system_openxr)
    add_subdirectory(openxr)
endif ()
qt_install_3rdparty_library_wrap_config_extra_file(BundledOpenXR)
