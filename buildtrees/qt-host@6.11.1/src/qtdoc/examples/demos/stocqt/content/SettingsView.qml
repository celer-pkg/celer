// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: rectangle
    color: "#101010"

    property bool portrait: width < height

    Rectangle {
        id: banner
        width: parent.width
        height: rectangle.portrait? 70 : 0
        color: parent.color
        anchors.top: parent.top
        anchors.topMargin: 10
        visible: rectangle.portrait

        Image {
            id: logoBig
            visible: rectangle.width < rectangle.height
            source: "images/qtLogo.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: logoSmall
            visible: rectangle.width > rectangle.height
            source: "images/qtLogo2.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }
    }

    GridLayout {
        id: gridLayout
        width: parent.width
        anchors.top: banner.bottom
        anchors.bottom: parent.bottom
        flow: GridLayout.TopToBottom
        rows: rectangle.portrait? 2 : 1
        columns: 2
        anchors.bottomMargin: 10
        anchors.topMargin: 10

        Rectangle {
            id: infoPanel
            width: 323
            height: 307
            color: "#1d1d1d"
            radius: 10
            border.color: "#3c3535"
            border.width: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: 22
            Layout.leftMargin: 22

            Text {
                id: infoText
                color: "#dfdfdf"
                text: qsTr("StocQt is an example application for visualising stock market trends using the QtGraphs module. The Example showcases real-time stock price data and allows users to select their preferred stock charts, explore historical trends, and toggle various timeframes. The application also allows for comparing of 5 stocks simultaneously")
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                wrapMode: Text.Wrap
                anchors.topMargin: 18
                anchors.bottomMargin: 18
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                font.weight: Font.Normal
                font.family: "Titillium Web"
            }
        }


    }
}
