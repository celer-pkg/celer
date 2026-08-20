# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtGrpc
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt GRPC and Protobuf generator and bindings."
    HOMEPAGE_URL "https://qt.io"
    LANGUAGES CXX C
)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED Core)

# Try to find Qt6::qtprotobufgen and Qt6::qtgrpcgen targets from host tools
# when cross-compiling.
if(NOT "${QT_HOST_PATH}" STREQUAL "")
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/tools/host")
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Network Gui Widgets Quick
    QmlNetwork QuickControls2 QuickTest)

include("${CMAKE_CURRENT_LIST_DIR}/src/tools/qtprotobufgen/Qt6ProtobufToolsMacros.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/src/tools/qtgrpcgen/Qt6GrpcToolsMacros.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/src/protobuf/Qt6ProtobufBuildInternals.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/src/wellknown/Qt6ProtobufWellKnownTypesBuildInternals.cmake")

qt_build_repo()
