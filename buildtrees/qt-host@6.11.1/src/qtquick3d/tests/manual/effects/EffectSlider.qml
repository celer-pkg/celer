// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    property string description
    property int precision: 2
    property bool exponential: false
    property double expValue: exponential ? Math.pow(2.0, sliderValue) : sliderValue
    property alias sliderValue: slider.value
    property alias fromValue: slider.from
    property alias toValue: slider.to

    Slider {
        id: slider
        from: 0.0
        to: 1.0
        value: 0.5
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        text: (parent.description.length == 0 ? "" : parent.description + ": ")
                   + parent.expValue.toFixed(precision);
    }
}
