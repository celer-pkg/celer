// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("Lights Example")

    property bool isLandscape: width > height

    View3D {
        id: v3d

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: window.isLandscape ? settingsDrawer.right : parent.left
        anchors.top: window.isLandscape ? parent.top : settingsDrawer.bottom

        environment: SceneEnvironment {
            clearColor: "#808080"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 400, 600)
            eulerRotation.x: -30
            clipFar: 2000
        }

        //! [directional light]
        DirectionalLight {
            id: light1
            color: Qt.rgba(1.0, 0.1, 0.1, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            position: Qt.vector3d(0, 200, 0)
            rotation: Quaternion.fromEulerAngles(-135, -90, 0)
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            pcfFactor: 1
            visible: directionalLightCheckBox.checked
            castsShadow: checkBoxShadows.checked
            brightness: directionalLightSlider.value
            SequentialAnimation on rotation {
                loops: Animation.Infinite
                QuaternionAnimation {
                    to: Quaternion.fromEulerAngles(-45, -90, 0)
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                QuaternionAnimation {
                    to: Quaternion.fromEulerAngles(-135, -90, 0)
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }
        //! [directional light]

        //! [point light]
        PointLight {
            id: light2
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            position: Qt.vector3d(0, 300, 0)
            shadowMapFar: 2000
            shadowMapQuality: Light.ShadowMapQualityHigh
            visible: pointLightCheckBox.checked
            castsShadow: checkBoxShadows.checked
            brightness: pointLightSlider.value
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 400
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }
        //! [point light]

        //! [spot light]
        SpotLight {
            id: light4
            color: Qt.rgba(1.0, 0.9, 0.7, 1.0)
            ambientColor: Qt.rgba(0.0, 0.0, 0.0, 0.0)
            position: Qt.vector3d(0, 250, 0)
            eulerRotation.x: -45
            shadowMapFar: 2000
            shadowMapQuality: Light.ShadowMapQualityHigh
            visible: spotLightCheckBox.checked
            castsShadow: checkBoxShadows.checked
            brightness: spotLightSlider.value
            coneAngle: 110
            innerConeAngle: 70
            PropertyAnimation on eulerRotation.y {
                loops: Animation.Infinite
                from: 0
                to: -360
                duration: 10000
            }
        }
        //! [spot light]

        //! [rectangle models]
        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(15, 15, 15)
            eulerRotation.x: -90
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(0.8, 0.6, 0.4, 1.0)
                }
            ]
        }
        Model {
            source: "#Rectangle"
            z: -400
            scale: Qt.vector3d(15, 15, 15)
            materials: [
                 PrincipledMaterial {
                    baseColor: Qt.rgba(0.8, 0.8, 0.9, 1.0)
                }
            ]
        }
        //! [rectangle models]

        RotatingLogo {
            visible: !checkBoxCustomMaterial.checked
            material: PrincipledMaterial {
                baseColor: Qt.rgba(0.9, 0.9, 0.9, 1.0)
            }
            animate: checkBoxAnimate.checked
        }

        RotatingLogo {
            visible: checkBoxCustomMaterial.checked
            material: CustomMaterial {
                id: customMaterial
                vertexShader: "custom.vert"
                property real uAmplitude: 0.005
                property real uTime: 0.0
                SequentialAnimation {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation { target: customMaterial; property: "uTime"; from: 0.0; to: 10.0; duration: 10000 }
                    NumberAnimation { target: customMaterial; property: "uTime"; from: 10.0; to: 0.0; duration: 10000 }
                }
            }
            animate: checkBoxAnimate.checked
        }

        //! [light models]
        Model {
            // Directional Light Marker
            property real size: directionalLightSlider.highlight ? 0.2 : 0.1
            source: "#Cube"
            position: light1.position
            rotation: light1.rotation
            scale: Qt.vector3d(size, size, size * 2)
            materials: [
                PrincipledMaterial {
                    baseColor: light1.color
                    lighting: PrincipledMaterial.NoLighting
                }
            ]
            castsShadows: false
            visible: directionalLightCheckBox.checked
        }
        Model {
            // Point Light Marker
            source: "#Sphere"
            position: light2.position
            rotation: light2.rotation
            property real size: pointLightSlider.highlight ? 0.2 : 0.1
            scale: Qt.vector3d(size, size, size)
            materials: [
                PrincipledMaterial {
                    baseColor: light2.color
                    lighting: PrincipledMaterial.NoLighting
                }
            ]
            castsShadows: false
            visible: pointLightCheckBox.checked
        }
        Node {
            // Spot Light Marker
            position: light4.position
            rotation: light4.rotation
            property real size: spotLightSlider.highlight ? 0.2 : 0.1
            scale: Qt.vector3d(size, size, size)
            Model {
                source: "#Cone"
                castsShadows: false
                eulerRotation.x: 90
                materials: PrincipledMaterial {
                    baseColor: light4.color
                    lighting: PrincipledMaterial.NoLighting
                }
            }
            visible: spotLightCheckBox.checked
        }
        //! [light models]

        DebugView {
            id: dbg
            anchors.top: parent.top
            anchors.right: parent.right
            source: v3d
            visible: debugViewCheckbox.checked
        }
    }

    RowLayout {
        anchors.bottom: v3d.bottom
        anchors.left: v3d.left
        anchors.margins: 10
        Item {
            id: settingsButton
            implicitWidth: 64
            implicitHeight: 64
            Image {
                anchors.centerIn: parent
                source: "icon_settings.png"
            }

            HoverHandler {
                id: hoverHandler
            }
        }
        Text {
            Layout.alignment: Qt.AlignVCenter
            text: settingsDrawer.title
            visible: !settingsDrawer.isOpen
            color: "white"
            font.pointSize: 16
        }
        TapHandler {
            // qmllint disable signal-handler-parameters
            onTapped: settingsDrawer.isOpen = !settingsDrawer.isOpen;
            // qmllint enable signal-handler-parameters
        }
    }



    component SliderWithValue : RowLayout {
        property alias value: slider.value
        property alias from: slider.from
        property alias to: slider.to
        readonly property bool highlight: slider.hovered || slider.pressed
        Slider {
            id: slider
            stepSize: 0.01
            Layout.minimumWidth: 200
            Layout.maximumWidth: 200
        }
        Label {
            id: valueText
            text: slider.value.toFixed(2)
            Layout.minimumWidth: 80
            Layout.maximumWidth: 80
        }
    }

    SettingsDrawer {
        id: settingsDrawer
        title: qsTr("Settings")
        isLandscape: window.isLandscape
        width: window.isLandscape ? implicitWidth : window.width
        height: window.isLandscape ? window.height : window.height * 0.33

        CheckBox {
            id: checkBoxShadows
            text: qsTr("Enable Shadows")
            checked: true
        }
        CheckBox {
            id: checkBoxAnimate
            text: qsTr("Rotate Logo")
            checked: true
        }
        CheckBox {
            id: checkBoxCustomMaterial
            text: qsTr("Custom Material")
            checked: false
        }

        Label {
            // spacer
        }

        CheckBox {
            id: directionalLightCheckBox
            text: qsTr("Directional Light")
            checked: true
        }
        SliderWithValue {
            id: directionalLightSlider
            value: 0.5
            from: 0
            to: 1
            enabled: directionalLightCheckBox.checked
        }

        Label {
            // spacer
        }

        CheckBox {
            id: pointLightCheckBox
            text: qsTr("Point Light")
            checked: false
        }
        SliderWithValue {
            id: pointLightSlider
            value: 6
            from: 0
            to: 10
            enabled: pointLightCheckBox.checked
        }
        Label {
            // spacer
        }
        CheckBox {
            id: spotLightCheckBox
            text: qsTr("Spot Light")
            checked: false
        }
        SliderWithValue {
            id: spotLightSlider
            value: 10
            from: 0
            to: 30
            enabled: spotLightCheckBox.checked
        }
        Label {
            // spacer
        }
        CheckBox {
            id: debugViewCheckbox
            text: qsTr("Enable Debug View")
            checked: false
        }
    }
}
