// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    color: "lightblue"
    width: 300; height: 200

//! [flow item]
    Flow {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 10

        Repeater {
            id: repeater
            model: {
                var strings = ["Text", "items", "flowing", "inside", "a",
                               "Flow", "item"];
                strings;
            }

            Rectangle {
                color: "white"
                width: textItem.width + 4
                height: textItem.height + 4
                Text {
                    id: textItem
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: repeater.model[index]
                    font.pixelSize: 40
                }
            }
        }
    }
//! [flow item]
}
