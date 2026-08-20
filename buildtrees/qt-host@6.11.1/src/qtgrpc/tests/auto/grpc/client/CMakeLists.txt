# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
add_subdirectory(shared)

add_subdirectory(deadline)
add_subdirectory(unarycall)
add_subdirectory(serverstream)
add_subdirectory(clientstream)
add_subdirectory(bidistream)
if(QT_FEATURE_ssl AND NOT WIN32)
    add_subdirectory(ssl)
endif()
if(TARGET grpc_mock_server)
    add_subdirectory(end2end)
    add_subdirectory(interceptors)
endif()
