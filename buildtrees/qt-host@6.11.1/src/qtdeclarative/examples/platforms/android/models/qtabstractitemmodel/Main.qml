// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls

//! [0]
Rectangle {
    id: mainRectangle

    property AbstractItemModel dataModel
//! [0]
    color: "#00414A"
    border.color: "#00414A"
    border.width: 2

//! [1]
    TableView {
        id: tableView

        model: mainRectangle.dataModel

        anchors {fill: parent; margins: 20}
        columnSpacing: 4
        rowSpacing: 6
        boundsBehavior: TableView.OvershootBounds
        clip: true

        ScrollBar.vertical: ScrollBar {
           policy: ScrollBar.AsNeeded
        }
        ScrollBar.horizontal: ScrollBar{
           policy: ScrollBar.AsNeeded
        }
//! [1]

//! [2]
        delegate: Rectangle {
            implicitWidth: (tableView.height > tableView.width) ? tableView.width / 10 : tableView.height / 5
            implicitHeight: implicitWidth

            required property var model

            color: "#2CDE85"
            border {color: "#00414A"; width: 2}

            TextEdit {
                // Calls MyDataModel::data to get data based on the roles.
                // Called in Qt qtMainLoopThread thread context.
                //
                // After editing is finished, call MyDataModel::setData()
                // to update the value of selected cell.
                onEditingFinished: parent.model.edit = text

                text: parent.model.display
                font {pixelSize: 26; bold: true}
                padding: 5
                anchors.fill: parent
                wrapMode: TextEdit.Wrap
                horizontalAlignment: TextEdit.AlignHCenter
                verticalAlignment: TextEdit.AlignVCenter
            }
        }
//! [2]
    }
}
