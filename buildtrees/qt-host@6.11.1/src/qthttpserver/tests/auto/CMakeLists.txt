# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from auto.pro.

add_subdirectory(cmake)
add_subdirectory(qhttpserver)
if (QT_FEATURE_concurrent)
    add_subdirectory(qhttpservermultithreaded)
endif()
add_subdirectory(qhttpserverresponder)
add_subdirectory(qhttpserverrouter)
add_subdirectory(qhttpserverresponse)
if(QT_FEATURE_private_tests)
    add_subdirectory(qabstracthttpserver)
    add_subdirectory(qhttpserverrequestfilter)
endif()
