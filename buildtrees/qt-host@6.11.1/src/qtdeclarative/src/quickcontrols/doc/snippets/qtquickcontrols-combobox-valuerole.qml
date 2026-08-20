// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//! [file]
ApplicationWindow {
    width: 640
    height: 480
    visible: true

    // Used as an example of a backend - this would usually be
    // e.g. a C++ type exposed to QML.
    QtObject {
        id: backend
        property int modifier
    }

    ComboBox {
        model: [
            { value: Qt.NoModifier, text: qsTr("No modifier") },
            { value: Qt.ShiftModifier, text: qsTr("Shift") },
            { value: Qt.ControlModifier, text: qsTr("Control") }
        ]
        textRole: "text"
        valueRole: "value"
        // Set currentValue to the value stored in the backend.
        currentValue: backend.modifier
        // When an item is selected, update the backend.
        onActivated: backend.modifier = currentValue
    }
}
//! [file]
