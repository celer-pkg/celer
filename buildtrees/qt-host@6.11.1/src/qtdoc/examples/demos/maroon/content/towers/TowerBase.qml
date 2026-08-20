// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import examples.Maroon

Item {
    property real hp: 1
    property real range: 0
    property real damage: 0
    property int rof: 100
    property int fireCounter: 0
    property int income: 0
    property int row: 0
    property int col: 0

    width: (parent as GameCanvas)?.squareSize ?? 0
    height: (parent as GameCanvas)?.squareSize ?? 0
    //This is how it is placed on the gameboard, do not modify/animate the X/Y/Z of a TowerBase please
    x: col * width
    y: row * height
    z: 1000

    function dieImpl() { stdDeath.start(); destroy(1000); }
    function sell() { destroy(); }

    SequentialAnimation on opacity {
        id: stdDeath
        running: false
        loops: 2
        NumberAnimation { from: 1; to: 0; }
        NumberAnimation { from: 0; to: 1; }
    }
}
