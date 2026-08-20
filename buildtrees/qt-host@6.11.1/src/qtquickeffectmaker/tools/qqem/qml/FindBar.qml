// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: rootItem

    property bool show: false
    property bool stringNotFound: currentEditorComponent ? currentEditorComponent.stringNotFound : false

    function setFindString(findString) {
        findTextField.text = findString;
        findTextField.selectAll();
        findTextField.focus = true;
    }
    function findNext() {
        if (currentEditorComponent)
            currentEditorComponent.findNext(findTextField.text);
    }
    function findPrev() {
        if (currentEditorComponent)
            currentEditorComponent.findPrev(findTextField.text);
    }

    Layout.fillWidth: true
    height: 44
    visible: rootItem.show
    enabled: visible
    color: mainView.backgroundColor1

    RowLayout {
        id: findTextRow
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        spacing: 5
        Item {
            width: 5
            height: 1
        }
        Text {
            font.pixelSize: 14
            color: mainView.foregroundColor2
            text: "Find:"
        }
        CustomTextField {
            id: findTextField
            Layout.fillWidth: true
            onAccepted: {
                findNext();
            }
            Item {
                id: stringNotFoundItem

                readonly property alias show: rootItem.stringNotFound
                property real showAnimated: show

                anchors.fill: parent
                anchors.margins: -1
                z: -1
                opacity: showAnimated
                visible: opacity > 0
                Rectangle {
                    anchors.fill: parent
                    color: "#dd4040"
                }
                Behavior on showAnimated {
                    NumberAnimation {
                        duration: 400
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        Button {
            id: findPrevButton
            Layout.preferredHeight: 40
            text: "Previous"
            onClicked: {
                findPrev();
            }
            Action {
                shortcut: StandardKey.FindPrevious
                enabled: rootItem.show
                onTriggered: {
                    findPrev();
                }
            }
        }
        Button {
            id: findNextButton
            Layout.preferredHeight: 40
            text: "Next"
            onClicked: {
                findNext();
            }
            Action {
                shortcut: StandardKey.FindNext
                enabled: rootItem.show
                onTriggered: {
                    findNext();
                }
            }
        }
        CustomIconButton {
            height: 22
            width: height
            icon: "images/icon_remove_shadow.png"
            onClicked: {
                mainView.showFindBar = false;
            }
        }
        Item {
            width: 1
            height: 1
        }
    }
}

