// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    width: 100
    height: 100

// this should have no effect
// qmlformat on

// qmlformat off
// qmlformat off
Rectangle {width:200;height:200;color:"red"}
// qmlformat on

Item {
// qmlformat off
Item {
Item {
// qmlformat on
// this should start from 3 * indentsize
}
}
}

// this disables formatting for the rest of the file
// qmlformat off
            Rectangle {
                width: 50
                height: 50
                color: "blue"
            }
}
