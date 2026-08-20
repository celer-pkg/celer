# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
cmake_minimum_required(VERSION 3.16)

#! [0]
set(proto_files "${CMAKE_CURRENT_LIST_DIR}/../proto/clientguide.proto")

find_package(Qt6 COMPONENTS Protobuf Grpc)
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(clientguide_client main.cpp interceptors.cpp)

# Using the executable as input target will append the generated files to it.
qt_add_protobuf(clientguide_client
    PROTO_FILES ${proto_files}
)
qt_add_grpc(clientguide_client CLIENT
    PROTO_FILES ${proto_files}
)

target_link_libraries(clientguide_client PRIVATE Qt6::Protobuf Qt6::Grpc)
#! [0]

target_compile_definitions(clientguide_client PRIVATE
    SERVER_FILE_NAME="$<TARGET_FILE_NAME:clientguide_server>")

install(TARGETS clientguide_client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
