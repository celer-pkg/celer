# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_program(go NAMES go go.exe)

if(go AND QT_ALLOW_DOWNLOAD AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(test_server_go)
    set(qt_internal_grpc_use_golang_test_server TRUE)
else()
    add_subdirectory(test_server)
endif()
add_subdirectory(client_service)
add_subdirectory(client_test_common)
add_subdirectory(mockserver)
