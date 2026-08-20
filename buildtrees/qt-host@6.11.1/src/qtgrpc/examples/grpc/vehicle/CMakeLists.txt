# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(vehicle LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/grpc/vehicle")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Protobuf
    ProtobufWellKnownTypes
    Grpc
    Quick
)

qt_standard_project_setup()

qt_add_executable(vehicle_client
    main.cpp
    vehiclemanager.h
    vehiclemanager.cpp
    vehiclethread.h
    vehiclethread.cpp
    navigationthread.h
    navigationthread.cpp
)

qt_add_protobuf(vehiclelib
    PROTO_FILES
        proto/vehicleservice.proto
        proto/navigationservice.proto
)

qt_add_grpc(vehiclelib CLIENT
    PROTO_FILES
        proto/vehicleservice.proto
        proto/navigationservice.proto
)

target_link_libraries(vehiclelib PRIVATE Qt6::ProtobufWellKnownTypes)

set_target_properties(vehicle_client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

qt_add_qml_module(vehicle_client
    URI qtgrpc.examples.vehicle
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        "StyledText.qml"
        "StyledProgressBar.qml"
        "Main.qml"
)

qt_add_resources(vehicle_client
    "resources"
    PREFIX "/"
    BASE "resources"
    FILES
        "resources/direction_left.svg"
        "resources/direction_right.svg"
        "resources/direction_straight.svg"
        "resources/fuel_icon.svg"
)

target_link_libraries(vehicle_client PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Protobuf
    Qt6::ProtobufWellKnownTypes
    Qt6::Grpc
    vehiclelib
)

install(TARGETS vehicle_client vehiclelib
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

add_subdirectory(server)
if(TARGET vehicle_server)
    add_dependencies(vehicle_client vehicle_server)
else()
    message(WARNING "${PROJECT_NAME} server requires gRPC. Please install the C++ libraries in your"
        " system or use vcpkg:"
        "\n${QT${QT_VERSION_MAJOR}_INSTALL_BINS}/qt-cmake -S ${CMAKE_CURRENT_SOURCE_DIR} \\"
        "\n-DQT_USE_VCPKG=TRUE")
endif()
