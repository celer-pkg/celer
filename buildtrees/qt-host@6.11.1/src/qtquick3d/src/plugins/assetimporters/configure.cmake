# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs

# input quick3d-assimp
set(INPUT_quick3d_assimp "undefined" CACHE STRING "")
set_property(CACHE INPUT_quick3d_assimp PROPERTY STRINGS undefined no qt system)



#### Libraries

qt_find_package(WrapQuick3DAssimp PROVIDED_TARGETS WrapQuick3DAssimp::WrapQuick3DAssimp
    MODULE_NAME assetimporters
    QMAKE_LIB quick3d_assimp
    VCPKG_PORT assimp
    VCPKG_ADD_TO_FEATURE quick3d-assimp
)
set(assimp_minimum_version 5.1.6)
if (assimp_VERSION AND assimp_VERSION VERSION_LESS assimp_minimum_version)
    message(FATAL_ERROR "Assimp version ${assimp_VERSION} is not supported, please use version ${assimp_minimum_version} or later")
endif()

# Work around QTBUG-115064
# Assimp depends on draco_X, but only one of the two targets gets promoted by qt_find_package
# by default, and when qt3d find_package(draco)s again, CMake complains that only one target is
# available. Promote both targets to global.
if(NOT QT_CONFIGURE_RUNNING AND TARGET assimp::assimp)
    foreach(_assimp_draco_target draco_static draco_shared)
        if(TARGET ${_assimp_draco_target})
            get_property(_assimp_draco_is_global
                TARGET ${_assimp_draco_target} PROPERTY IMPORTED_GLOBAL)
            if(NOT _assimp_draco_is_global)
                set_property(TARGET ${_assimp_draco_target} PROPERTY IMPORTED_GLOBAL TRUE)
            endif()
         endif()
    endforeach()
endif()

qt_config_compile_test("quick3d_assimp"
                   LABEL "Assimp"
                   PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.tests/quick3d_assimp"
                   LIBRARIES WrapQuick3DAssimp::WrapQuick3DAssimp
                   PACKAGES PACKAGE WrapQuick3DAssimp)


#### Tests



#### Features

qt_feature("quick3d-assimp" PUBLIC PRIVATE
    LABEL "Assimp"
)
qt_feature_definition("quick3d-assimp" "QT_NO_QUICK3D_ASSIMP" NEGATE VALUE "1")
qt_feature("system-assimp" PRIVATE SYSTEM_LIBRARY
    LABEL "System Assimp"
    CONDITION QT_FEATURE_quick3d_assimp AND TEST_quick3d_assimp
    ENABLE INPUT_quick3d_assimp STREQUAL 'system'
    DISABLE INPUT_quick3d_assimp STREQUAL 'qt'
)
qt_configure_add_summary_section(NAME "QtQuick3D")
qt_configure_add_summary_entry(ARGS "quick3d-assimp")
qt_configure_add_summary_entry(ARGS "system-assimp")
qt_configure_end_summary_section() # end of "QtQuick3D" section
