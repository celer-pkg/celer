// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: rootItem

    property alias text: textItem.text

    Rectangle {
        id: descriptionItem
        anchors.fill: parent
        color: mainView.backgroundColor2
        border.width: 1
        border.color: mainView.foregroundColor1
    }
    Flickable {
        id: flickableItem
        width: textItem.width
        height: textItem.height
        contentWidth: textItem.width
        contentHeight: textItem.contentHeight + 20
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        function ensureVisible(r) {
            if (contentX >= r.x)
                contentX = r.x;
            else if (contentX+width <= r.x+r.width)
                contentX = r.x+r.width-width;
            if (contentY >= r.y)
                contentY = r.y;
            else if (contentY+height <= r.y+r.height)
                contentY = r.y+r.height-height;
        }
        TextEdit {
            id: textItem
            width: rootItem.width
            height: rootItem.height
            padding: 10
            wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere
            onCursorRectangleChanged: flickableItem.ensureVisible(cursorRectangle)
            color: mainView.foregroundColor2
        }
    }
}
