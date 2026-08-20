// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

ListView {
    id: listView
    width: 200
    height: 320
    required model

    delegate: Rectangle {
        width: listView.width;
        height: 25

        required property var modelData
        required property int number
        required property string text

        property var currentValue: number + ": " + text
        property var currentData: modelData.number + ": " + modelData.text

        Text {
            text: currentValue
        }

        function setValue(value: int)
        {
            number = value
        }

        function setModelValue(value: int)
        {
            modelData.number = value;
        }

        function setModelData(data)
        {
            modelData = data;
        }
    }
}
