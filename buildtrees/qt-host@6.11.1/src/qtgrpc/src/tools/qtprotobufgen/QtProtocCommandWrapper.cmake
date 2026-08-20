#!${CMAKE_COMMAND} -P
# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(GENERATOR_NAME EQUAL "qtgrpc")
    set(ENV{QT_GRPC_OPTIONS} "${QT_GRPC_OPTIONS}")
elseif(GENERATOR_NAME EQUAL "qtprotobuf")
    set(ENV{QT_PROTOBUF_OPTIONS} "${QT_PROTOBUF_OPTIONS}")
endif()


execute_process(COMMAND
    ${PROTOC_EXECUTABLE}
    ${PROTOC_ARGS}
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Unable to execute ${PROTOC_EXECUTABLE}:(${result}) ${output}")
elseif(output)
    message("${output}")
endif()
