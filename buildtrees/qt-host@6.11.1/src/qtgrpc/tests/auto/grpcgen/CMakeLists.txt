# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qtgrpcgen LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
    include("${CMAKE_CURRENT_LIST_DIR}/../protocplugintestcommon/CMakeLists.txt")
endif()

set(cmake_generated_dir "cmake_generated")
qt_internal_add_test(tst_qtgrpcgen
    SOURCES
        tst_qtgrpcgen.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        Qt::Grpc
        protocplugintestcommon
    DEFINES
        PROTOC_EXECUTABLE="$<TARGET_FILE:WrapProtoc::WrapProtoc>"
        PROTOC_PLUGIN="$<TARGET_FILE:Qt6::qtgrpcgen>"
        CMAKE_GENERATED_DIR="${cmake_generated_dir}"
        BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

qt_internal_extend_target(tst_qtgrpcgen
    CONDITION
        CMAKE_CROSSCOMPILING
    DEFINES
        CROSSCOMPILING
)

qt_add_protobuf(tst_qtgrpcgen_protobuf_common
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/protobuf_common"
    GENERATE_PACKAGE_SUBFOLDERS
    PROTO_FILES
        data/qtgrpcgen.proto
)
qt_autogen_tools_initial_setup(tst_qtgrpcgen_protobuf_common)

set(cmake_generator_tests "")
qt_add_grpc(tst_qtgrpcgen_generate_package_subfolders CLIENT
    PROTO_FILES
        data/qtgrpcgen.proto
    GENERATE_PACKAGE_SUBFOLDERS
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/generate_package_subfolders"
)
target_link_libraries(tst_qtgrpcgen_generate_package_subfolders
    PRIVATE tst_qtgrpcgen_protobuf_common)
list(APPEND cmake_generator_tests generate_package_subfolders)

qt_add_grpc(tst_qtgrpcgen_no_options CLIENT
    PROTO_FILES
        data/qtgrpcgen.proto
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/no_options"
)
target_link_libraries(tst_qtgrpcgen_no_options
    PRIVATE tst_qtgrpcgen_protobuf_common)
target_include_directories(tst_qtgrpcgen_no_options PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/protobuf_common/qtgrpc/tests")
list(APPEND cmake_generator_tests no_options)

qt_add_protobuf(tst_qtgrpcgen_protobuf_invalid-identifier
    PROTO_FILES
        data/666-invalid-identifier.proto
    HEADER_GUARD filename
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/invalid-identifier"
)
qt_add_grpc(tst_qtgrpcgen_invalid-identifier CLIENT
    PROTO_FILES
        data/666-invalid-identifier.proto
    HEADER_GUARD filename
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/invalid-identifier"
)
target_link_libraries(tst_qtgrpcgen_invalid-identifier
    PRIVATE tst_qtgrpcgen_protobuf_invalid-identifier)
list(APPEND cmake_generator_tests invalid-identifier protobuf_invalid-identifier)

if(TARGET Qt6::GrpcQuick)
    qt_internal_extend_target(tst_qtgrpcgen
        DEFINES
            HAVE_QML
    )

    qt_add_protobuf(tst_qtgrpcgen_qml
        PROTO_FILES
            data/qtgrpcgen.proto
        QML
        OUTPUT_DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/qml"
    )

    qt_add_grpc(tst_qtgrpcgen_qml CLIENT
        PROTO_FILES
            data/qtgrpcgen.proto
        QML
        OUTPUT_DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/qml"
    )
    list(APPEND cmake_generator_tests qml)
endif()

foreach(test_name IN LISTS cmake_generator_tests)
    set(target tst_qtgrpcgen_${test_name})
    qt_autogen_tools_initial_setup(${target})
    get_target_property(qml_plugin_target ${target} QT_QML_MODULE_PLUGIN_TARGET)
    if(TARGET ${qml_plugin_target})
        qt_autogen_tools_initial_setup(${qml_plugin_target})
    endif()
    add_dependencies(tst_qtgrpcgen ${target})
endforeach()

list(JOIN cmake_generator_tests "," cmake_generator_tests_string)
qt_internal_extend_target(tst_qtgrpcgen
    DEFINES
        CMAKE_GENERATOR_TESTS="${cmake_generator_tests_string}"
)
