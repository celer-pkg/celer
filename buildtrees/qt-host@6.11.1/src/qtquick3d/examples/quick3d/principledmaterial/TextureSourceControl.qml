// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick.Dialogs

import Example

RowLayout {
    id: root
    property Texture targetTexture: null
    property bool stampMode: false
    required property url defaultTexture
    property url stampSource: ""
    property alias defaultClearColor: drawer.clearColor
    property bool envMapMode: false

    GroupBox {
        title: "Image Source Mode"
        ColumnLayout {
            RadioButton {
                id: noTextureChoice
                text: "None"
                checked: true
                onCheckedChanged: {
                    root.targetTexture = null
                }
            }
            RadioButton {
                id: selectTextureChoice
                text: "Texture"
                checked: false
                onCheckedChanged: root.targetTexture = selectedTexture
            }
            RadioButton {
                id: loadImageChoice
                text: "Load Image"
                onCheckedChanged: root.targetTexture = loadTextureTexture
            }
            RadioButton {
                id: drawerChoice
                text: "Draw Texture"
                onCheckedChanged: root.targetTexture = drawerTexture
            }
            Label {
                visible: uvScale.visible
                Layout.fillWidth: true
                text: "UV Scale (" + uvScale.value.toFixed(2) + ")"
            }
            Slider {
                id: uvScale
                visible: !noTextureChoice.checked
                from: 0.0
                to: 50.0
                value: 1.0
                onValueChanged: {
                    updateUVScale()
                }
                function updateUVScale(){
                    if ( root.targetTexture ) {
                        root.targetTexture.scaleU = value
                        root.targetTexture.scaleV = value
                    }
                }
                Connections {
                    target: root
                    function onTargetTextureChanged(targetTexture){
                        uvScale.updateUVScale()
                    }
                }
            }
        }
    }
    Item {
        visible: selectTextureChoice.checked
        implicitWidth: 256
        implicitHeight: 256
        Image {
            id: previewImage
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
            source: root.defaultTexture
            Texture {
                id: selectedTexture
                source: root.defaultTexture
                mappingMode: root.envMapMode ? Texture.Environment : Texture.UV
            }
        }
    }

    Rectangle {
        id: loadTextureFrame
        implicitWidth: 256
        implicitHeight: 256
        color: "transparent"
        border.color: "black"
        visible: loadImageChoice.checked
        property url textureSource: ""
        Text {
            anchors.centerIn: parent
            text: "[Load Image]"
        }
        Image {
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
            visible: loadTextureFrame.textureSource !== null
            source: loadTextureFrame.textureSource
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textureSourceDialog.open()
            }
        }

        Texture {
            id: loadTextureTexture
            source: loadTextureFrame.textureSource
            mappingMode: root.envMapMode ? Texture.Environment : Texture.UV
        }

        ImageHelper {
            id: imageHelper
        }

        FileDialog {
            id: textureSourceDialog
            title: "Open an Image File"
            nameFilters: [ imageHelper.getSupportedImageFormatsFilter()]
            onAccepted: {
                if (textureSourceDialog.selectedFile !== null) {
                    loadTextureFrame.textureSource = textureSourceDialog.selectedFile
                }
            }
        }
    }
    ColumnLayout {
        visible: drawerChoice.checked
        Rectangle {
            implicitWidth: 260
            implicitHeight: 260
            color: "transparent"
            border.color: "black"
            Canvas {
                id: drawer
                width: 256
                height: 256
                x: 2
                y: 2

                property color penColor: "blue"
                property real penWidth: penWidthSlider.value
                property bool needsClear: true
                property color clearColor: "white"
                property var commands: []
                property var stampCommands: []
                property bool stampMode: root.stampMode
                //property point prevPoint : Qt.point(0, 0)

                onPaint: {
                    let ctx = getContext('2d');
                    if (needsClear) {
                        ctx.fillStyle = Qt.rgba(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                        ctx.fillRect(0, 0, width, height)
                        needsClear = false;
                    }
                    if (!stampMode) {
                        ctx.strokeStyle = Qt.rgba(penColor.r, penColor.g, penColor.b, penColor.a)
                        ctx.lineCap = "round"
                        ctx.lineWidth = penWidth;

                        for (let i = 0; i < commands.length; ++i) {
                            let command = commands[i];
                            ctx.beginPath()
                            ctx.moveTo(command.start.x, command.start.y);
                            ctx.lineTo(command.end.x, command.end.y);
                            ctx.stroke();
                        }
                        commands = [];
                    } else {
                        for (let i = 0; i < stampCommands.length; ++i) {
                            let stampCommand = stampCommands[i]
                            // get offset
                            let dX = stampCommand.x - stampcursor.width * 0.5
                            let dY = stampCommand.y - stampcursor.height * 0.5
                            ctx.drawImage(stampcursor, dX, dY)
                        }
                        stampCommands = [];
                    }
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: drawer
                enabled: drawerChoice.checked
                hoverEnabled: true
                //acceptedButtons: Qt.LeftButton
                property bool isDrawing: false
                property var lastPosition: Qt.point(0, 0)
                preventStealing: true
                clip: true

                Item {
                    id: cursor
                    Rectangle {
                        anchors.centerIn: parent
                        visible: !root.stampMode
                        width: drawer.penWidth
                        height: drawer.penWidth
                        radius: width * 0.5
                        color: drawer.penColor
                    }

                    Image {
                       id: stampcursor
                       anchors.centerIn: parent
                       visible: root.stampMode
                       source: root.stampSource
                    }
                }

                onEntered: cursor.visible = true
                onExited: cursor.visible = false

                onPressed: (mouse)=> {
                               if (mouse.button === Qt.LeftButton && !root.stampMode) {
                                   lastPosition = Qt.point(mouse.x, mouse.y)
                                   isDrawing = true
                               }
                           }
                onPositionChanged: (mouse)=> {
                                       if (isDrawing) {
                                           let pos = Qt.point(mouse.x, mouse.y);
                                           let command = {"start": lastPosition, "end": pos}
                                           drawer.commands.push(command)
                                           lastPosition = pos;
                                           drawer.requestPaint();
                                       }
                                       cursor.x = mouse.x
                                       cursor.y = mouse.y
                                   }
                onReleased: (mouse)=> {
                                if (mouse.button === Qt.LeftButton && isDrawing) {
                                    let pos = Qt.point(mouse.x, mouse.y);
                                    let command = {"start": lastPosition, "end": pos}
                                    drawer.commands.push(command)
                                    isDrawing = false;
                                    drawer.requestPaint();
                                } else if (root.stampMode) {
                                    drawer.stampCommands.push(Qt.point(mouse.x, mouse.y));
                                    drawer.requestPaint();
                                }
                            }
            }

        }
        RowLayout {
            visible: !root.stampMode
            spacing: 0
            Rectangle {
                id: whiteBrush
                implicitWidth: 25
                implicitHeight: 25
                color: "white"
                border.color: "black"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        drawer.penColor = parent.color;
                    }
                }
            }
            Rectangle {
                id: blackBrush
                implicitWidth: 25
                implicitHeight: 25
                color: "black"
                border.color: "black"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        drawer.penColor = parent.color;
                    }
                }
            }
            Rectangle {
                id: redBrush
                implicitWidth: 25
                implicitHeight: 25
                color: "red"
                border.color: "black"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        drawer.penColor = parent.color;
                    }
                }
            }
            Rectangle {
                id: greenBrush
                implicitWidth: 25
                implicitHeight: 25
                color: "green"
                border.color: "black"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        drawer.penColor = parent.color;
                    }
                }
            }
            Rectangle {
                id: blueBrush
                implicitWidth: 25
                implicitHeight: 25
                color: "blue"
                border.color: "black"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        drawer.penColor = parent.color;
                    }
                }
            }
            Label {
                text: " "
            }

            Button {
                text: "Clear"
                onClicked: {
                    drawer.needsClear = true
                    drawer.requestPaint()
                }
            }

        }
        RowLayout {
            visible: !root.stampMode
            Slider {
                id: penWidthSlider
                from: 1
                to: 50
                value: 5
            }
            Label {
                Layout.fillWidth: true
                text: "Pen Width"
            }
        }
    }

    Texture {
        id: drawerTexture
        sourceItem: drawerChoice.checked ? drawer : null
        mappingMode: root.envMapMode ? Texture.Environment : Texture.UV
    }

}

