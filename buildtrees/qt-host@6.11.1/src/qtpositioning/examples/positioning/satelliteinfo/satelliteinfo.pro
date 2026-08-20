# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

TEMPLATE = app
TARGET = satelliteinfo

QT += quick positioning positioningquick

CONFIG += qmltypes
QML_IMPORT_NAME = SatelliteInformation
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += main.cpp \
    satellitemodel.cpp \
    sortfiltermodel.cpp

HEADERS += \
    roles.h \
    satellitemodel.h \
    sortfiltermodel.h

qml_resources.files = \
    qmldir \
    ApplicationScreen.qml \
    Button.qml \
    Header.qml \
    HelpPopup.qml \
    LegendBox.qml \
    Main.qml \
    RssiView.qml \
    PageButton.qml \
    PermissionsScreen.qml \
    PositionBox.qml \
    SatelliteView.qml \
    SettingsView.qml \
    SkyView.qml \
    Theme.qml \
    ViewSwitch.qml

qml_resources.prefix = /qt/qml/SatelliteInformation

RESOURCES += qml_resources

icon_resources.files = \
    icons/checkbox.svg \
    icons/checkbox_blank.svg \
    icons/darkmode.svg \
    icons/filter.svg \
    icons/help.svg \
    icons/lightmode.svg \
    icons/place.svg \
    icons/qtlogo_green.png \
    icons/qtlogo_white.png \
    icons/rssiview.svg \
    icons/satellite_small.png \
    icons/satellite1.png \
    icons/satellite2.png \
    icons/search.svg \
    icons/settings.svg \
    icons/skyview.svg \
    icons/sort.svg \
    icons/tableview.svg

icon_resources.prefix = /qt/qml/SatelliteInformation

RESOURCES += icon_resources

data_resources.files = \
    nmealog.txt \
    fonts/TitilliumWeb-Regular.ttf \
    fonts/TitilliumWeb-SemiBold.ttf

data_resources.prefix = /

RESOURCES += data_resources

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

android {
    # explicitly link with serialport in order to
    # deploy it as a dependecy for nmea plugin
    QT += serialport
}

target.path = $$[QT_INSTALL_EXAMPLES]/positioning/satelliteinfo
INSTALLS += target

# Sign the app for location permissions to work properly
macos:!macx-xcode: QMAKE_POST_LINK += codesign -s - satelliteinfo.app
