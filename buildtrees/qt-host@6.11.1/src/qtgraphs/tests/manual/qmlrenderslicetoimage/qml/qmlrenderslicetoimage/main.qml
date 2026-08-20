// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtGraphs
import "."

Item {
    id: mainview
    width: 1920
    height: 1080

    ColumnLayout {
        id: controls
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        property url selectedFile: fileDialog.selectedFile

        Button {
            text: "Set Save Path"
            onClicked: {
                fileDialog.open()
            }
        }

        Text {
            text: "Path to save :"
        }

        Text {
            text: controls.selectedFile.toString()
        }

        RadioButton {
            id: rowRadio
            text: "Row"
            checked: true
        }

        RadioButton {
            text: "Column"
        }

        Text {
            text: "Selected " + (rowRadio.checked ? "Row" : "Column")
        }

        TextField {
            id: textField
            validator: RegularExpressionValidator{
                regularExpression: /\d+/
            }
        }

        Button {
            text: "Slice To Image"
            onClicked: {
                var rowCol = (rowRadio.checked ? Graphs3D.SliceCaptureType.RowImage : Graphs3D.SliceCaptureType.ColumnImage)
                var index = textField.text
                if (tabBar.currentIndex === 0)
                    surfaceGraph.graph.renderSliceToImage(-1, index, rowCol, controls.selectedFile);
                else
                    barGraph.graph.renderSliceToImage(index, rowCol, controls.selectedFile);
            }
        }
    }

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        fileMode: FileDialog.SaveFile
        selectedFile: controls.selectedFile
        onAccepted: {
            controls.selectedFile = selectedFile
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: controls.left
        height: parent.height
        TabBar {
            id: tabBar
            width: parent.width
            TabButton {
                text: "Surface"
            }

            TabButton {
                text: "Bar"
            }
        }

        StackLayout {
            width: parent.width
            currentIndex: tabBar.currentIndex
            anchors.top: tabBar.bottom
            anchors.bottom: parent.bottom

            SurfaceGraph {
                id: surfaceGraph
            }

            BarGraph {
                id: barGraph
            }
        }
    }
}
