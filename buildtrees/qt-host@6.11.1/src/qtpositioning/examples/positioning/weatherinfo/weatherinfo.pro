# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

TEMPLATE = app
TARGET = weatherinfo

QT += core network positioning qml quick

CONFIG += qmltypes
QML_IMPORT_NAME = Weather
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += main.cpp \
    appmodel.cpp \
    openmeteobackend.cpp \
    openweathermapbackend.cpp \
    providerbackend.cpp \
    weatherapibackend.cpp

HEADERS += appmodel.h \
    openmeteobackend.h \
    openweathermapbackend.h \
    providerbackend.h \
    weatherapibackend.h

qml_resources.files = \
    qmldir \
    BigForecastIcon.qml \
    ForecastIcon.qml \
    WeatherIcon.qml \
    WeatherInfo.qml \
    icons/weather-few-clouds.svg \
    icons/weather-fog.svg \
    icons/weather-haze.svg \
    icons/weather-icy.svg \
    icons/weather-overcast.svg \
    icons/weather-showers.svg \
    icons/weather-showers-scattered.svg \
    icons/weather-sleet.svg \
    icons/weather-snow.svg \
    icons/weather-storm.svg \
    icons/weather-sunny-very-few-clouds.svg \
    icons/weather-sunny.svg \
    icons/weather-thundershower.svg \
    icons/waypoint.svg

qml_resources.prefix = /qt/qml/Weather

RESOURCES += qml_resources

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/positioning/weatherinfo
INSTALLS += target

# Sign the app for location permissions to work properly
macos:!macx-xcode: QMAKE_POST_LINK += codesign -s - weatherinfo.app
