# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if (TARGET Qt::Network AND QT_FEATURE_scxml_ecmascriptdatamodel AND NOT ANDROID)
    qt_internal_add_example(ftpclient)
endif()

if(TARGET Qt::Widgets)
    qt_internal_add_example(trafficlight-widgets-static)
    qt_internal_add_example(trafficlight-widgets-dynamic)
    if(QT_FEATURE_scxml_ecmascriptdatamodel)
        qt_internal_add_example(sudoku)
    endif()
endif()

if(TARGET Qt::Qml AND TARGET Qt::Gui)
    qt_internal_add_example(calculator)
    qt_internal_add_example(trafficlight-qml-static)
    qt_internal_add_example(trafficlight-qml-dynamic)
    qt_internal_add_example(trafficlight-qml-simple)
    qt_internal_add_example(mediaplayer)
    qt_internal_add_example(invoke)
endif()
