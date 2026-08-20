// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: root

    readonly property bool debugEnabled: qnItem ? qnItem.debug.drawCallCount > 0 : false
    property int drawCallCount: qnItem ? qnItem.debug.drawCallCount : 0
    property int triCount: qnItem ? qnItem.debug.triangleCount : 0

    property int fillDrawCallCount: qnItem ? qnItem.debug.fillDrawCallCount : 0
    property int strokeDrawCallCount: qnItem ? qnItem.debug.strokeDrawCallCount : 0
    property int textDrawCallCount: qnItem ? qnItem.debug.textDrawCallCount : 0

    property int fillTriCount: qnItem ? qnItem.debug.fillTriangleCount : 0
    property int strokeTriCount: qnItem ? qnItem.debug.strokeTriangleCount : 0
    property int textTriCount: qnItem ? qnItem.debug.textTriangleCount : 0

    height: debugEnabled ? infoContent.height + 20 * dp : warningContent.height + 20 * dp
    width: debugEnabled ? infoContent.width + 40 * dp : warningContent.width + 40 * dp

    component InfoTextItem: Text {
        property bool mainText: false
        font.pixelSize: mainText ? 20 * dp : 16 * dp
        color: "#d0d0d0"
        textFormat: Text.StyledText
    }

    Rectangle {
        anchors.fill: parent
        color: "#404040"
    }

    Text {
        id: warningContent
        x: 20 * dp
        y: 10 * dp
        font.pixelSize: 20 * dp
        color: "#d0d0d0"
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        text: "No debugging enabled.\nPlease set QCPAINTER_DEBUG_COLLECT\nenvironment variable."
        visible: !root.debugEnabled
    }

    Column {
        id: infoContent
        visible: root.debugEnabled
        x: 20 * dp
        y: 10 * dp
        InfoTextItem {
            mainText: true
            text: "DRAW CALLS: <b>" + root.drawCallCount + "</b>"
        }
        InfoTextItem {
            text: "* FILL: <b>" + root.fillDrawCallCount + "</b>"
        }
        InfoTextItem {
            text: "* STROKE: <b>" + root.strokeDrawCallCount + "</b>"
        }
        InfoTextItem {
            text: "* TEXT: <b>" + root.textDrawCallCount + "</b>"
        }
        Item {
            width: 1
            height: 10 * dp
        }
        InfoTextItem {
            mainText: true
            text: "TRIANGLES: <b>" + root.triCount + "</b>"
        }
        InfoTextItem {
            text: "* FILL: <b>" + root.fillTriCount + "</b>"
        }
        InfoTextItem {
            text: "* STROKE: <b>" + root.strokeTriCount + "</b>"
        }
        InfoTextItem {
            text: "* TEXT: <b>" + root.textTriCount + "</b>"
        }
    }
}
