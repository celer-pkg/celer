# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

QT += quick sensors svg

TARGET = sensorsshowcase
TEMPLATE = app

SOURCES = main.cpp

qml_resources.files = \
    qmldir \
    Main.qml \
    Accelerometer.qml \
    Compass.qml \
    Gyroscope.qml \
    Magnetometer.qml \
    Proximity.qml \
    ProgressXYZBar.qml \
    images/compass.svg \
    images/magnet.svg \
    images/qt_logo.png

qml_resources.prefix = /qt/qml/SensorShowcaseModule

data_resources.files = \
    qtquickcontrols2.conf

data_resources.prefix = /

RESOURCES += \
    qml_resources \
    data_resources

android {
    OTHER_FILES = android/AndroidManifest.xml
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

ios {
    QMAKE_INFO_PLIST = Info.plist
    EXAMPLE_FILES += Info.plist
}

target.path = $$[QT_INSTALL_EXAMPLES]/sensors/sensorsshowcase
INSTALLS += target
