# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Basic functionality
if(TARGET Qt6::qtprotobufgen)
    get_target_property(protoc_version WrapProtoc::WrapProtoc _qt_internal_protobuf_version)

    add_subdirectory(basic)
    add_subdirectory(json)
    add_subdirectory(syntax)
    add_subdirectory(nopackage)
    add_subdirectory(enums)
    add_subdirectory(externalpackage)
    add_subdirectory(sequence)
    add_subdirectory(extranamespace)
    add_subdirectory(nested)
    add_subdirectory(converters)
    add_subdirectory(duplicated_metatypes)
    add_subdirectory(recursive)
    if(protoc_version VERSION_GREATER_EQUAL "3.12")
        add_subdirectory(optional)
    endif()

    if(TARGET Qt6::ProtobufWellKnownTypes)
        add_subdirectory(wellknown)
    endif()

    if(NOT CMAKE_CROSSCOMPILING AND TARGET Qt6::ProtobufWellKnownTypes)
        add_subdirectory(conformance)
    endif()
endif()
add_subdirectory(qprotobuflazymessagepointer)
add_subdirectory(qprotobufoneof)
add_subdirectory(qprotobufpropertyorderingbuilder)
add_subdirectory(transparentwrapper)
