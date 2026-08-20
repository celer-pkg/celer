// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

ListView {
    id: listView
    width: 200
    height: 320

    required delegateModelAccess
    required model

    delegate: Rectangle {
        width: listView.width;
        height: 25

        required property var modelData
        required property int number
        property int modelNumber: modelData.number

        Text {
            text: number
        }

        function setValue(value)
        {
            number = value
        }

        function setModelValue(value: int)
        {
            modelData.number = value
        }
    }
}
