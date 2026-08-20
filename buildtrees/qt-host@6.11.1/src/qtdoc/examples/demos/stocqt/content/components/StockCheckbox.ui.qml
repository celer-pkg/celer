// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick 2.15
import QtQuick.Controls 2.15

CheckBox {
    id: cb
    width: 43
    height: 22
    property color activeTextColor: "#cca35e"

    property color activeColor: "#33ae9469"

    property alias cbText: ctext.text

    indicator: Rectangle {
        id: rectangle
        radius: 4
        border.color: cb.checked ? "#000000" : "#4d3c3535"
        anchors.fill: parent
        color: cb.checked? cb.activeColor : "transparent"

        Text {
            id: ctext
            width: 42
            height: 22
            color: cb.checked ? cb.activeTextColor : "#bfbfbf"
            text: qsTr("Low")
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            anchors.verticalCenterOffset: 4
            anchors.horizontalCenter: parent.horizontalCenter
            font.weight: cb.checked ? Font.Bold : Font.Normal
            font.family: "Roboto"

        }
    }
}
