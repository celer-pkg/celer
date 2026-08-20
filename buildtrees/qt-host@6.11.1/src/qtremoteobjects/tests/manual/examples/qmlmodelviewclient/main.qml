// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.5
import QtQuick.Window 2.2

Window {
    visible: true
    width: 800
    height: 600
    Text {
        id: dummy
    }

    ListView {
        id: view
        anchors.fill: parent
        focus: true

        currentIndex: 10

        model: remoteModel

        snapMode: ListView.SnapToItem
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 0

        delegate: Rectangle {
            width: view.width
            height: dummy.font.pixelSize * 2
            color: _color
            Text {
                anchors.centerIn: parent
                text: _text
            }
        }
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Home:
                view.currentIndex = 0;
            break;
            case Qt.Key_PageUp:
                currentIndex -= Math.random() * 300;
                if (currentIndex < 0)
                    currentIndex = 0;
                break;
            case Qt.Key_PageDown:
                currentIndex += Math.random() * 300;
                if (currentIndex >= count)
                    currentIndex = count - 1;
                break;
            case Qt.Key_End:
                currentIndex = count - 1;
            break;
            }
        }
    }
}
