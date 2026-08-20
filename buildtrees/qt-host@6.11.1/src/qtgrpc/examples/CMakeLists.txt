# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

# Pass 3rd party dependency locations to examples as external projects.
if(NOT Protobuf_ROOT AND "$ENV{Protobuf_ROOT}")
    set(Protobuf_ROOT "$ENV{Protobuf_ROOT}")
endif()

if(NOT gRPC_ROOT AND "$ENV{gRPC_ROOT}")
    set(gRPC_ROOT "$ENV{gRPC_ROOT}")
endif()

if(NOT absl_ROOT AND "$ENV{absl_ROOT}")
    set(absl_ROOT "$ENV{absl_ROOT}")
endif()

if(NOT OPENSSL_ROOT_DIR AND "$ENV{OPENSSL_ROOT_DIR}")
    set(OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}")
endif()

set(QT_EXAMPLE_CMAKE_VARS_TO_PASS Protobuf_ROOT:STRING gRPC_ROOT:STRING OPENSSL_ROOT_DIR:STRING)

# In the CI, we assume the absl dependency is next to the gRPC one.
if(gRPC_ROOT AND NOT absl_ROOT)
    get_filename_component(absl_ROOT "${gRPC_ROOT}" DIRECTORY)
    string(APPEND absl_ROOT "/absl")
    if(EXISTS "${absl_ROOT}")
        list(APPEND QT_EXAMPLE_CMAKE_VARS_TO_PASS absl_ROOT:STRING)
    endif()
endif()

qt_examples_build_begin(EXTERNAL_BUILD)

add_subdirectory(protobuf)

if(TARGET Qt6::Grpc)
    add_subdirectory(grpc)
endif()
qt_examples_build_end()
