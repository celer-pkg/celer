// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// This example shows how to create your own highlight delegate for a ListView
// that uses a SpringAnimation to provide custom movement when the
// highlight bar is moved between items.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    width: 300
    height: 400
    implicitHeight: mainLayout.implicitHeight

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        GridLayout {
            id: settingsLayout
            columns: 2
            Label {
                text: "headerPositioning"
            }
            ComboBox {
                implicitContentWidthPolicy: ComboBox.WidestText
                currentIndex: 2
                onActivated: {
                    console.log("currentValue: " + currentValue + ", listView.header: " + listView.header)
                    if (currentValue == -1) {
                        if (listView.header)
                            listView.header = null
                    } else {
                        console.log("createObject()")
                        if (!listView.header)
                            listView.header = listViewHeader
                    }
                    listView.headerPositioning = currentValue
                }

                model: [
                    {text: "No header", value: -1},
                    {text: "InlineHeader", value: ListView.InlineHeader},
                    {text: "OverlayHeader", value: ListView.OverlayHeader},
                    {text: "PullBackHeader", value: ListView.PullBackHeader},
                ]
                textRole: "text"
                valueRole: "value"
            }


            Label {
                text: "highlight range"
            }

            RangeSlider {
                id: rs
                from: 0
                to: 300
                stepSize: 1
                snapMode: RangeSlider.SnapAlways
                first.value: 60
                second.value: 240

                function updateListView() {
                    listView.preferredHighlightBegin = first.value
                    listView.preferredHighlightEnd = second.value
                }

                first.onMoved: updateListView()
                second.onMoved: updateListView()

                Text {
                    anchors.centerIn: parent
                    parent: rs.first.handle
                    text: listView.preferredHighlightBegin.toFixed(0)
                }
                Text {
                    anchors.centerIn: parent
                    parent: rs.second.handle
                    text: listView.preferredHighlightEnd.toFixed(0)
                }
            }
        }

        Component {
            id: listViewHeader
            Rectangle {
                width: parent.width
                height: 20
                color: "green"
                Text {
                    text: "Header"
                    anchors.centerIn: parent
                }
            }
        }

        ListView {
            id: listView
            clip: true
            activeFocusOnTab: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 200
            snapMode: ListView.SnapToItem
            model: 9
            header: listViewHeader
            headerPositioning: ListView.OverlayHeader
            delegate: Rectangle {
                id: delegateRoot
                width: parent.width
                height: 50
                border.color: "black"
                border.width: 1
                color: "transparent"

                required property int index

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    text: 'Item: ' + delegateRoot.index
                }
                // indent the item if it is the current item
                states: State {
                    name: "Current"
                    when: delegateRoot.ListView.isCurrentItem
                    PropertyChanges { delegateRoot.x: 20 }
                }
                transitions: Transition {
                    NumberAnimation {
                        properties: "x"
                        duration: 200
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: delegateRoot.ListView.view.currentIndex = delegateRoot.index
                }
            }
            focus: true

            // Set the highlight delegate. Note we must also set highlightFollowsCurrentItem
            // to false so the highlight delegate can control how the highlight is moved.
            highlight: Rectangle {
                width: parent.width
                height: 50
                color: "#FFFF88"
                y: ListView.view.currentItem.y
                Behavior on y {
                    SpringAnimation {
                        spring: 2
                        damping: 0.1
                    }
                }
            }

            highlightFollowsCurrentItem: true
            preferredHighlightBegin: 60
            preferredHighlightEnd: 240
            highlightRangeMode: ListView.StrictlyEnforceRange


            //////// geometry visualizers. Non-interactive.
            Rectangle {
                // background and color for the ListView to visualize its geometry
                color: "#10000000"
                anchors.fill: listView
                border.color:"black"
            }
            Rectangle {
                // Only a human aid for visualizing the hightlight range
                id: highlightRangeVisualizer
                y: listView.preferredHighlightBegin
                height: listView.preferredHighlightEnd - y
                width: parent.width
                color: "transparent"
                border.color: "red"
                border.width: 1
            }
        }
    }
}
