// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property real volume: volumeSlider.value / 100

    Layout.minimumWidth: 50
    Layout.maximumWidth: 200

    RowLayout {
        anchors.fill: root
        spacing: root.width >= 85 ? 10 : 2

        Image {
            visible: root.width >= 85
            source: ControlImages.iconSource("Mute_Icon")
        }

        CustomSlider {
            id: volumeSlider
            enabled: true
            to: 100.0
            value: 100.0

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }

        Image {
            source: ControlImages.iconSource("Volume_Icon")
        }
    }
}
