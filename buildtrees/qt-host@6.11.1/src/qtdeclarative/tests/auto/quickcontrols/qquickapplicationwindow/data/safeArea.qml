// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 600
    height: 400

    menuBar: Item { height: 30; }
    header: Item { height: 40; }

    property var margins: SafeArea.margins

    Item {
        anchors.fill: parent
    }

    footer: Item { height: 50; }
}
