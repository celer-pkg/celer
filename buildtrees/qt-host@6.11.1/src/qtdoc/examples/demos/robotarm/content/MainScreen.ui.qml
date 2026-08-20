// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Layouts
import Backend
import QtQml

Pane {
    id: root

    readonly property bool mobile: Qt.platform.os === "android"
    readonly property bool horizontal: width > height
    property real sliderWidth: width * 0.15
    property real buttonRowWidth: width * 0.12
    property real buttonMinWidth: 65
    readonly property alias darkMode: darkModeToggle.checked

    leftPadding: 60
    rightPadding: 60
    topPadding: 50
    bottomPadding: 50

    width: 800
    height: 600
    state: "mobileHorizontal"

    Backend {
        id: backend
        rotation1Angle: rotation1Slider.value
        rotation2Angle: rotation2Slider.value
        rotation3Angle: rotation3Slider.value
        rotation4Angle: rotation4Slider.value
        clawsAngle: clawToggle.checked ? 0 : 90
    }

    Toggle {
        id: darkModeToggle
        text: qsTr("Dark mode")
        anchors.top: parent.top
    }

    ColumnLayout {
        id: slidersColumn
        spacing: 6
        anchors.bottom: parent.bottom

        LabeledSlider {
            id: rotation1Slider
            sliderWidth: root.sliderWidth
            Layout.minimumWidth: 160
            labelText: qsTr("Rotation 1")
            from: -90
            to: 90
            value: 60
        }

        LabeledSlider {
            id: rotation2Slider
            sliderWidth: root.sliderWidth
            Layout.minimumWidth: 160
            labelText: qsTr("Rotation 2")
            from: -135
            to: 135
            value: 45
        }

        LabeledSlider {
            id: rotation3Slider
            sliderWidth: root.sliderWidth
            Layout.minimumWidth: 160
            labelText: qsTr("Rotation 3")
            from: -90
            to: 90
            value: 45
        }

        LabeledSlider {
            id: rotation4Slider
            sliderWidth: root.sliderWidth
            Layout.minimumWidth: 160
            labelText: qsTr("Rotation 4")
            from: -180
            to: 180
        }
    }

    Toggle {
        id: clawToggle
        text: qsTr("Claw")
        anchors.bottom: slidersColumn.top
        anchors.bottomMargin: 30
    }

    GridLayout {
        id: buttonsRow
        columns: 2
        rows: 2
        columnSpacing: 16
        rowSpacing: 8
        anchors.bottom: clawToggle.top
        anchors.bottomMargin: 30

        Button {
            id: pose1
            text: qsTr("Pose 1")
            Layout.preferredWidth: root.buttonRowWidth / 2
            Layout.minimumWidth: root.buttonMinWidth
            Layout.preferredHeight: 45

            Connections {
                target: pose1
                function onClicked() {
                    rotation1Slider.value = 30
                    rotation2Slider.value = 60
                    rotation3Slider.value = 90
                    rotation4Slider.value = 145
                }
            }
        }

        Button {
            id: pose2
            text: qsTr("Pose 2")
            Layout.preferredWidth: root.buttonRowWidth / 2
            Layout.minimumWidth: root.buttonMinWidth
            Layout.preferredHeight: 45

            Connections {
                target: pose2
                function onClicked() {
                    rotation1Slider.value = 60
                    rotation2Slider.value = 45
                    rotation3Slider.value = 45
                    rotation4Slider.value = 60
                }
            }
        }

        Button {
            id: pose3
            text: qsTr("Pose 3")
            Layout.preferredWidth: root.buttonRowWidth / 2
            Layout.minimumWidth: root.buttonMinWidth
            Layout.preferredHeight: 45

            Connections {
                target: pose3
                function onClicked() {
                    rotation1Slider.value = -90
                    rotation2Slider.value = -60
                    rotation3Slider.value = -45
                    rotation4Slider.value = -180
                }
            }
        }

        Button {
            id: resetPose
            text: qsTr("Reset")
            Layout.preferredWidth: root.buttonRowWidth / 2
            Layout.minimumWidth: root.buttonMinWidth
            Layout.preferredHeight: 45

            Connections {
                target: resetPose
                function onClicked() {
                    rotation1Slider.value = 0
                    rotation2Slider.value = 0
                    rotation3Slider.value = 0
                    rotation4Slider.value = 0
                    clawToggle.checked = false
                }
            }
        }
    }

    View3D {
        anchors.fill: parent

        camera: perspectiveCamera
        Node {
            PointLight {
                x: 760
                z: 770
                quadraticFade: 0
                brightness: 1
            }

            DirectionalLight {
                eulerRotation.z: 30
                eulerRotation.y: -165
            }

            DirectionalLight {
                y: 1000
                brightness: 0.4
                eulerRotation.z: -180
                eulerRotation.y: 90
                eulerRotation.x: -90
            }

            PerspectiveCamera {
                id: perspectiveCamera
                x: 1050
                y: 375
                z: -40
                pivot.x: 200
                eulerRotation.y: 85
            }
            RoboticArm {
                id: roboArm
                rotation1: backend.rotation1Angle
                rotation2: backend.rotation2Angle
                rotation3: backend.rotation3Angle
                rotation4: backend.rotation4Angle
                clawsAngle: backend.clawsAngle
            }
        }

        NodeIndicator {
            scenePosition: roboArm.hand_position
            isFocused: clawToggle.hasFocus
            label: clawToggle.text
            size: 30
        }

        NodeIndicator {
            scenePosition: roboArm.hand_hinge_position
            isFocused: rotation1Slider.activeFocus
            label: rotation1Slider.labelText
            size: 40
        }

        NodeIndicator {
            scenePosition: roboArm.arm_position
            isFocused: rotation2Slider.activeFocus
            label: rotation2Slider.labelText
            size: 50
        }

        NodeIndicator {
            scenePosition: roboArm.forearm_position
            isFocused: rotation3Slider.activeFocus
            label: rotation3Slider.labelText
            size: 60
        }

        NodeIndicator {
            scenePosition: roboArm.root_position
            isFocused: rotation4Slider.activeFocus
            label: rotation4Slider.labelText
            size: 60
        }

        environment: sceneEnvironment

        SceneEnvironment {
            id: sceneEnvironment
            antialiasingQuality: SceneEnvironment.VeryHigh
            antialiasingMode: SceneEnvironment.MSAA
        }
    }

    Label {
        id: robotStatus
        text: backend.status
        anchors.top: parent.top
        font.italic: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 15
    }

    states: [
        State {
            name: "mobileHorizontal"
            when: root.mobile && root.horizontal

            PropertyChanges {
                target: root
                leftPadding: 45
                topPadding: 15
                bottomPadding: 0
                sliderWidth: width * 0.4
                buttonRowWidth: width * 0.2
                buttonMinWidth: 75
            }

            PropertyChanges {
                target: roboArm
                z: -200
            }
        },
        State {
            name: "desktopVertical"
            when: !root.mobile && !root.horizontal

            PropertyChanges {
                target: root
                sliderWidth: width * 0.4
                buttonRowWidth: width * 0.2
                bottomPadding: 20
            }
            AnchorChanges {
                target: slidersColumn
                anchors.right: slidersColumn.parent.right
            }
            PropertyChanges {
                target: slidersColumn
                anchors.rightMargin: 20
            }

            AnchorChanges {
                target: buttonsRow
                anchors.bottom: undefined
                anchors.top: slidersColumn.top
            }

            AnchorChanges {
                target: clawToggle
                anchors.bottom: undefined
                anchors.top: buttonsRow.bottom
            }

            PropertyChanges {
                target: roboArm
                scale.x: 0.7
                scale.y: 0.7
                scale.z: 0.7
                y: 250
                z: 150
            }
        },
        State {
            name: "mobileVertical"
            when: root.mobile && !root.horizontal

            PropertyChanges {
                target: root
                sliderWidth: width * 0.85
                topPadding: 15
                leftPadding: 45
                bottomPadding: 0
                buttonRowWidth: width * 0.2
                buttonMinWidth: 75
            }

            AnchorChanges {
                target: slidersColumn
                anchors.left: undefined
                anchors.horizontalCenter: slidersColumn.parent.horizontalCenter
            }

            AnchorChanges {
                target: clawToggle
                anchors.left: undefined
                anchors.right: slidersColumn.right
            }

            AnchorChanges {
                target: buttonsRow
                anchors.bottom: slidersColumn.top
                anchors.left: slidersColumn.left
            }

            PropertyChanges {
                target: roboArm
                scale.x: 0.7
                scale.y: 0.7
                scale.z: 0.7
                y: 280
                z: 100
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation {
            properties: "sliderWidth, scale.x, scale.y, scale.z, y, z"
        }
        AnchorAnimation {}
    }
}
