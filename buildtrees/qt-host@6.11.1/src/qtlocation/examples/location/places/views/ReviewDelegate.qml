// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls

Item {
    id: root
    signal showReview()

    height: icon.height + 8

    Image {
        id: icon

        width: 64
        height: 64

        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.leftMargin: 4

        source: model.supplier.icon.url(Qt.size(64, 64), Icon.List)
        fillMode: Image.PreserveAspectFit
    }

    Label {
        anchors.top: icon.top
        anchors.topMargin: 4
        anchors.left: icon.right
        anchors.leftMargin: 4
        anchors.right: root.right
        anchors.rightMargin: 4

        text: model.title
        font.bold: true

        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        maximumLineCount: 2
    }

    RatingView {
        anchors.bottom: icon.bottom
        anchors.bottomMargin: 4
        anchors.left: icon.right
        anchors.leftMargin: 4
        anchors.right: root.right
        anchors.rightMargin: 4

        rating: model.rating
        size: 16
    }

    MouseArea {
        anchors.fill: parent
        onClicked: showReview()
    }
}
