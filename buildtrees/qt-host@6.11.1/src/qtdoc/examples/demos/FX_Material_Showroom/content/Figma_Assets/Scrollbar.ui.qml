// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: scrollbar
    width: 4
    height: 764
    anchors.rightMargin: -24
    property real scrollPos: 0

    Rectangle {
        id: base
        x: 0
        y: 0
        width: 4
        height: 764
        color: "#1affffff"
        radius: 8
    }

    Rectangle {
        id: filing
        x: 0
        y: scrollbar.scrollPos * 183
        width: 4
        height: 581
        color: "#ffffff"
        radius: 8
    }
}

/*##^##
Designer {
    D{i:0;height:764;width:4}D{i:1;uuid:"b67cda01-6e13-5f27-af3d-8696beeac4d2"}D{i:2;uuid:"90ac16c2-d7c5-5b9c-b9a1-812dcb63d874"}
}
##^##*/

