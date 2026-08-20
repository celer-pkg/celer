// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: button
    property alias imgSrc: image.source
    property alias system: emitter.system
    property alias group: emitter.group
    signal clicked
    property bool rotatedButton: false

    width: image.width
    height: image.sourceSize.height
    Image {
        id: image
        height: parent.height
        width: height/sourceSize.height * sourceSize.width

        anchors.horizontalCenter: parent.horizontalCenter
        rotation: button.rotatedButton ? ((Math.random() * 3 + 2) * (Math.random() <= 0.5 ? -1 : 1)) : 0
        MenuEmitter {
            id: emitter
            anchors.fill: parent
            //shape: MaskShape {source: image.source}
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {parent.clicked(); emitter.burst(400);}
    }
}
