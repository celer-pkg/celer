// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//! [document]
import QtQuick

//! [focusscope window]
Rectangle {
    id: window

    color: "white"; width: 240; height: 150

    Column {
        anchors.centerIn: parent; spacing: 15

        MyFocusScopeWidget {
            focus: true             //set this MyWidget to receive the focus
            color: "lightblue"
        }
        MyFocusScopeWidget {
            color: "palegreen"
        }
    }

}
//! [focusscope window]
//! [document]
