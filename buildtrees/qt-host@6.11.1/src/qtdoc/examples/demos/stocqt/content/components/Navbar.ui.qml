// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: bottom_Navbar
    width: 360
    height: 72
    color: "#1D1D1D"
    border.color: "#3C3535"

    property alias favoriteButton: favoriteButtonMouse
    property alias homeButton: homeButtonMouse
    property alias settingsButton: settingsButtonMouse

    state: "Home"

    Rectangle {
        id: horizontal_container
        color: "transparent"
        anchors.fill: parent
        anchors.rightMargin: 20
        anchors.leftMargin: 20
        anchors.topMargin: 12
        anchors.bottomMargin: 12

        RowLayout {
            anchors.fill: parent
            spacing: 76

            Rectangle {
                id: home
                width: 56
                height: 40
                Layout.preferredWidth: 56
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                color: "transparent"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    Image {
                        id: homeIcon
                        x: 16
                        width: 24
                        height: 24
                        clip: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                        source: "../images/home.png"
                    }

                    Text {
                        id: home1
                        x: 15
                        y: 28
                        width: 28
                        height: 12
                        color: "#787878"
                        text: qsTr("Home")
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignTop
                        wrapMode: Text.NoWrap
                        font.weight: Font.Normal
                        font.family: "Roboto"
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 12
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    }
                }

                MouseArea {
                    id: homeButtonMouse
                    anchors.fill: parent
                }
            }

            Rectangle {
                id: favoriteRect
                width: 56
                height: 40
                color: "transparent"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    Image {
                        id: starIcon
                        x: 16
                        width: 24
                        height: 24
                        clip: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                        source: "../images/stackStar.svg"
                    }

                    Text {
                        id: favorite
                        x: 7
                        y: 28
                        width: 43
                        height: 12
                        color: "#787878"
                        text: qsTr("Favorites")
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignTop
                        wrapMode: Text.NoWrap
                        font.weight: Font.Normal
                        font.family: "Roboto"
                        Layout.preferredWidth: 43
                        Layout.preferredHeight: 12
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    }
                }

                MouseArea {
                    id: favoriteButtonMouse
                    anchors.fill: parent
                }
                Layout.preferredWidth: 56
                Layout.preferredHeight: 40
            }

            Rectangle {
                id: setting
                width: 56
                height: 40
                color: "transparent"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.preferredWidth: 56
                Layout.preferredHeight: 40

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    Image {
                        id: infoIcon
                        x: 16
                        width: 24
                        height: 24
                        clip: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                        source: "../images/settings.svg"
                    }

                    Text {
                        id: about
                        x: 15
                        y: 28
                        width: 28
                        height: 12
                        color: "#787878"
                        text: qsTr("Settings")
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignTop
                        wrapMode: Text.NoWrap
                        font.weight: Font.Normal
                        font.family: "Roboto"
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 12
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    }
                }

                MouseArea {
                    id: settingsButtonMouse
                    anchors.fill: parent
                }
            }
        }
    }
    states: [
        State {
            name: "Default"
        },
        State {
            name: "Home"
            extend: "Default"

            PropertyChanges {
                target: home1
                color: "#f2f2f2"
                font.weight: Font.DemiBold
            }
        },
        State {
            name: "Favorite"
            extend: "Default"

            PropertyChanges {
                target: favorite
                color: "#f2f2f2"
                font.weight: Font.DemiBold
            }
        },

        State {
            name: "Settings"
            extend: "Default"

            PropertyChanges {
                target: about
                color: "#f2f2f2"
                font.weight: Font.DemiBold
            }
        }
    ]
}
