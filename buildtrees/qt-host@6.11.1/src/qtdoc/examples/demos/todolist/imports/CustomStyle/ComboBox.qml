// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import ToDoList

T.ComboBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            90)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    padding: 5

    onCurrentIndexChanged: {
        comboBoxPopup.close()
    }

    contentItem: RowLayout {
        width: control.implicitBackgroundWidth

        Text {
            id: content

            color: Constants.secondaryColor
            text: control.displayText
            font.pixelSize: AppSettings.fontSize - 2
            Layout.alignment: Qt.AlignHCenter
        }

        Image {
            source: "images/Expand_Icon.svg"
        }
    }

    delegate: Text {
        required property int index
        required property string modelData

        text: modelData
        font.pixelSize: AppSettings.fontSize - 2
        elide: Text.ElideRight
        padding: 2
        width: control.width
        color: index === control.currentIndex ? "#41CD52" : Constants.secondaryColor
        horizontalAlignment: Text.AlignHCenter

        TapHandler {
            onTapped: control.currentIndex = parent.index
        }
    }

    popup: T.Popup {
        id: comboBoxPopup

        implicitWidth: control.implicitWidth
        implicitHeight: contentHeight
        padding: 2

        contentItem: ListView {
            implicitHeight: contentHeight
            model: control.delegateModel
            spacing: 10
            clip: true

            T.ScrollIndicator.vertical: T.ScrollIndicator {}
        }

        background: Pane { }
    }
}
