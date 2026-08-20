# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(shared)

qt_internal_add_example(nodejs)
qt_internal_add_example(qwclient)
qt_internal_add_example(chatclient-html)

if(TARGET Qt::WebSockets AND NOT ANDROID)
    qt_internal_add_example(chatserver-cpp)
endif()
if(TARGET Qt::WebSockets AND TARGET Qt::Widgets)
    qt_internal_add_example(standalone)
    qt_internal_add_example(chatclient-qml)
endif()
