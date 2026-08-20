// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/* the RowLayout is in theory not necessary, but it is important for tooling
   that the type has the same shape independent of which Style is used
*/
RowLayout {
    id: root
    property string labelText: qsTr("Text")
    property alias from: slider.from
    property alias to: slider.to
    required property real sliderWidth
    property alias value: slider.value
    Slider {
        id: slider
        stepSize: 1

        Layout.preferredWidth: root.sliderWidth

        Label {
            text: root.labelText
            anchors.left: parent.left
            anchors.bottom: parent.top
            bottomPadding: -12
        }
        Label {
            text: parent.value
            anchors.right: parent.right
            anchors.bottom: parent.top
            bottomPadding: -12
        }
    }
}
