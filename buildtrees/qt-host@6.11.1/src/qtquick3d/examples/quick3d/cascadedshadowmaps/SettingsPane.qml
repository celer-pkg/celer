// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    width: settingsDrawer.width
    height: parent.height
    property alias drawerVisible: settingsDrawer.visible
    property bool drawerOpen: settingsDrawer.position === 1.0

    property real shadowFactor: sliderDirectionaLightShadowFactor.value
    property vector3d eulerRotation: Qt.vector3d(sliderDirectionalLightRotX.value,
                                                 sliderDirectionalLightRotY.value,
                                                 0)
    property real csmSplit1: sliderCSMSplit1.value
    property real csmSplit2: sliderCSMSplit2.value
    property real csmSplit3: sliderCSMSplit3.value
    property int csmNumSplits: sliderNumSplits.currentIndex
    property int shadowMapQuality: shadowmapquality_combobox.currentIndex
    property real csmBlendRatio: sliderBlendRatio.value
    property real shadowBias: sliderShadowBiasDirLight.value
    property real pcfFactor: sliderPCFFactor.value
    property int softShadowQuality: softshadowquality_combobox.currentIndex
    property real shadowMapFar: sliderShadowMapFar.value
    property real clipFar: sliderCameraClipFar.value
    property bool lockShadowmapTexels: checkboxLockShadowmapTexels.checked

    property real viewX: settingsDrawer.visible ? (settingsDrawer.x + settingsDrawer.width) : 0

    RoundButton {
        id: iconOpen
        icon.source: "sliders.svg"
        icon.width: 25
        icon.height: 25
        padding: 10
        x: padding + root.viewX
        y: padding
        onClicked: {
            settingsDrawer.visible = !settingsDrawer.visible;
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

                    component SliderWithValue : RowLayout {
                        id: sliderWithValue
                        property alias value: slider.value
                        property alias from: slider.from
                        property alias to: slider.to
                        property alias stepSize: slider.stepSize
                        property int numDecimals: 3
                        readonly property bool highlight: slider.hovered || slider.pressed
                        Slider {
                            id: slider
                            stepSize: 0.01
                            Layout.minimumWidth: 200
                            Layout.maximumWidth: 200
                        }
                        Label {
                            id: valueText
                            text: slider.value.toFixed(sliderWithValue.numDecimals)
                            Layout.minimumWidth: 80
                            Layout.maximumWidth: 80
                        }
                    }



                    Label {
                        text: "Camera Clip Far"
                    }

                    SliderWithValue {
                        id: sliderCameraClipFar
                        value: 15000
                        from: 0
                        to: 30000
                        stepSize: 1000
                        numDecimals: 0
                    }

                    Label {
                        text: "Shadow Map Far"
                    }

                    SliderWithValue {
                        id: sliderShadowMapFar
                        value: 15000
                        from: 0
                        to: 30000
                        stepSize: 1000
                        numDecimals: 0
                    }

                    Label {
                        text: "Light Euler Rotation XY"
                    }

                    SliderWithValue {
                        id: sliderDirectionalLightRotX
                        value: -40
                        from: -180
                        to: 0
                        numDecimals: 0
                    }

                    SliderWithValue {
                        id: sliderDirectionalLightRotY
                        value: -120
                        from: -180
                        to: 180
                        numDecimals: 0
                    }

                    Label {
                        text: "Shadow Factor"
                    }

                    SliderWithValue {
                        id: sliderDirectionaLightShadowFactor
                        value: 75
                        from: 0
                        to: 100
                        numDecimals: 0
                    }

                    Label {
                        text: "Num Cascade Splits"
                    }

                    ComboBox {
                        id: sliderNumSplits
                        model: ListModel {
                            id: model
                            ListElement { text: "0" }
                            ListElement { text: "1" }
                            ListElement { text: "2" }
                            ListElement { text: "3" }
                        }
                        currentIndex: 2
                    }

                    Label {
                        text: "Cascade Splits"
                    }

                    SliderWithValue {
                        id: sliderCSMSplit1
                        value: 0.15
                        from: 0.01
                        to: 0.99
                        numDecimals: 2
                    }

                    SliderWithValue {
                        id: sliderCSMSplit2
                        value: 0.5
                        from: 0.01
                        to: 0.99
                        numDecimals: 2
                    }

                    SliderWithValue {
                        id: sliderCSMSplit3
                        value: 0.75
                        from: 0.01
                        to: 0.99
                        numDecimals: 2
                    }

                    Label {
                        text: "Cascade Blend Ratio"
                    }

                    SliderWithValue {
                        id: sliderBlendRatio
                        value: 0.05
                        from: 0
                        to: 1
                        numDecimals: 2
                    }

                    Label {
                        text: "PCF Factor"
                    }

                    SliderWithValue {
                        id: sliderPCFFactor
                        value: 3
                        from: 0
                        to: 30
                        stepSize: 1
                        numDecimals: 0
                    }

                    Label {
                        text: "Shadow Bias"
                    }

                    SliderWithValue {
                        id: sliderShadowBiasDirLight
                        value: 10
                        from: 0
                        to: 30
                        stepSize: 1
                        numDecimals: 0
                    }

                    Label {
                        text: "Soft-shadow Quality"
                    }

                    ComboBox {
                        id: softshadowquality_combobox
                        editable: false
                        model: ListModel {
                            ListElement { text: "Hard" }
                            ListElement { text: "PCF4" }
                            ListElement { text: "PCF8" }
                            ListElement { text: "PCF16" }
                        }
                        currentIndex: 3
                    }

                    Label {
                        text: "Shadow Map Quality"
                    }

                    ComboBox {
                        id: shadowmapquality_combobox
                        editable: false
                        model: ListModel {
                            ListElement { text: "Low" }
                            ListElement { text: "Medium" }
                            ListElement { text: "High" }
                            ListElement { text: "VeryHigh" }
                            ListElement { text: "Ultra" }
                        }
                        currentIndex: 2
                    }

                    CheckBox {
                        id: checkboxLockShadowmapTexels
                        checked: false
                        text: qsTr("Lock Shadow Map Texels")
                    }
                }
            }
        }
    }
}

