# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_basic LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt6_add_protobuf(tst_protobuf_basictypes_gen
    PROTO_FILES
        proto/basicmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_basictypes_gen)

qt_internal_add_test(tst_protobuf_basictypes
    SOURCES
        tst_protobuf_basictypes.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
)

qt6_add_protobuf(tst_protobuf_repeatedtypes_gen
    PROTO_FILES
        proto/repeatedmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_repeatedtypes_gen)

# @todo: this explicit linking is needed, try to make it automatic since it's a dependency
# between the proto files. In the future!
target_link_libraries(tst_protobuf_repeatedtypes_gen
    PUBLIC
        tst_protobuf_basictypes_gen
)

qt_internal_add_test(tst_protobuf_repeatedtypes
    SOURCES
        tst_protobuf_repeatedtypes.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_repeatedtypes_gen
)

qt6_add_protobuf(tst_protobuf_maptypes_gen
    PROTO_FILES
        proto/mapmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
if(MINGW)
    target_compile_options(tst_protobuf_maptypes_gen PRIVATE "-Wa,-mbig-obj")
endif()

qt_autogen_tools_initial_setup(tst_protobuf_maptypes_gen)

# @todo: this explicit linking is needed, try to make it automatic since it's a dependency
# between the proto files.
target_link_libraries(tst_protobuf_maptypes_gen
    PUBLIC
        tst_protobuf_basictypes_gen
)

qt_internal_add_test(tst_protobuf_maptypes
    SOURCES
        tst_protobuf_maptypes.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_maptypes_gen
)

qt6_add_protobuf(tst_protobuf_fieldindexrange_gen
    PROTO_FILES
        proto/fieldindexrange.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_fieldindexrange_gen)

qt_internal_add_test(tst_protobuf_serialization_basictypes
    SOURCES
        tst_protobuf_serialization_basictypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_fieldindexrange_gen
)

qt_internal_add_test(tst_protobuf_serialization_repeatedtypes
    SOURCES
        tst_protobuf_serialization_repeatedtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_repeatedtypes_gen
)

qt_internal_add_test(tst_protobuf_serialization_maptypes
    SOURCES
        tst_protobuf_serialization_maptypes.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_maptypes_gen
)

qt_internal_add_test(tst_protobuf_serialization_complex_maptypes
    SOURCES
        tst_protobuf_serialization_complex_maptypes.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_maptypes_gen
)


qt_internal_add_test(tst_protobuf_deserialization_basictypes
    SOURCES
        tst_protobuf_deserialization_basictypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_fieldindexrange_gen
)

qt_internal_add_test(tst_protobuf_deserialization_repeatedtypes
    SOURCES
        tst_protobuf_deserialization_repeatedtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_repeatedtypes_gen
)

qt_internal_add_test(tst_protobuf_deserialization_maptypes
    SOURCES
        tst_protobuf_deserialization_maptypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_maptypes_gen
)

qt_internal_add_test(tst_protobuf_deserialization_complex_maptypes
    SOURCES
        tst_protobuf_deserialization_complex_maptypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_basictypes_gen
        tst_protobuf_maptypes_gen
)

qt_internal_add_test(tst_protobuf_non_packed_repeatedtypes
    SOURCES
        tst_protobuf_non_packed_repeatedtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::CorePrivate
        Qt::Protobuf
)
qt6_add_protobuf(tst_protobuf_non_packed_repeatedtypes
    PROTO_FILES
        proto/repeatednonpackedmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_non_packed_repeatedtypes)

qt_internal_add_test(tst_protobuf_unknown_field
    SOURCES
        tst_protobuf_unknown_field.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf)

qt6_add_protobuf(tst_protobuf_unknown_field
    PROTO_FILES
        proto/unknownfield.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)

qt_internal_add_test(tst_protobuf_raw_serializers
    SOURCES
        tst_protobuf_raw_serializers.cpp
    LIBRARIES
        Qt::Test
        Qt::ProtobufPrivate
        tst_protobuf_basictypes_gen
)

qt6_add_protobuf(tst_protobuf_non_final_basictypes_gen
    PROTO_FILES
        proto/basicmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated/non_final"
    GENERATE_NON_FINAL_MESSAGES
    EXTRA_NAMESPACE "nonfinal"
    HEADER_GUARD pragma
)
qt_autogen_tools_initial_setup(tst_protobuf_non_final_basictypes_gen)

if(UNIX AND NOT CMAKE_CROSSCOMPILING)
    qt_internal_add_test(tst_protobuf_internals
        SOURCES
            tst_protobuf_internals.cpp
        LIBRARIES
            Qt::Test
            Qt::Protobuf
            tst_protobuf_basictypes_gen
            tst_protobuf_non_final_basictypes_gen
    )
endif()
