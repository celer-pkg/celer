// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick

ThermostatInfo {
    required property list<real> temperatureValues
    property real maxTemp
    property real minTemp
    property real avgTemp

    title: qsTr("Temperature")
    leftIcon: "images/temperature"
    topLabel: qsTr("Average: %1°C".arg(avgTemp.toFixed(1)))
    bottomLeftLabel: qsTr("Minimum: %1°C".arg(minTemp.toFixed(1)))
    bottomLeftIcon: "images/minTemp.svg"
    bottomRightLabel: qsTr("Maximum: %1°C".arg(maxTemp.toFixed(1)))
    bottomRightIcon: "images/maxTemp.svg"
}
