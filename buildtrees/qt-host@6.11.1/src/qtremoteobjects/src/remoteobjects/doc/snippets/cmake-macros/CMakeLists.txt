# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(directconnectserver)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

#! [remote_objects_cmake]
find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)
#! [remote_objects_cmake]

set(SOURCES
    main.cpp
    simpleswitch.cpp
)

#! [simpleSwitch_cmake_add_repc_source]
qt6_add_repc_sources(directconnectserver
    simpleswitch.rep
)
#! [simpleSwitch_cmake_add_repc_source]

#! [simpleSwitch_cmake_add_repc_replica]
qt6_add_repc_replicas(directconnectclient
    simpleswitch.rep
)
#! [simpleSwitch_cmake_add_repc_replica]

#! [simpleSwitch_cmake_add_repc_merged]
qt6_add_repc_merged(directconnectexample
    simpleswitch.rep
)
#! [simpleSwitch_cmake_add_repc_merged]

add_executable(directconnectserver ${SOURCES})
#! [remote_objects_cmake_link]
target_link_libraries(directconnectserver PRIVATE Qt6::RemoteObjects)
#! [remote_objects_cmake_link]

#! [simpleSwitch_cmake_rep_from_header]
qt6_reps_from_headers(directconnectexample
    simpleswitch.h
)
#! [simpleSwitch_cmake_rep_from_header]
