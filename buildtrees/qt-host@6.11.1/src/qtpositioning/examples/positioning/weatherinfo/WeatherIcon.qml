// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects

Item {
    id: container

    property string weatherIcon: "sunny"


    Image {
        id: mask
        source: "icons/weather-" + container.weatherIcon + ".svg"
        smooth: true
        anchors.fill: parent
        sourceSize.width: width
        sourceSize.height: height
        layer.enabled: true
        visible: false
    }
    MultiEffect {
        source: mask
        anchors.fill: parent
        brightness: 1 // make icons white, remove for dark icons
    }
}
