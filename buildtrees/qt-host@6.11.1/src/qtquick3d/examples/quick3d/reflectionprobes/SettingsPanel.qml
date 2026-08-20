// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: settingsDrawer.width
    height: parent.height

    property int timeSlicingIndex: timeSlicingComboBox.currentIndex
    property int refreshModeIndex: refreshModeComboBox.currentIndex
    property int qualityIndex: qualityComboBox.currentIndex
    property vector3d probeSize: Qt.vector3d(probeSizeXSlider.value, probeSizeYSlider.value, probeSizeZSlider.value)
    property vector3d probePosition: Qt.vector3d(probePositionXSlider.value, probePositionYSlider.value, probePositionZSlider.value)
    property bool probeParallaxCorrection: parallaxCheckBox.checked
    property real sphereRoughness: materialRoughnessSlider.value
    property bool sphereReceivesReflection: sphereReceivesReflectionsCheckBox.checked
    property bool floorReceivesReflection: floorReceivesReflectionsCheckBox.checked
    property bool spriteParticlesEnabled: particleSystemSpriteCheckBox.checked
    property bool modelParticlesEnabled: particleSystemModelCheckBox.checked

    property real iconSize: 50

    Button {
        x: settingsDrawer.visible ? (settingsDrawer.x + settingsDrawer.width) : 0
        anchors.top: parent.top
        width: root.iconSize
        height: width
        icon.width: width * 0.3
        icon.height: height * 0.3
        icon.source: "res/icon_settings.png"
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            settingsDrawer.visible = !settingsDrawer.visible;
        }
    }

    Drawer {
        id: settingsDrawer
        edge: Qt.LeftEdge
        interactive: false
        modal: false

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

        ScrollView {
            anchors.fill: parent
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            padding: 10
            background: Rectangle {
                color: "white"
            }

            Flickable {
                clip: true
                contentWidth: settingsArea.width
                contentHeight: settingsArea.height

                Column {
                    id: settingsArea
                    spacing: 5

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Reflection Map"
                        font.pixelSize: 14
                    }

                    Item { width: 1; height: 10 }

                    Text {
                        text: "Time Slicing"
                    }

                    ComboBox {
                        id: timeSlicingComboBox
                        width: 200
                        model: [ "None", "All Faces At Once", "Individual Faces" ]
                    }

                    Text {
                        text: "Refresh Mode"
                    }

                    ComboBox {
                        id: refreshModeComboBox
                        width: 200
                        model: [ "Every Frame", "First Frame" ]
                    }

                    Text {
                        text: "Reflection Map Quality"
                    }

                    ComboBox {
                        id: qualityComboBox
                        width: 200
                        model: [ "Very Low", "Low", "Medium", "High", "Very High" ]
                    }

                    Item { width: 1; height: 10 }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Floor Reflection Probe"
                        font.pixelSize: 14
                    }

                    Item { width: 1; height: 10 }

                    CheckBox {
                        id: parallaxCheckBox
                        checked: true
                        text: qsTr("Parallax Correction")
                    }

                    Text {
                        text: "Box Size (" + probeSizeXSlider.value + ", " + probeSizeYSlider.value + ", " + probeSizeZSlider.value + ")"
                    }

                    Slider {
                        id: probeSizeXSlider
                        from: 0
                        value: 1000
                        to: 1000
                        stepSize: 1
                    }

                    Slider {
                        id: probeSizeYSlider
                        from: 0
                        value: 950
                        to: 1000
                        stepSize: 1
                    }

                    Slider {
                        id: probeSizeZSlider
                        from: 0
                        value: 650
                        to: 1000
                        stepSize: 1
                    }

                    Text {
                        text: "Position (" + probePositionXSlider.value + ", " + probePositionYSlider.value + ", " + probePositionZSlider.value + ")"
                    }

                    Slider {
                        id: probePositionXSlider
                        from: 0
                        value: 0
                        to: 500
                        stepSize: 1
                    }

                    Slider {
                        id: probePositionYSlider
                        from: 0
                        value: 0
                        to: 500
                        stepSize: 1
                    }

                    Slider {
                        id: probePositionZSlider
                        from: 0
                        value: 0
                        to: 500
                        stepSize: 1
                    }

                    Item { width: 1; height: 10 }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Particles"
                        font.pixelSize: 14
                    }

                    CheckBox {
                        id: particleSystemSpriteCheckBox
                        checked: false
                        text: qsTr("Sprite Particles")
                    }

                    CheckBox {
                        id: particleSystemModelCheckBox
                        checked: false
                        text: qsTr("Model Particles")
                    }

                    Item { width: 1; height: 10 }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Model"
                        font.pixelSize: 14
                    }

                    Item { width: 1; height: 10 }

                    Text {
                        text: "Sphere Roughness : " + materialRoughnessSlider.value.toFixed(2);
                    }

                    Slider {
                        id: materialRoughnessSlider
                        from: 0
                        to: 1.0
                        value: 0.1
                    }

                    CheckBox {
                        id: sphereReceivesReflectionsCheckBox
                        checked: true
                        text: qsTr("Sphere receives reflections")
                    }

                    CheckBox {
                        id: floorReceivesReflectionsCheckBox
                        checked: true
                        text: qsTr("Floor receives reflections")
                    }
                }
            }
        }
    }
}
