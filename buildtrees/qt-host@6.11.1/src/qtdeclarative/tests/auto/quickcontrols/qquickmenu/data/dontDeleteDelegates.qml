// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 400
    height: 400

    property alias menu: menu
    property alias delegateComponent1: delegateComponent1
    property alias delegateComponent2: delegateComponent2

    Component {
        id: delegateComponent1

        MenuItem {}
    }

    Component {
        id: delegateComponent2

        MenuItem {}
    }

    Menu {
        id: menu
    }
}
