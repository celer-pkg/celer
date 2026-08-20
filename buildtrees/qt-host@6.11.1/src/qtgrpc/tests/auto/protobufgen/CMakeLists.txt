# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qtprotobufgen LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
    include("${CMAKE_CURRENT_LIST_DIR}/../protocplugintestcommon/CMakeLists.txt")
endif()

get_target_property(protoc_version WrapProtoc::WrapProtoc _qt_internal_protobuf_version)

set(cmake_generated_dir "cmake_generated")
qt_internal_add_test(tst_qtprotobufgen
    SOURCES
        tst_qtprotobufgen.cpp
    LIBRARIES
        protocplugintestcommon
        Qt::Test
        Qt::Protobuf
    DEFINES
        PROTOC_EXECUTABLE="$<TARGET_FILE:WrapProtoc::WrapProtoc>"
        PROTOC_PLUGIN="$<TARGET_FILE:Qt6::qtprotobufgen>"
        CMAKE_GENERATED_DIR="${cmake_generated_dir}"
        BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

qt_internal_extend_target(tst_qtprotobufgen
    CONDITION
        CMAKE_CROSSCOMPILING
    DEFINES
        CROSSCOMPILING
)

qt_autogen_tools_initial_setup(tst_qtprotobufgen)

set(cmake_generator_tests "")
qt_add_protobuf(tst_qtprotobufgen_no_options
    PROTO_FILES
        data/qtprotobufgen.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/no_options"
)
list(APPEND cmake_generator_tests no_options)

qt_add_protobuf(tst_qtprotobufgen_copy_comments
    PROTO_FILES
        data/annotation.proto
    COPY_COMMENTS
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/copy_comments"
)
list(APPEND cmake_generator_tests copy_comments)

qt_add_protobuf(tst_qtprotobufgen_extra_namespace
    PROTO_FILES
        data/qtprotobufgen.proto
    EXTRA_NAMESPACE
        MyTopLevelNamespace
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/extra_namespace"
)
list(APPEND cmake_generator_tests extra_namespace)

qt_add_protobuf(tst_qtprotobufgen_generate_package_subfolders
    PROTO_FILES
        data/qtprotobufgen.proto
    GENERATE_PACKAGE_SUBFOLDERS
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/generate_package_subfolders"
)
list(APPEND cmake_generator_tests generate_package_subfolders)

qt_add_protobuf(tst_qtprotobufgen_export_macro
    PROTO_FILES
        data/qtprotobufgen.proto
    EXPORT_MACRO "CUSTOM_EXPORT_NAME"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/export_macro"
)
list(APPEND cmake_generator_tests export_macro)

add_library(tst_qtprotobufgen_static STATIC)
qt_add_protobuf(tst_qtprotobufgen_static
    PROTO_FILES
        data/qtprotobufgen.proto
    EXPORT_MACRO "CUSTOM_EXPORT_NAME"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/static"
)
list(APPEND cmake_generator_tests static)

add_library(tst_qtprotobufgen_non_final_messages STATIC)
qt_add_protobuf(tst_qtprotobufgen_non_final_messages
    GENERATE_NON_FINAL_MESSAGES
    PROTO_FILES
        data/qtprotobufgenminimal.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/non_final_messages"
)
list(APPEND cmake_generator_tests non_final_messages)

if(TARGET Qt6::ProtobufQuick)
    qt_internal_extend_target(tst_qtprotobufgen
        DEFINES
            HAVE_QML
    )

    qt_add_protobuf(tst_qtprotobufgen_qml
        PROTO_FILES
            data/qtprotobufgen.proto
        QML
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/qml"
    )
    list(APPEND cmake_generator_tests qml)

    qt_add_protobuf(tst_qtprotobufgen_qml_uri
        PROTO_FILES
            data/qtprotobufgen.proto
        QML
        QML_URI "nopackage.uri.test"
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${cmake_generated_dir}/qml_uri"
    )
    list(APPEND cmake_generator_tests qml_uri)
endif()

foreach(test_name IN LISTS cmake_generator_tests)
    set(target tst_qtprotobufgen_${test_name})
    if(protoc_version VERSION_GREATER_EQUAL "3.12" AND protoc_version VERSION_LESS "3.15")
        qt_internal_extend_target(${target}
            DEFINES
                ALLOW_PROTO3_OPTIONAL
        )
    endif()
    qt_autogen_tools_initial_setup(${target})
    get_target_property(qml_plugin_target ${target} QT_QML_MODULE_PLUGIN_TARGET)
    if(TARGET ${qml_plugin_target})
        qt_autogen_tools_initial_setup(${qml_plugin_target})
    endif()
    add_dependencies(tst_qtprotobufgen ${target})
endforeach()

list(JOIN cmake_generator_tests "," cmake_generator_tests_string)
qt_internal_extend_target(tst_qtprotobufgen
    DEFINES
        CMAKE_GENERATOR_TESTS="${cmake_generator_tests_string}"
)
