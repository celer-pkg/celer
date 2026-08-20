# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

QT += nfc quick quickcontrols2

CONFIG += qmltypes
QML_IMPORT_NAME = NdefEditorModule
QML_IMPORT_MAJOR_VERSION = 1

TARGET = ndefeditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    nfcmanager.cpp \
    nfctarget.cpp \
    ndefmessagemodel.cpp

HEADERS += \
    nfcmanager.h \
    nfctarget.h \
    ndefmessagemodel.h

qml_resources.files = \
    qmldir \
    Main.qml \
    MainWindow.qml \
    NdefRecordDelegate.qml

qml_resources.prefix = /qt/qml/NdefEditorModule

theme_resources.files = \
    icons/ndefeditor/20x20@2/add.png \
    icons/ndefeditor/20x20@2/arrow_back.png \
    icons/ndefeditor/20x20@2/file_download.png \
    icons/ndefeditor/20x20@2/file_upload.png \
    icons/ndefeditor/20x20@2/link.png \
    icons/ndefeditor/20x20@2/text_snippet.png \
    icons/ndefeditor/20x20@3/add.png \
    icons/ndefeditor/20x20@3/arrow_back.png \
    icons/ndefeditor/20x20@3/file_download.png \
    icons/ndefeditor/20x20@3/file_upload.png \
    icons/ndefeditor/20x20@3/link.png \
    icons/ndefeditor/20x20@3/text_snippet.png \
    icons/ndefeditor/20x20@4/add.png \
    icons/ndefeditor/20x20@4/arrow_back.png \
    icons/ndefeditor/20x20@4/file_download.png \
    icons/ndefeditor/20x20@4/file_upload.png \
    icons/ndefeditor/20x20@4/link.png \
    icons/ndefeditor/20x20@4/text_snippet.png \
    icons/ndefeditor/20x20/add.png \
    icons/ndefeditor/20x20/arrow_back.png \
    icons/ndefeditor/20x20/file_download.png \
    icons/ndefeditor/20x20/file_upload.png \
    icons/ndefeditor/20x20/link.png \
    icons/ndefeditor/20x20/text_snippet.png \
    icons/ndefeditor/index.theme

theme_resources.prefix = /

RESOURCES += qml_resources theme_resources

ios: QMAKE_INFO_PLIST = Info.qmake.plist

target.path = $$[QT_INSTALL_EXAMPLES]/nfc/ndefeditor
INSTALLS += target
