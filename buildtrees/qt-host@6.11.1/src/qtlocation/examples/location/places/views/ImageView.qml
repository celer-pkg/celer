// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls

Item {
    id: root
    property Place place
    width: parent.width
    height: parent.height

    GridView {
        id: gridView

        anchors.fill: parent

        model: place.imageModel

        cellWidth: width / 3
        cellHeight: cellWidth

        delegate: Rectangle {
            width: gridView.cellWidth
            height: gridView.cellHeight

            color: "#30FFFFFF"

            Image {
                anchors.fill: parent
                anchors.margins: 5

                source: url

                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.positionViewAtIndex(index, ListView.Contain);
                    root.state = "list";
                }
            }
        }
    }

    ListView {
        id: listView

        anchors.top: parent.top
        anchors.bottom: position.top
        width: parent.width
        spacing: 10

        model: place.imageModel
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem

        visible: false

        delegate: Item {
            width: listView.width
            height: listView.height

            Image {
                anchors.fill: parent
                source: url
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.state = ""
                }
            }

            Button {
                id: button
                text: qsTr("Open url")
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    Qt.openUrlExternally(supplier.url)
                }
            }
        }
    }

    Label {
        id: position

        width: parent.width
        anchors.bottom: parent.bottom
        visible: listView.visible

        text: (listView.currentIndex + 1) + '/' + listView.model.totalCount
        horizontalAlignment: Text.AlignRight
    }

    states: [
        State {
            name: "list"
            PropertyChanges {
                target: gridView
                visible: false
            }
            PropertyChanges {
                target: listView
                visible: true
            }
        }
    ]
}
