// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: layout
    width: 1920
    height: 1080
    color: "#00ffffff"
    border.color: "#00000000"
    border.width: 0
    property alias dotBtns: buttonTabs

    ButtonTabs {
        id: buttonTabs
        x: 780
        y: 972
        width: 360
        height: 80
        state: "state_state_Default"
    }
}

/*##^##
Designer {
    D{i:0;uuid:"4afa61f1-f47a-5131-a122-bc39511354ce"}
}
##^##*/

