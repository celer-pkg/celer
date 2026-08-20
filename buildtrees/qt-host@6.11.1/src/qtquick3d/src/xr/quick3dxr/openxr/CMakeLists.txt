qt_find_package(WrapOpenXR PROVIDED_TARGETS WrapOpenXR::WrapOpenXR)
target_link_libraries(Quick3DXr PUBLIC WrapOpenXR::WrapOpenXR)
qt_find_package(EGL PROVIDED_TARGETS EGL::EGL)

qt_internal_extend_target(Quick3DXr
    SOURCES
        qopenxrplatform_p.h
        qquick3dxrmanager_openxr.cpp qquick3dxrmanager_openxr_p.h
        qabstractopenxrgraphics.cpp qabstractopenxrgraphics_p.h
        qopenxrinputmanager.cpp qopenxrinputmanager_p.h
        qquick3dxranchormanager_openxr.cpp qquick3dxranchormanager_openxr_p.h
        qopenxrhelpers.cpp qopenxrhelpers_p.h
)

# Backends

# Win32 / DX11
qt_internal_extend_target(Quick3DXr CONDITION WIN32
    SOURCES
        qopenxrgraphics_d3d11.cpp qopenxrgraphics_d3d11_p.h
        qopenxrgraphics_d3d12.cpp qopenxrgraphics_d3d12_p.h
    DEFINES
        XR_USE_PLATFORM_WIN32
        XR_USE_GRAPHICS_API_D3D11
        XR_USE_GRAPHICS_API_D3D12
    LIBRARIES
        user32
)

# Linux / glx
qt_internal_extend_target(Quick3DXr CONDITION QT_FEATURE_xcb AND QT_FEATURE_xcb_glx_plugin
    DEFINES
        XR_USE_PLATFORM_XLIB
        XR_USE_PLATFORM_XCB
)

# Linux / wayland
qt_internal_extend_target(Quick3DXr CONDITION LINUX AND QT_FEATURE_wayland
    DEFINES
        XR_USE_PLATFORM_WAYLAND
)

# Linux / egl
qt_internal_extend_target(Quick3DXr CONDITION LINUX AND QT_FEATURE_egl AND (QT_FEATURE_opengles2 OR QT_FEATURE_opengl)
    DEFINES
        XR_USE_PLATFORM_EGL
    LIBRARIES
        EGL::EGL
)

# OpenGL

qt_internal_extend_target(Quick3DXr CONDITION QT_FEATURE_opengl AND NOT QT_FEATURE_opengles2 AND NOT MACOS
    SOURCES
        qopenxrgraphics_opengl.cpp qopenxrgraphics_opengl_p.h
    DEFINES
        XR_USE_GRAPHICS_API_OPENGL
)

qt_internal_extend_target(Quick3DXr CONDITION ANDROID AND QT_FEATURE_opengles2
    SOURCES
        qopenxrgraphics_opengles.cpp qopenxrgraphics_opengles_p.h
    DEFINES
        XR_USE_GRAPHICS_API_OPENGL_ES
)

# Vulkan
qt_internal_extend_target(Quick3DXr CONDITION QT_FEATURE_vulkan
    SOURCES
        qopenxrgraphics_vulkan.cpp qopenxrgraphics_vulkan_p.h
    DEFINES
        XR_USE_GRAPHICS_API_VULKAN
)

qt_internal_extend_target(Quick3DXr CONDITION ANDROID
     DEFINES
        XR_USE_PLATFORM_ANDROID
)

if (MACOS)
    enable_language(OBJCXX)
endif ()

# macOS AND Metal && OpenXR 1.1.40 or later
qt_internal_extend_target(Quick3DXr CONDITION MACOS AND QT_FEATURE_metal
    SOURCES
        qopenxrgraphics_metal.mm qopenxrgraphics_metal_p.h
    DEFINES
        XR_USE_GRAPHICS_API_METAL
    LIBRARIES
        ${FWFoundation}
        ${FWQuartzCore}
        ${FWMetal}
)
