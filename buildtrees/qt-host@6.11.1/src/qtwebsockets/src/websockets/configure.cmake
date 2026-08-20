# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries



#### Tests



#### Features

qt_feature("websockets-qml" PUBLIC
    LABEL "WebSocket QML Type"
    PURPOSE "Provides QML Type for Qt WebSockets."
    CONDITION TARGET Qt::Quick
)

qt_configure_add_summary_section(NAME "Qt WebSockets")
qt_configure_add_summary_entry(ARGS "websockets-qml")
qt_configure_end_summary_section() # end of "Qt WebSockets" section
