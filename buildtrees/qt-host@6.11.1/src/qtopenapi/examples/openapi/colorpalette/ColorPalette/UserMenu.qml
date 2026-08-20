// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import QtOpenApiExampleStyle

Popup {
    id: userMenu

    required property bool loggedInValue
    required property int loggedInUserId
    property int currentUserPage: 1
    property int totalUserPages: 0

    readonly property alias currentUsers: usersListModel

    signal connectionError(string errorStr)

    ListModel {
        id: usersListModel
    }

    function fetchUsers(page) {
        usersListModel.clear()
        UsersApi.getUsersByPage(page)
    }

    Connections {
        target: UsersApi

        function onGetUsersByPageFinished(summary) {
            for (let i = 0; i < summary.getDataValue.length; i++) {
                usersListModel.append({
                    id: summary.getDataValue[i].getIdValue,
                    email: summary.getDataValue[i].getEmailValue,
                    firstName: summary.getDataValue[i].getFirstNameValue,
                    lastName: summary.getDataValue[i].getLastNameValue,
                    avatar: summary.getDataValue[i].getAvatarValue,
                });
                userMenu.currentUserPage = summary.getPageValue
                userMenu.totalUserPages = summary.getTotalPagesValue
            }
        }

        function onGetUsersByPageErrorOccurred(errorType, errorStr) {
            userMenu.connectionError(errorStr);
        }
    }

    onCurrentUserPageChanged: fetchUsers(currentUserPage)

    width: 280
    height: 270

    background: Item {}

    Rectangle {
        radius: 8
        border.width: 0
        color: UIStyle.background

        anchors.fill: parent

        ListView {
            id: userListView
            anchors.fill: parent
            anchors.topMargin: 5
            anchors.bottomMargin: 2

            model: usersListModel
            spacing: 7
            footerPositioning: ListView.PullBackFooter
            clip: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            delegate: Item {
                id: userInfo
                anchors.left: parent ? parent.left : undefined
                anchors.right: parent ? parent.right : undefined
                anchors.leftMargin: 10
                anchors.rightMargin: 5

                height: 30
                width: userListView.width

                required property var model

                // Check if the delegate's user is the currently logged-in user :
                readonly property bool logged: (model.id === userMenu.loggedInUserId)

                Item {
                    id: userImageCliped
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 30
                    height: 30

                    Image {
                        id: userImage
                        anchors.fill: parent
                        source: userInfo.model.avatar
                        visible: false
                    }

                    Image {
                        id: userMask
                        source: "qrc:/qt/qml/ColorPalette/icons/userMask.svg"
                        anchors.fill: userImage
                        anchors.margins: 4
                        visible: false
                    }

                    MultiEffect {
                        source: userImage
                        anchors.fill: userImage
                        maskSource: userMask
                        maskEnabled: true
                    }
                }

                Text {
                    id: userMailLabel
                    anchors.left: userImageCliped.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 5
                    text: userInfo.model.email
                    color: UIStyle.textColor
                    font.bold: userInfo.logged
                }

                ToolButton {
                    id: logInOutButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 5

                    icon.source: UIStyle.iconPath(userInfo.logged ? "logout" : "login")
                    enabled: userInfo.logged || !userMenu.loggedInValue

                    onClicked: {
                        if (userInfo.logged) {
                            UsersApi.logoutUser()
                        } else {
                            //! [0]
                            let credentials = Credentials.create(userInfo.model.email, "apassword");
                            UsersApi.loginUser(credentials);
                            //! [0]
                            userMenu.close()
                        }
                    }
                }

            }
            footer: ToolBar {
                // Paginate buttons if more than one page
                visible: userMenu.totalUserPages > 1
                rightPadding: 8
                implicitWidth: parent.width

                RowLayout {
                    anchors.fill: parent

                    Item { Layout.fillWidth: true /* spacer */ }

                    Repeater {
                        model: userMenu.totalUserPages

                        ToolButton {
                            text: page
                            fontBold: userMenu.currentUserPage === page

                            required property int index
                            readonly property int page: (index + 1)

                            onClicked: userMenu.currentUserPage = page
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        radius: 8
        border.color: UIStyle.buttonOutline
        border.width: 2
        color: "transparent"

        anchors.fill: parent
    }
}
