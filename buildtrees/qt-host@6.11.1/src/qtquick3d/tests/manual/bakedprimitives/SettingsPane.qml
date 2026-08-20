// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    width: settingsDrawer.width
    height: parent.height

    property var viewport

    property bool lmEnabled: checkboxBakingEnabled.checked
    property string shape0Source: shapeSelector0.currentText
    property string shape1Source: shapeSelector1.currentText
    property string shape2Source: shapeSelector2.currentText
    property string shape3Source: shapeSelector3.currentText

    property string shape0Key: shapeKey0.text
    property string shape1Key: shapeKey1.text
    property string shape2Key: shapeKey2.text
    property string shape3Key: shapeKey3.text

    property real shape0TPU: shape0TPU.value
    property real shape1TPU: shape1TPU.value
    property real shape2TPU: shape2TPU.value
    property real shape3TPU: shape3TPU.value

    property real viewX: settingsDrawer.visible ? (settingsDrawer.x + settingsDrawer.width) : 0

    RoundButton {
        id: iconOpen
        padding: 10
        x: padding + root.viewX
        y: padding
        text: "="
        onClicked: {
            settingsDrawer.visible = !settingsDrawer.visible
        }
    }

    Drawer {
        id: settingsDrawer
        edge: Qt.LeftEdge
        interactive: false
        modal: false
        height: parent.height

        enter: Transition {
            NumberAnimation {
                property: "position"
                to: 1.0
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }

        exit: Transition {
            NumberAnimation {
                property: "position"
                to: 0.0
                duration: 400
                easing.type: Easing.InOutQuad
            }
        }

        Page {
            anchors.fill: parent

            header: ToolBar {
                width: parent.width
                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Settings"
                    font.pointSize: 17
                }
            }

            ScrollView {
                anchors.fill: parent
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                padding: 10
                contentHeight: settingsArea.implicitHeight

                ColumnLayout {
                    id: settingsArea
                    width: parent.width
                    spacing: 5

                    component SliderWithValue: RowLayout {
                        id: sliderWithValue
                        property alias value: slider.value
                        property alias from: slider.from
                        property alias to: slider.to
                        property alias stepSize: slider.stepSize
                        property int numDecimals: 3
                        readonly property bool highlight: slider.hovered
                                                          || slider.pressed
                        Slider {
                            id: slider
                            stepSize: 0.01
                            Layout.minimumWidth: 200
                            Layout.maximumWidth: 200
                        }
                        Label {
                            id: valueText
                            text: slider.value.toFixed(
                                      sliderWithValue.numDecimals)
                            Layout.minimumWidth: 80
                            Layout.maximumWidth: 80
                        }
                    }

                    CheckBox {
                        id: checkboxBakingEnabled
                        checked: true
                        text: qsTr("Enable Baking")
                    }

                    Button {
                        text: "Bake lightmap"
                        onClicked: viewport.bakeLightmap()
                    }

                    Button {
                        text: "Denoise lightmap"
                        onClicked: viewport.denoiseLightmap()
                    }

                    Label {
                        text: "Shape A"
                        font.pointSize: 12
                        font.bold: true
                    }

                    ComboBox {
                        id: shapeSelector0
                        model: ["#Sphere", "#Cone", "#Cube", "#Cylinder"]
                        currentIndex: 2
                    }

                    TextField {
                        id: shapeKey0
                        text: "keyA"
                    }

                    Text {
                        text: "Texels per unit: " + shape0TPU.value.toFixed(1)
                    }

                    Slider {
                        id: shape0TPU
                        from: 0
                        to: 1
                        stepSize: 0.1
                    }

                    Label {
                        text: "Shape B"
                        font.pointSize: 12
                        font.bold: true
                    }

                    ComboBox {
                        id: shapeSelector1
                        model: ["#Sphere", "#Cone", "#Cube", "#Cylinder"]
                        currentIndex: 0
                    }

                    TextField {
                        id: shapeKey1
                        text: "keyB"
                    }

                    Text {
                        text: "Texels per unit: " + shape1TPU.value.toFixed(1)
                    }

                    Slider {
                        id: shape1TPU
                        from: 0
                        to: 1
                        stepSize: 0.1
                    }

                    Label {
                        text: "Shape C"
                        font.pointSize: 12
                        font.bold: true
                    }

                    ComboBox {
                        id: shapeSelector2
                        model: ["#Sphere", "#Cone", "#Cube", "#Cylinder"]
                        currentIndex: 1
                    }

                    TextField {
                        id: shapeKey2
                        text: "keyC"
                    }

                    Text {
                        text: "Texels per unit: " + shape2TPU.value.toFixed(1)
                    }

                    Slider {
                        id: shape2TPU
                        from: 0
                        to: 1
                        stepSize: 0.1
                    }

                    Label {
                        text: "Shape D"
                        font.pointSize: 12
                        font.bold: true
                    }

                    ComboBox {
                        id: shapeSelector3
                        model: ["#Sphere", "#Cone", "#Cube", "#Cylinder"]
                        currentIndex: 3
                    }

                    TextField {
                        id: shapeKey3
                        text: "keyD"
                    }

                    Text {
                        text: "Texels per unit: " + shape3TPU.value.toFixed(1)
                    }

                    Slider {
                        id: shape3TPU
                        from: 0
                        to: 1
                        stepSize: 0.1
                    }
                }
            }
        }
    }
}
