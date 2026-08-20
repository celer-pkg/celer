# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(adueditor
        GUI
    SOURCES
        interface.ui
        main.cpp
        mainwindow.cpp mainwindow.h
        modbustcpclient.cpp modbustcpclient.h modbustcpclient_p.h
        plaintextedit.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
        Qt::SerialPort
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)
