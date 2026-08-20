// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    property string description
    property int precision: 2
    property alias from: slider.from
    property alias to: slider.to
    property alias value: slider.value

    Layout.fillWidth: true

    spacing: -10

    Slider {
        id: slider
        from: 0.0
        to: 1.0
        value: 0.0
        Layout.fillWidth: true
    }

    Label {
        id: label
        text: (parent.description.length == 0 ? "" : parent.description + ": ")
              + parent.value.toFixed(root.precision);
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    }
}
