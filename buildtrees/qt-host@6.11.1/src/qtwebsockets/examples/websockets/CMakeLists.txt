# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Quick)
    qt_internal_add_example(qmlwebsocketclient)
    qt_internal_add_example(qmlwebsocketserver)
endif()
if (NOT ANDROID)
    qt_internal_add_example(echoclient)
    qt_internal_add_example(echoserver)
    qt_internal_add_example(simplechat)
    if(QT_FEATURE_ssl)
        qt_internal_add_example(sslechoserver)
        qt_internal_add_example(sslechoclient)
    endif()
endif()
