// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    id: window
    width: 1800
    height: 1200
    visible: true
    title: qsTr("Manual Test SSR")

    property var ssr: null

    // Since SsrEffect is private in ExtendedSceneEnvironment we do this
    // roundabout way so we can tweak its private properties
    function findSSR() {
        const env = v3d.environment
        if (!env) return null

        for (let i = 0; i < env.children.length; i++) {
            let c = env.children[i]
            // check type name
            if (c && c.toString().indexOf("SsrEffect") !== -1) {
                window.ssr = c
                return window.ssr
            }
        }

        console.warn("SSR not found")
        return null
    }

    Component.onCompleted: findSSR()

    SplitView {
        anchors.fill: parent
        Pane {
            visible: true
            SplitView.minimumWidth: 250
            ScrollView {
                id: scrollView
                anchors.fill: parent
                ColumnLayout {
                    id: column
                    width: scrollView.availableWidth

                    CheckBox {
                        id: cbSSREnabled
                        checked: true
                        text: "SSR Enabled"
                    }

                    CheckBox {
                        id: cbMSAA
                        checked: true
                        text: "MSAA"
                    }

                    GroupBox {
                        title: "SSR"
                        ColumnLayout {
                            Layout.fillWidth: true

                            GroupBox {
                                title: "Global"
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "roughnessCut" }
                                        Slider {
                                            from: 0
                                            to: 1
                                            stepSize: 0.01
                                            value: window.ssr ? window.ssr.roughnessCut : 0.0
                                            onValueChanged: if (window.ssr) window.ssr.roughnessCut = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.roughnessCut.toFixed(2) : "0.00"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        CheckBox {
                                            id: checkboxRunning
                                            text: "Run continously"
                                            checked: true
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        CheckBox {
                                            id: checkboxAnimate
                                            text: "Animate Logo"
                                            checked: true
                                        }
                                    }
                                }
                            }

                            GroupBox {
                                title: "Main pass"
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "stepSize" }
                                        Slider {
                                            from: 0
                                            to: 0.1
                                            stepSize: 0.001
                                            value: window.ssr ? window.ssr.stepSize : 0.0
                                            onValueChanged: if (window.ssr) window.ssr.stepSize = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.stepSize.toFixed(3) : "0.000"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "minRayStep" }
                                        Slider {
                                            from: 0
                                            to: 0.1
                                            stepSize: 0.001
                                            value: window.ssr ? window.ssr.minRayStep : 0.0
                                            onValueChanged: if (window.ssr) window.ssr.minRayStep = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.minRayStep.toFixed(3) : "0.000"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "maxSteps" }
                                        Slider {
                                            from: 10
                                            to: 2048
                                            stepSize: 1.0
                                            value: window.ssr ? window.ssr.maxSteps : 2048
                                            onValueChanged: if (window.ssr) window.ssr.maxSteps = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.maxSteps.toFixed(0) : "2048"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "binarySteps" }
                                        Slider {
                                            from: 0
                                            to: 128
                                            stepSize: 1.0
                                            value: window.ssr ? window.ssr.binarySteps : 0
                                            onValueChanged: if (window.ssr) window.ssr.binarySteps = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.binarySteps.toFixed(0) : "0"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label { text: "baseThickness" }
                                        Slider {
                                            from: 0
                                            to: 50
                                            stepSize: 0.001
                                            value: window.ssr ? window.ssr.baseThickness : 0
                                            onValueChanged: if (window.ssr) window.ssr.baseThickness = value
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: window.ssr ? window.ssr.baseThickness.toFixed(3) : "0"
                                            Layout.minimumWidth: 50
                                        }
                                    }
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: "Lights"
                        ColumnLayout {
                            Layout.fillWidth: true
                            GroupBox {
                                title: "Directional Light"
                                ColumnLayout {
                                    CheckBox {
                                        text: "Enabled"
                                        checked: dirLight.visible
                                        onCheckedChanged:  dirLight.visible = checked
                                    }
                                    Label { text: "Brightness" }

                                    RowLayout {
                                        Slider {
                                            id: dlBrightnessSlider
                                            value: dirLight.brightness
                                            from: 0
                                            to: 10
                                            onValueChanged: dirLight.brightness = value
                                        }
                                        Text { text: dlBrightnessSlider.value.toFixed(0) }
                                    }

                                    Label { text: "Rotation" }

                                    RowLayout {
                                        Slider {
                                            id: lightXRotationSlider
                                            value: dirLight.eulerRotation.x
                                            from: -360
                                            to: 360
                                            onValueChanged: dirLight.eulerRotation.x = value
                                        }
                                        Text { text: lightXRotationSlider.value.toFixed(0) }
                                    }
                                    RowLayout {
                                        Slider {
                                            id: lightYRotationSlider
                                            value: dirLight.eulerRotation.y
                                            from: -360
                                            to: 360
                                            onValueChanged: dirLight.eulerRotation.y = value
                                        }
                                        Text { text: lightYRotationSlider.value.toFixed(0) }
                                    }
                                }
                            }

                            GroupBox {
                                title: "Point Light"
                                ColumnLayout {
                                    CheckBox {
                                        text: "Enabled"
                                        checked: pointLight.visible
                                        onCheckedChanged:  pointLight.visible = checked
                                    }

                                    Label { text: "Brightness" }
                                    RowLayout {
                                        Slider {
                                            value: pointLight.brightness
                                            from: 0
                                            to: 10
                                            onValueChanged: pointLight.brightness = value
                                        }
                                        Text { text: pointLight.brightness.toFixed(0) }
                                    }
                                    Label { text: "Constant fade" }
                                    RowLayout {
                                        Slider {
                                            value: pointLight.constantFade
                                            from: 0
                                            to: 2
                                            stepSize: 0.01
                                            onValueChanged: pointLight.constantFade = value
                                        }
                                        Text { text: pointLight.constantFade.toFixed(2) }
                                    }
                                    Label { text: "Linear fade" }
                                    RowLayout {
                                        Slider {
                                            value: pointLight.linearFade
                                            from: 0
                                            to: 2
                                            stepSize: 0.01
                                            onValueChanged: pointLight.linearFade = value
                                        }
                                        Text { text: pointLight.linearFade.toFixed(2) }
                                    }
                                    Label { text: "Quadratic fade" }
                                    RowLayout {
                                        Slider {
                                            value: pointLight.quadraticFade
                                            from: 0
                                            to: 2
                                            stepSize: 0.01
                                            onValueChanged: pointLight.quadraticFade = value
                                        }
                                        Text { text: pointLight.quadraticFade.toFixed(2) }
                                    }

                                    Label { text: "Position" }
                                    RowLayout {
                                        Slider {
                                            id: plXPositionSlider
                                            value: pointLight.position.x
                                            from: -50
                                            to: 50
                                            onValueChanged: pointLight.position.x = value
                                        }
                                        Text { text: plXPositionSlider.value.toFixed(0) }
                                    }
                                    RowLayout {
                                        Slider {
                                            id: plYPositionSlider
                                            value: pointLight.position.y
                                            from: 0
                                            to: 400
                                            onValueChanged: pointLight.position.y = value
                                        }
                                        Text { text: plYPositionSlider.value.toFixed(0) }
                                    }

                                    RowLayout {
                                        Slider {
                                            id: plZPositionSlider
                                            value: pointLight.position.z
                                            from: -50
                                            to: 50
                                            onValueChanged: pointLight.position.z = value
                                        }
                                        Text { text: plZPositionSlider.value.toFixed(0) }
                                    }
                                }
                            }
                        }
                    }

                    GroupBox {
                        title: "Logo Scene"
                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: "Roughness" }
                            RowLayout {
                                Slider {
                                    id: sceneRoughnessSlider
                                    value: 0.4
                                    from: 0
                                    to: 1
                                }
                                Text { text: sceneRoughnessSlider.value.toFixed(1) }
                            }
                            Label { text: "Metalness" }
                            RowLayout {
                                Slider {
                                    id: sceneMetalnessSlider
                                    value: 0.8
                                    from: 0
                                    to: 1
                                }
                                Text { text: sceneMetalnessSlider.value.toFixed(1) }
                            }
                        }
                    }
                }
            }
        }

        View3D {
            id: v3d
            SplitView.fillHeight: true
            SplitView.fillWidth: true

            environment: ExtendedSceneEnvironment {
                clearColor: "black"// "#808080"
                property Texture skyTexture: Texture { textureData:  ProceduralSkyTextureData {} }
                property Texture noTexture: Texture { }
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: skyTexture
                antialiasingMode: cbMSAA.checked ? SceneEnvironment.MSAA : SceneEnvironment.NoAA
                ssrEnabled: cbSSREnabled.checked
            }

            Model {
                source: "#Rectangle"
                y: -200
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.x: -90
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    roughness: sceneRoughnessSlider.value
                    metalness: sceneMetalnessSlider.value
                }
            }

            MovingRing {
                eulerRotation.x: 90
                material: PrincipledMaterial {
                    baseColor: "#2CDE85"
                    roughness: sceneRoughnessSlider.value
                    metalness: sceneMetalnessSlider.value
                }
                animate: checkboxAnimate.checked
            }

            Model {
                source: "#Cube"

                scale: Qt.vector3d(2, 2, 2)
                x: 300
                z: 300
                y: -100
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.9, 0.4, 0.4, 1.0)
                    roughness: sceneRoughnessSlider.value
                    metalness: sceneMetalnessSlider.value
                }
            }

            DirectionalLight {
                id: dirLight
                visible: true
                eulerRotation.x: -60
                castsShadow: true
                shadowFactor: 90
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityHigh
                softShadowQuality: Light.PCF8
                pcfFactor: 2
            }

            PointLight {
                id: pointLight
                visible: false
                castsShadow: true
                shadowFactor: 90
                shadowMapQuality: Light.ShadowMapQualityHigh
                softShadowQuality: Light.PCF8
                pcfFactor: 2
                brightness: 1
                y: 178
            }

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 100, 1200)
            }

            WasdController {
                controlledObject: camera
            }
        }

        FrameAnimation {
            id: frameAnimation
            running: checkboxRunning.checked
        }
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            color: "white"
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: v3d
                id: dbg
                visible: false
            }
        }
    }
}
