// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Shapes

import ColorPalette
import QtOpenApiExampleStyle

pragma ComponentBehavior: Bound

Rectangle {
    id: root

    property int currentColorPage: 1
    property int totalColorPages: 0

    property bool loggedIn: false
    property int userId: -1
    property string currentUserAvatar: ""

    signal errorOccurred()

    ListModel {
        id: colorListModel
    }

    function fetchColors(page) {
        colorListModel.clear()
        //! [0]
        ColorsApi.getColors(page)
        //! [0]
    }

    Connections {
        target: ColorsApi

        function onGetColorsFinished(summary) {
            for (let i = 0; i < summary.getDataValue.length; i++) {
                colorListModel.append({
                    id: summary.getDataValue[i].getIdValue,
                    name: summary.getDataValue[i].getNameValue,
                    color: summary.getDataValue[i].getColorValue,
                    pantone_value: summary.getDataValue[i].getPantoneValueValue
                });
            }

            if (summary.getDataValue.length > 0) {
                root.currentColorPage = summary.getPageValue
                root.totalColorPages = summary.getTotalPagesValue
            } else if (root.totalColorPages > 0) { // summary.getDataValue.length == 0
                root.totalColorPages--;
                root.currentColorPage = root.totalColorPages;
            }
        }

        function onGetColorsErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }

        function onDeleteColorByIdFinished() {
            root.fetchColors(root.currentColorPage)
        }

        function onDeleteColorByIdErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }

        //! [1]
        function onAddColorFinished() {
            root.fetchColors(root.currentColorPage)
        }

        function onAddColorErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }
        //! [1]

        function onUpdateColorByIdFinished() {
            root.fetchColors(root.currentColorPage)
        }

        function onUpdateColorByIdErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }
    }

    Connections {
        target: UsersApi

        function onGetUserByIdFinished(summary) {
            // Check if the user id from the response matches the currently logged-in user id:
            if (summary.getIdValue === root.userId) {
                root.currentUserAvatar = summary.getAvatarValue
            } else {
                // This means an old request completed after a new login/logout.
                console.log("Ignored old user data for id: ", summary.getIdValue, " Current ID: ",
                            root.userId)
            }
        }

        function onGetUsersByPageErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }

        function onLoginUserFinished(summary) {
            root.userId = summary.getIdValue
            UsersApi.getUserById(root.userId)

            root.loggedIn = true

            UsersApi.setApiKey("token", summary.getTokenValue)
            ColorsApi.setApiKey("token", summary.getTokenValue)
        }

        function onLoginUserErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }

        function onLogoutUserFinished() {
            root.loggedIn = false
            root.userId = -1
            UsersApi.setApiKey("token", "")
            ColorsApi.setApiKey("token", "")
        }

        function onLogoutUserErrorOccurred(errorType, errorStr) {
            root.handleError(errorStr)
        }
    }

    onCurrentColorPageChanged: fetchColors(root.currentColorPage)

    color: UIStyle.background

    ColorDialogEditor {
        id: colorPopup
        onColorAdded: (colorNameField, colorRGBField, colorPantoneField) => {
            const colorData = {
                "name" : colorNameField,
                "color" : colorRGBField,
                "pantone_value" : colorPantoneField,
            }
            var color = Color.create(colorData)
            ColorsApi.addColor(color)
        }

        onColorUpdated: (colorNameField, colorRGBField, colorPantoneField, cid) => {
            const colorData = {
                "name" : colorNameField,
                "color" : colorRGBField,
                "pantone_value" : colorPantoneField,
            }
            var color = Color.create(colorData)
            ColorsApi.updateColorById(cid, color)
        }
    }

    ColorDialogDelete {
        id: colorDeletePopup
        onDeleteClicked: (cid) => {
            ColorsApi.deleteColorById(cid)
        }
    }

    ColumnLayout {
        // The main application layout
        anchors.fill :parent
        spacing: 0
        ToolBar {
            Layout.fillWidth: true
            Layout.minimumHeight: 35

            UserMenu {
                id: userMenu
                loggedInValue: root.loggedIn
                loggedInUserId: root.userId
                onConnectionError: (errorStr) => root.handleError(errorStr)
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5

                AbstractButton {
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: UIStyle.buttonBackground
                        border.color: UIStyle.buttonOutline
                        border.width: 1
                    }

                    Image {
                        source: UIStyle.iconPath("plus")
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        sourceSize.width: width
                        sourceSize.height: height

                    }
                    visible: root.loggedIn
                    onClicked: colorPopup.createNewColor()
                }

                AbstractButton {
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: UIStyle.buttonBackground
                        border.color: UIStyle.buttonOutline
                        border.width: 1
                    }

                    Image {
                        source: UIStyle.iconPath("update")
                        fillMode: Image.PreserveAspectFit
                        anchors.fill: parent
                        sourceSize.width: width
                        sourceSize.height: height
                    }

                    onClicked: {
                        root.fetchColors(root.currentColorPage)
                        userMenu.fetchUsers(userMenu.currentUserPage)
                    }
                }

                Item { Layout.fillWidth: true }

                Image {
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    source: "qrc:/qt/qml/ColorPalette/icons/qt.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                    text: qsTr("Color Palette")
                    font.pixelSize: UIStyle.fontSizeM
                    font.bold: true
                    color: UIStyle.titletextColor
                }

                Item { Layout.fillWidth: true }

                AbstractButton {
                    id: loginButton
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: 25
                    Item {
                        id: userImageCliped
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 25
                        height: 25

                        Image {
                            id: userImage
                            anchors.fill: parent
                            source: root.loggedIn ? root.currentUserAvatar
                                                  : UIStyle.iconPath("user")
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

                    onClicked: {
                        userMenu.fetchUsers(userMenu.currentUserPage);
                        userMenu.open()
                        var pos = mapToGlobal(Qt.point(x, y))
                        pos = userMenu.parent.mapFromGlobal(pos)
                        userMenu.x = x - userMenu.width + 50
                        userMenu.y = y + 15
                    }

                    Shape {
                        id: bubble
                        x: -loginTextElement.width - 25
                        y: -3
                        anchors.margins: 3

                        preferredRendererType: Shape.CurveRenderer

                        visible: !root.loggedIn

                        ShapePath {
                            strokeWidth: 0
                            fillColor: UIStyle.highlightColor
                            strokeColor: UIStyle.highlightBorderColor
                            startX: 5
                            startY: 0
                            PathLine {
                                x: 5 + loginTextElement.width + 6
                                y: 0
                            }
                            PathArc {
                                x: 10 + loginTextElement.width + 6
                                y: 5
                                radiusX: 5
                                radiusY: 5
                            }
                            // arrow
                            PathLine {
                                x: 10 + loginTextElement.width + 6
                                y: 8 + loginTextElement.height / 2 - 6
                            }
                            PathLine {
                                x: 10 + loginTextElement.width + 6 + 6
                                y: 8 + loginTextElement.height / 2
                            }
                            PathLine {
                                x: 10 + loginTextElement.width + 6
                                y: 8 + loginTextElement.height / 2 + 6
                            }
                            PathLine {
                                x: 10 + loginTextElement.width + 6
                                y: 5 + loginTextElement.height + 6
                            }
                            // end arrow
                            PathArc {
                                x: 5 + loginTextElement.width + 6
                                y: 10 + loginTextElement.height + 6
                                radiusX: 5
                                radiusY: 5
                            }
                            PathLine {
                                x: 5
                                y: 10 + loginTextElement.height + 6
                            }
                            PathArc {
                                x: 0
                                y: 5 + loginTextElement.height + 6
                                radiusX: 5
                                radiusY: 5
                            }
                            PathLine {
                                x: 0
                                y: 5
                            }
                            PathArc {
                                x: 5
                                y: 0
                                radiusX: 5
                                radiusY: 5
                            }
                        }
                        Text {
                            x: 8
                            y: 8
                            id: loginTextElement
                            color: UIStyle.textColor
                            text: qsTr("Log in to edit")
                            font.bold: true
                            horizontalAlignment: Qt.AlignHCenter
                            verticalAlignment: Qt.AlignVCenter
                        }
                    }
                }
            }
        }

        ListView {
            id: colorListView
            model: colorListModel

            footerPositioning: ListView.OverlayFooter
            spacing: 15
            clip: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            header:  Rectangle {
                height: 32
                width: parent.width
                color: UIStyle.background

                RowLayout {
                    anchors.fill: parent

                    component HeaderText : Text {
                        Layout.alignment: Qt.AlignVCenter
                        horizontalAlignment: Qt.AlignHCenter

                        font.pixelSize: UIStyle.fontSizeS
                        color: UIStyle.titletextColor
                    }
                    HeaderText {
                        id: headerName
                        text: qsTr("Color Name")
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 30
                    }
                    HeaderText {
                        id: headerRgb
                        text: qsTr("Rgb Value")
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 25
                    }
                    HeaderText {
                        id: headerPantone
                        text: qsTr("Pantone Value")
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 25
                        font.pixelSize: UIStyle.fontSizeS
                    }
                    HeaderText {
                        id: headerAction
                        text: qsTr("Action")
                        Layout.fillWidth: true
                        Layout.horizontalStretchFactor: 20
                    }
                }
            }

            delegate: Item {
                id: colorInfo

                required property var modelData

                width: colorListView.width
                height: (colorListView.height - 55) / 6 - colorListView.spacing
                // Header: 35, Footer 20, 55 together
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5

                    Rectangle {
                        id: colorSample
                        Layout.alignment: Qt.AlignVCenter
                        implicitWidth: 36
                        implicitHeight: 36
                        radius: 6
                        color: colorInfo.modelData.color
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.3 - colorSample.width
                        horizontalAlignment: Qt.AlignLeft
                        leftPadding: 5
                        text: colorInfo.modelData.name
                        color: UIStyle.textColor
                        font.pixelSize: UIStyle.fontSizeS
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.25
                        horizontalAlignment: Qt.AlignHCenter
                        text: colorInfo.modelData.color
                        color: UIStyle.textColor
                        font.pixelSize: UIStyle.fontSizeS
                    }

                    Text {
                        Layout.preferredWidth: colorInfo.width * 0.25
                        horizontalAlignment: Qt.AlignHCenter
                        text: colorInfo.modelData.pantone_value
                        color: UIStyle.textColor
                        font.pixelSize: UIStyle.fontSizeS
                    }

                    Item {
                        Layout.maximumHeight: 28
                        implicitHeight: buttonBox.implicitHeight
                        implicitWidth: buttonBox.implicitWidth

                        RowLayout {
                            id: buttonBox
                            anchors.fill: parent
                            ToolButton {
                                icon.source: UIStyle.iconPath("delete")
                                enabled: root.loggedIn
                                onClicked: colorDeletePopup.maybeDelete(colorInfo.modelData)
                            }
                            ToolButton {
                                icon.source: UIStyle.iconPath("edit")
                                enabled: root.loggedIn
                                onClicked: colorPopup.updateColor(colorInfo.modelData)
                            }
                        }
                    }
                }
            }

            footer: ToolBar {
                // Paginate buttons if more than one page
                visible: root.totalColorPages > 1
                rightPadding: 8
                implicitWidth: parent.width

                RowLayout {
                    anchors.fill: parent

                    Item { Layout.fillWidth: true /* spacer */ }

                    Repeater {
                        model: root.totalColorPages

                        ToolButton {
                            text: page
                            fontBold: root.currentColorPage === page

                            required property int index
                            readonly property int page: (index + 1)

                            onClicked: root.currentColorPage = page
                        }
                    }
                }
            }
        }
    }

    function resetState() {
        console.log("Resetting application state due to server disconnection/issue.");

        colorListModel.clear();
        userMenu.currentUsers.clear()

        root.currentColorPage = 1;
        root.totalColorPages = 0;
        userMenu.currentUserPage = 1;
        userMenu.totalUserPages = 0;

        // reset user session state
        root.loggedIn = false;
        root.userId = -1;
        root.currentUserAvatar = "";

        UsersApi.setApiKey("token", "");
        ColorsApi.setApiKey("token", "");

        userMenu.close()
    }

    function handleError(errorStr) {
        if (!root.visible)
            return;
        console.warn("Error message:", errorStr);
        root.resetState();
        root.errorOccurred()
    }
}
