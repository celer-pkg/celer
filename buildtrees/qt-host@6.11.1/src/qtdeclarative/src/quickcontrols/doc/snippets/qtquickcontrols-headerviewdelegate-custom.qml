// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 580
    height: 390
    visible: true

    HorizontalHeaderView {
        id: horizontalHeaderView
        anchors.left: parent.left
        anchors.leftMargin: verticalHeaderView.width
        anchors.right: parent.right
        anchors.top: parent.top
        model: ["Name", "Address", "Quant"]

        //! [horizontal-delegate]
        delegate: HorizontalHeaderViewDelegate {
            id: horizontalDelegate

            required property int index
            required property string modelData

            //! [horizontal-background]
            background: Rectangle {
                height: horizontalDelegate.height
                color: columnCheckBox.checked ? palette.highlight : palette.base
                radius: 8
            }
            //! [horizontal-background]

            //! [horizontal-contentItem]
            contentItem: Item {
                implicitWidth: columnCheckBox.implicitWidth * 2
                implicitHeight: 40

                CheckBox {
                    id: columnCheckBox
                    anchors.centerIn: parent
                    text: horizontalDelegate.modelData
                    Component.onCompleted: checked = horizontalDelegate.index === 1
                }
            }
            //! [horizontal-contentItem]
        }
        //! [horizontal-delegate]
    }

    VerticalHeaderView {
        id: verticalHeaderView
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: horizontalHeaderView.height
        model: 6

        //! [vertical-delegate]
        delegate: VerticalHeaderViewDelegate {
            id: verticalDelegate

            required property int index

            //! [background]
            background: Rectangle {
                height: verticalDelegate.height
                color: palette.base
                border.width: rowCheckBox.checked ? 2 : 0
                border.color: palette.highlight
                radius: 8
            }
            //! [vertical-background]

            //! [vertical-contentItem]
            contentItem: Item {
                implicitWidth: rowCheckBox.implicitWidth * 2
                implicitHeight: 40

                CheckBox {
                    id: rowCheckBox
                    anchors.centerIn: parent
                    text: verticalDelegate.index + 1
                    Component.onCompleted: checked = verticalDelegate.index % 3 === 0
                }
            }
            //! [vertical-contentItem]
        }
        //! [vertical-delegate]
    }
}
