# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

TEMPLATE = app
TARGET = lowenergyscanner

QT += quick bluetooth

CONFIG += qmltypes
QML_IMPORT_NAME = Scanner
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += main.cpp \
    device.cpp \
    deviceinfo.cpp \
    serviceinfo.cpp \
    characteristicinfo.cpp

HEADERS += \
    device.h \
    deviceinfo.h \
    serviceinfo.h \
    characteristicinfo.h

qml_resources.files = \
    qmldir \
    Characteristics.qml \
    Devices.qml \
    Dialog.qml \
    Header.qml \
    Label.qml \
    Main.qml \
    Menu.qml \
    Services.qml \
    assets/busy_dark.png

qml_resources.prefix = /qt/qml/Scanner

RESOURCES = qml_resources

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/lowenergyscanner
INSTALLS += target
