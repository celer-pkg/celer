// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: root
    required property string labelText
    required property real from
    required property real to
    required property real sliderWidth
    property alias value: rotationSlider.value

    spacing: 2

    Label {
        text: root.labelText
    }
    Slider {
        id: rotationSlider
        Layout.preferredWidth: root.sliderWidth
        Layout.minimumWidth: 160
        from: root.from
        to: root.to
    }
    Label {
        text: rotationSlider.value.toFixed(0)
    }
}
