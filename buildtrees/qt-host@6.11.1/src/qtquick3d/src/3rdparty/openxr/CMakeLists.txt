# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_3rdparty_library(BundledOpenXR
    QMAKE_LIB_NAME openxr_loader
    STATIC
    SKIP_AUTOMOC
    EXCEPTIONS
    INSTALL
    SOURCES
        src/loader/android_utilities.cpp
        src/loader/android_utilities.h
        src/loader/api_layer_interface.cpp
        src/loader/api_layer_interface.hpp
        src/loader/loader_core.cpp
        src/loader/loader_init_data.cpp
        src/loader/loader_init_data.hpp
        src/loader/loader_instance.cpp
        src/loader/loader_instance.hpp
        src/loader/loader_logger.cpp
        src/loader/loader_logger.hpp
        src/loader/loader_logger_recorders.cpp
        src/loader/loader_logger_recorders.hpp
        src/loader/loader_properties.cpp
        src/loader/loader_properties.hpp
        src/loader/manifest_file.cpp
        src/loader/manifest_file.hpp
        src/loader/runtime_interface.cpp
        src/loader/runtime_interface.hpp
        src/loader/xr_generated_loader.hpp
        src/loader/xr_generated_loader.cpp
        src/xr_generated_dispatch_table_core.c
        src/common/hex_and_handles.h
        src/common/android_logging.h
        src/common/object_info.cpp
        src/common/object_info.h
        src/common/platform_utils.hpp
        src/common/filesystem_utils.cpp
        src/external/jsoncpp/src/lib_json/json_reader.cpp
        src/external/jsoncpp/src/lib_json/json_value.cpp
        src/external/jsoncpp/src/lib_json/json_writer.cpp
    INCLUDE_DIRECTORIES
        src/
        src/common
        src/external/jsoncpp/include
        src/loader
    DEFINES
        JSON_USE_EXCEPTION=0
        XRLOADER_DISABLE_EXCEPTION_HANDLING
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

qt_internal_extend_target(BundledOpenXR CONDITION WIN32
    DEFINES
        XR_OS_WINDOWS
        NOMINMAX
        XR_USE_PLATFORM_WIN32
)

qt_internal_extend_target(BundledOpenXR CONDITION ANDROID
    DEFINES
        XR_OS_ANDROID
        XR_USE_PLATFORM_ANDROID
        XR_KHR_LOADER_INIT_SUPPORT=1
    SOURCES
        src/external/jnipp/jnipp.cpp
        src/external/jnipp/jnipp.h
        src/external/android-jni-wrappers/wrap/android.content.cpp
        src/external/android-jni-wrappers/wrap/android.content.h
        src/external/android-jni-wrappers/wrap/android.content.impl.h
        src/external/android-jni-wrappers/wrap/android.database.cpp
        src/external/android-jni-wrappers/wrap/android.database.h
        src/external/android-jni-wrappers/wrap/android.database.impl.h
        src/external/android-jni-wrappers/wrap/android.net.cpp
        src/external/android-jni-wrappers/wrap/android.net.h
        src/external/android-jni-wrappers/wrap/android.net.impl.h
        src/external/android-jni-wrappers/wrap/ObjectWrapperBase.h
    INCLUDE_DIRECTORIES
        src/external/android-jni-wrappers
        src/external/jnipp
    LIBRARIES
        android
        log
)

qt_internal_extend_target(BundledOpenXR CONDITION LINUX
    DEFINES
        XR_OS_LINUX
        HAVE_SECURE_GETENV
    LIBRARIES
        dl
)

qt_internal_extend_target(BundledOpenXR CONDITION QT_FEATURE_xcb
    DEFINES
        XR_USE_PLATFORM_XCB
)

qt_internal_extend_target(BundledOpenXR CONDITION APPLE
    DEFINES
        XR_OS_APPLE
)

qt_disable_warnings(BundledOpenXR)
qt_set_symbol_visibility_hidden(BundledOpenXR)
