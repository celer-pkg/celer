// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import ToDoList

T.Dialog {
    id: control

    property var model: [qsTr("Yes"), qsTr("No")]

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            implicitHeaderWidth,
                            implicitFooterWidth)

    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding + spacing)

    padding: 18
    spacing: 30

    background: Rectangle {
        color: Constants.mainColor
        border.color: Constants.secondaryColor
        radius: 8
    }

    contentItem: Text {
        text: control.title
        font.pixelSize: AppSettings.fontSize
        color: Constants.secondaryColor
    }

    footer: RowLayout {
        spacing: 0
        height: 40

        Button {
            text: qsTr("Yes")
            Layout.margins: 4
            Layout.fillHeight: true
            Layout.fillWidth: true
            onClicked: {
                control.accepted()
                control.close()
            }
        }

        Button {
            text: qsTr("No")
            Layout.margins: 4
            Layout.fillHeight: true
            Layout.fillWidth: true
            onClicked: {
                control.rejected()
                control.close()
            }
        }
    }

    T.Overlay.modal: Rectangle {
        color: "#aacfdbe7"
    }
}
