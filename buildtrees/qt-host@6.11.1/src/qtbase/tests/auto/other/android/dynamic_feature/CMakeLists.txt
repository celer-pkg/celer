# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_android_dynamic_feature LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

add_subdirectory(feature)

qt_internal_add_test(tst_android_dynamic_feature
    SOURCES
        tst_android_dynamic_feature.cpp
        storeloader/storeloader.h
        storeloader/storeloader.cpp
    INCLUDE_DIRECTORIES
        storeloader
    LIBRARIES
        Qt6::Test
        Qt6::Gui
        Qt6::CorePrivate
)

set_property(TARGET tst_android_dynamic_feature PROPERTY
    QT_ANDROID_PACKAGE_NAME "org.qtproject.example.android_dynamic_feature")
set_property(TARGET tst_android_dynamic_feature APPEND PROPERTY
    _qt_android_gradle_java_source_dirs "${CMAKE_CURRENT_SOURCE_DIR}/storeloader/java")

qt6_add_android_dynamic_features(tst_android_dynamic_feature FEATURE_TARGETS
    tst_android_dynamic_feature_resources)
