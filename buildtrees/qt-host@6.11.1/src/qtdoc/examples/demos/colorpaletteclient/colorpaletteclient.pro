# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

TEMPLATE = app
TARGET = colorpaletteclient

QT += core network qml quick

CONFIG += qmltypes
QML_IMPORT_NAME = ColorPalette
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += main.cpp \
           restservice.cpp \
           paginatedresource.cpp \
           basiclogin.cpp \

HEADERS += abstractresource.h \
           restservice.h \
           paginatedresource.h \
           basiclogin.h \

RESOURCES += colorpalette.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/demos/colorpaletteclient
INSTALLS += target
