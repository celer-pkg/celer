# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Configure file allow execute_process to detect changes to idl file.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/testserverlib.idl
    ${CMAKE_CURRENT_BINARY_DIR}/testserverlib.idl
    COPYONLY
)

# Run midl at configure time because qt6_target_typelibs checks
# for presence of tlb file at configure time
execute_process(
    COMMAND
        midl /h testserverlib.h /tlb testserverlib.tlb testserverlib.idl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

qt_add_library(testserver
SHARED
    testserver.h
    testserver.cpp
    testserver.def
    testserverlib.idl
    testserver.rc
)

# Trigger rebuild of resources if tlb file changed
set_property(
    SOURCE
        testserver.rc
    PROPERTY
        OBJECT_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/testserverlib.tlb
)

target_include_directories(testserver
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(testserver
    PRIVATE
        Qt::CorePrivate
        Qt::AxBasePrivate
)
