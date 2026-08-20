# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET WrapSystemOpenXR::WrapSystemOpenXR)
    set(WrapSystemOpenXR_FOUND ON)
    return()
endif()
set(WrapSystemOpenXR_REQUIRED_VARS __openxr_found)

if (ANDROID)
    # When using the Oculus OpenXR Mobile SDK, we need to use the loader provided by the SDK (for now)
    if (NOT "$ENV{OCULUS_OPENXR_MOBILE_SDK}" STREQUAL "")
        set(XR_LOADER_TYPE "Debug")
        if(DEFINED CMAKE_BUILD_TYPE)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(XR_LOADER_TYPE "Debug")
            elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
                set(XR_LOADER_TYPE "Release")
            elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
                set(XR_LOADER_TYPE "Release")
            endif()
        endif()
        file(TO_CMAKE_PATH $ENV{OCULUS_OPENXR_MOBILE_SDK} OCULUS_OPENXR_MOBILE_SDK)
        set(XR_LOADER_SO ${OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Libs/Android/${ANDROID_ABI}/${XR_LOADER_TYPE}/libopenxr_loader.so)
        if(EXISTS ${XR_LOADER_SO})
            message("Using OpenXR loader ${XR_LOADER_SO}")
            add_library(openxr_loader SHARED IMPORTED)
            set_property(
                TARGET
                    openxr_loader
                PROPERTY
                    IMPORTED_LOCATION
                ${XR_LOADER_SO}
            )

            add_library(WrapSystemOpenXR::WrapSystemOpenXR INTERFACE IMPORTED)
            target_link_libraries(WrapSystemOpenXR::WrapSystemOpenXR INTERFACE openxr_loader)

            # Oculus OpenXR Mobile SDK pre-v64
            if((EXISTS ${OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Include) AND (EXISTS ${OCULUS_OPENXR_MOBILE_SDK}/3rdParty/khronos/openxr/OpenXR-SDK/include))
                set(META_PREVIEW ${OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Include)
                set(OPENXR_HEADERS ${OCULUS_OPENXR_MOBILE_SDK}/3rdParty/khronos/openxr/OpenXR-SDK/include)
            # Oculus OpenXR Mobile SDK v64 and newer
            elseif((EXISTS ${OCULUS_OPENXR_MOBILE_SDK}/OpenXR/meta_openxr_preview) AND (EXISTS ${OCULUS_OPENXR_MOBILE_SDK}/Samples/3rdParty/khronos/openxr/OpenXR-SDK/include))
                set(META_PREVIEW ${OCULUS_OPENXR_MOBILE_SDK}/OpenXR/meta_openxr_preview)
                set(OPENXR_HEADERS ${OCULUS_OPENXR_MOBILE_SDK}/Samples/3rdParty/khronos/openxr/OpenXR-SDK/include)
            endif()

            target_include_directories(
                WrapSystemOpenXR::WrapSystemOpenXR INTERFACE
                ${META_PREVIEW}
                ${OPENXR_HEADERS}
            )
            set(WrapSystemOpenXR_FOUND TRUE)
            include(FindPackageHandleStandardArgs)
            find_package_handle_standard_args(WrapSystemOpenXR DEFAULT_MSG WrapSystemOpenXR_FOUND)
            return()
        else()
            message("OCULUS_OPENXR_MOBILE_SDK is set, but the proprietary loader library is not present; probably a >= v65 SDK, ignoring in favor of the bundled loader")
        endif()
    else()
        message("OCULUS_OPENXR_MOBILE_SDK is not set")
    endif()

endif ()


find_package(OpenXR ${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION} QUIET)

set(__openxr_target_name "OpenXR::openxr_loader")
if(OpenXR_FOUND AND TARGET "${__openxr_target_name}")
    set(__openxr_found TRUE)
    if(OpenXR_VERSION)
        set(WrapSystemOpenXR_VERSION "${OpenXR_VERSION}")
    endif()
endif()

if(OpenXR_LIBRARIES)
    list(PREPEND WrapSystemOpenXR_REQUIRED_VARS OpenXR_LIBRARIES)
endif()
if(OpenXR_VERSION)
    set(WrapSystemOpenXR_VERSION "${OpenXR_VERSION}")
elseif(OpenXR_VERSION_STRING)
    set(WrapSystemOpenXR_VERSION "${OpenXR_VERSION_STRING}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapSystemOpenXR
                                REQUIRED_VARS ${WrapSystemOpenXR_REQUIRED_VARS}
                                VERSION_VAR WrapSystemOpenXR_VERSION)

if(WrapSystemOpenXR_FOUND)
    add_library(WrapSystemOpenXR::WrapSystemOpenXR INTERFACE IMPORTED)
    target_link_libraries(WrapSystemOpenXR::WrapSystemOpenXR
                        INTERFACE "${__openxr_target_name}")
endif()


unset(__openxr_target_name)
unset(__openxr_found)
