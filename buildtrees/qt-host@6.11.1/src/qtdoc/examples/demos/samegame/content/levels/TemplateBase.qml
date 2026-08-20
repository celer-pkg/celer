// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle{
    id: control
    property int scoreTarget: -1
    property int timeTarget: -1
    property int moveTarget: -1
    property bool mustClear: true
    property string goalText: "Clear the level..."

    property var startingGrid //If this isn't an array of ints, we will refuse to load the level.
    /*  Ints are 0-4. If not enough ints are there it will be prepadded with 0s to fill the grid
        (which ruins everything if you have the wrong number of rows).
        0 - No block
        1 - Red
        2 - Blue
        3 - Green
        4 - Yellow
        Ideas for future colors, but not supported in this version:
        5 - Purple
        6 - Cyan
        7 - Gray
        8 - Black
        9 - White
    */

    width: 320
    height: 416
    color: "white" //TODO: Theme support for both setting themes per level, and seeing it in the control!
}
