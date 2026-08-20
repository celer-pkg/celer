// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    required property int fontSize
    property alias xText: xBar.text
    property alias xValue: xBar.value
    property alias yText: yBar.text
    property alias yValue: yBar.value
    property alias zText: zBar.text
    property alias zValue: zBar.value

    component NamedProgressBar: ColumnLayout {
        property alias text: axes.text
        property alias value: bar.value
        Text {
            id: axes
            font.pixelSize: root.fontSize
            Layout.fillWidth: true
        }
        ProgressBar {
            id: bar
            Layout.fillWidth: true
        }
    }

    NamedProgressBar {
        id: xBar
    }

    NamedProgressBar {
        id: yBar
    }

    NamedProgressBar {
        id: zBar
    }
}
