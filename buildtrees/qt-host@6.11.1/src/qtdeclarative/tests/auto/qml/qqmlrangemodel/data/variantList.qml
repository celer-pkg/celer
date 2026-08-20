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
        property var currentValue: modelData

        function setValue(value: int)
        {
            modelData = value;
        }
    }
}
