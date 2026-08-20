# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_BUILD_STANDALONE_TESTS)
    # Gui is need to build and run tests on Android Platforms.
    find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Protobuf Grpc)
    find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui Quick QuickTest)
endif()

# Find OpenSSL first since it's a transitive dependency that may not be available in CONFIG mode
find_package(OpenSSL QUIET)

set(_qt_grpc_backup_prefer_config ${CMAKE_FIND_PACKAGE_PREFER_CONFIG})
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Protobuf QUIET)
find_package(gRPC QUIET)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ${_qt_grpc_backup_prefer_config})
unset(_qt_grpc_backup_prefer_config)

set(QT_BUILD_TESTS_BATCHED FALSE)
qt_build_tests()
