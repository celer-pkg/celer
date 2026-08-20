// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import Config

Item {
    id: root

    property alias playbackRate: slider.value

    Layout.minimumWidth: 50
    Layout.maximumWidth: 200

    RowLayout {
        anchors.fill: root
        spacing: root.width >= 85 ? 10 : 2

        Image {
            visible: root.width >= 85
            source: ControlImages.iconSource("Rate_Icon")
        }

        CustomSlider {
            id: slider
            Layout.fillWidth: true
            snapMode: Slider.SnapOnRelease
            from: 0.5
            to: 2.5
            stepSize: 0.5
            value: 1.0
        }

        Label {
            text: slider.value.toFixed(1) + "x"
            color: Config.highlightColor
        }
    }
}
