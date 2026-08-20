// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects

Item {
    id: current

    property string topText: "20*"
    property string bottomText: "Mostly cloudy"
    property string weatherIcon: "sunny"
    property real smallSide: (current.width < current.height ? current.width : current.height)

    Text {
        id: text1
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: current.topText
        font.pixelSize: 64
        color: "white"
    }

    MultiEffect {
        source: text1
        anchors.fill: text1
        shadowEnabled: true
        shadowBlur: 0.5
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 2
        shadowOpacity: 0.6
    }

    WeatherIcon {
        id: img
        anchors.top: text1.bottom
        anchors.topMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter
        weatherIcon: current.weatherIcon
        width: current.smallSide * 0.5
        height: current.smallSide * 0.5
    }

    Text {
        id: text2
        anchors.top: img.bottom
        anchors.topMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter
        text: current.bottomText
        font.pixelSize: 32
        horizontalAlignment: Text.AlignHCenter
        color: "white"
    }

    MultiEffect {
        source: text2
        anchors.fill: text2
        shadowEnabled: true
        shadowBlur: 0.5
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 2
        shadowOpacity: 0.6
    }
}
