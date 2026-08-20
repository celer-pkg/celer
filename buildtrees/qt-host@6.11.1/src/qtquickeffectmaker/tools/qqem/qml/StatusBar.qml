// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: rootItem

    anchors.bottom: parent.bottom
    // Currently StatusBar is only visible when there are errors to show
    anchors.bottomMargin: (errorMessageItem.errorString == "") ? -height : 0
    anchors.left: parent.left
    anchors.right: parent.right
    height: 32 + errorMessageTextItem.lineCount * 14

    Behavior on anchors.bottomMargin {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    Rectangle {
        anchors.fill: parent
        color: mainView.backgroundColor2
    }

    Item {
        id: errorMessageItem
        property var error: effectManager.effectError
        property string errorString: error.message
        // Helper so when error disappears we can smoothly animate it away
        property string visibleErrorString

        function updateErrorString() {
            if (errorString === "") {
                showMessageAnimation.stop();
                hideMessageAnimation.restart();
            } else {
                hideMessageAnimation.stop();
                showMessageAnimation.restart();
            }
        }

        anchors.fill: parent
        opacity: 0

        onErrorStringChanged: {
            updateErrorString();
        }

        SequentialAnimation {
            id: showMessageAnimation
            ScriptAction {
                script: {
                    errorMessageItem.visibleErrorString = errorMessageItem.errorString;
                }
            }
            NumberAnimation {
                target: errorMessageItem
                property: "opacity"
                to: 1.0
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }
        SequentialAnimation {
            id: hideMessageAnimation
            NumberAnimation {
                target: errorMessageItem
                property: "opacity"
                to: 0.0
                duration: 500
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: errorMessageItem.visibleErrorString = "";
            }
        }

        Image {
            id: errorIcon
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 32
            width: height
            source: "images/icon_error.png"
            mipmap: true
        }
        Text {
            id: errorMessageTextItem
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: errorIcon.right
            anchors.leftMargin: 10
            anchors.right: showErrorButton.left
            anchors.rightMargin: 10
            font.pixelSize: 14
            text: errorMessageItem.visibleErrorString
            color: mainView.foregroundColor2
            elide: Text.ElideRight
            maximumLineCount: 3
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
        CustomIconButton {
            id: showErrorButton
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10
            height: 40
            width: height * (288 / 128)
            icon: "images/button_show.png"
            description: "Show Error"
            enabled: errorMessageItem.error.line > -1
            onClicked: {
                mainView.outputView.showCodePreview();
                outputEditorView.showErrorCodeLine();
            }
        }
    }
}
