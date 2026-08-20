# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if (NOT ANDROID)
    qt_internal_add_example(remoteobjects_server)
    qt_internal_add_example(simpleswitch)
endif()
add_subdirectory(websockets)
if(TARGET Qt::Widgets)
    qt_internal_add_example(modelviewclient)
    qt_internal_add_example(modelviewserver)
endif()
if(QT_FEATURE_ssl AND NOT ANDROID)
    qt_internal_add_example(ssl)
endif()
if(TARGET Qt::Quick)
    qt_internal_add_example(clientapp)
endif()
