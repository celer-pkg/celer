// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Qt.labs.platform
import QtCore

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

Window {
    id: windowRoot
    visible: true
    width: 1280
    height: 720

    property url importUrl;

    //! [base scene]
    View3D {
        id: view3D
        anchors.fill: parent
        environment: SceneEnvironment {
            id: env
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData{}
            }
            InfiniteGrid {
                visible: helper.gridEnabled
                gridInterval: helper.gridInterval
            }
        }

        camera: helper.orbitControllerEnabled ? orbitCamera : wasdCamera

        DirectionalLight {
            eulerRotation.x: -35
            eulerRotation.y: -90

            castsShadow: true
        }

        Node {
            id: orbitCameraNode
            PerspectiveCamera {
                id: orbitCamera
            }
        }

        PerspectiveCamera {
            id: wasdCamera
            onPositionChanged: {
                // Near/far logic copied from OrbitController
                let distance = position.length()
                if (distance < 1) {
                    clipNear = 0.01
                    clipFar = 100
                } else if (distance < 100) {
                    clipNear = 0.1
                    clipFar = 1000
                } else {
                    clipNear = 1
                    clipFar = 10000
                }
            }
        }

    //! [base scene]
        function resetView() {
            if (importNode.status === RuntimeLoader.Success) {
                helper.resetController()
            }
        }

        //! [instancing]
        RandomInstancing {
            id: instancing
            instanceCount: 30
            position: InstanceRange {
                property alias boundsDiameter: helper.boundsDiameter
                from: Qt.vector3d(-3*boundsDiameter, -3*boundsDiameter, -3*boundsDiameter);
                to: Qt.vector3d(3*boundsDiameter, 3*boundsDiameter, 3*boundsDiameter)
            }
            color: InstanceRange { from: "black"; to: "white" }
        }
        //! [instancing]

        QtObject {
            id: helper
            property real boundsDiameter: 0
            property vector3d boundsCenter
            property vector3d boundsSize
            property bool orbitControllerEnabled: true
            property bool gridEnabled: gridButton.checked
            property real cameraDistance: orbitControllerEnabled ? orbitCamera.z : wasdCamera.position.length()
            property real gridInterval: Math.pow(10, Math.round(Math.log10(cameraDistance)) - 1)

            function updateBounds(bounds) {
                boundsSize = Qt.vector3d(bounds.maximum.x - bounds.minimum.x,
                                         bounds.maximum.y - bounds.minimum.y,
                                         bounds.maximum.z - bounds.minimum.z)
                boundsDiameter = Math.max(boundsSize.x, boundsSize.y, boundsSize.z)
                boundsCenter = Qt.vector3d((bounds.maximum.x + bounds.minimum.x) / 2,
                                           (bounds.maximum.y + bounds.minimum.y) / 2,
                                           (bounds.maximum.z + bounds.minimum.z) / 2 )

                wasdController.speed = boundsDiameter / 1000.0
                wasdController.shiftSpeed = 3 * wasdController.speed
                wasdCamera.clipNear = boundsDiameter / 100
                wasdCamera.clipFar = boundsDiameter * 10
                view3D.resetView()
            }

            function resetController() {
                orbitCameraNode.eulerRotation = Qt.vector3d(0, 0, 0)
                orbitCameraNode.position = boundsCenter
                orbitCamera.position = Qt.vector3d(0, 0, 2 * helper.boundsDiameter)
                orbitCamera.eulerRotation = Qt.vector3d(0, 0, 0)
                orbitControllerEnabled = true
            }

            function switchController(useOrbitController) {
                if (useOrbitController) {
                    let wasdOffset = wasdCamera.position.minus(boundsCenter)
                    let wasdDistance = wasdOffset.length()
                    let wasdDistanceInPlane = Qt.vector3d(wasdOffset.x, 0, wasdOffset.z).length()
                    let yAngle = Math.atan2(wasdOffset.x, wasdOffset.z) * 180 / Math.PI
                    let xAngle = -Math.atan2(wasdOffset.y, wasdDistanceInPlane) * 180 / Math.PI

                    orbitCameraNode.position = boundsCenter
                    orbitCameraNode.eulerRotation = Qt.vector3d(xAngle, yAngle, 0)
                    orbitCamera.position = Qt.vector3d(0, 0, wasdDistance)

                    orbitCamera.eulerRotation = Qt.vector3d(0, 0, 0)
                } else {
                    wasdCamera.position = orbitCamera.scenePosition
                    wasdCamera.rotation = orbitCamera.sceneRotation
                    wasdController.focus = true
                }
                orbitControllerEnabled = useOrbitController
            }
        }

        //! [runtimeloader]
        RuntimeLoader {
            id: importNode
            source: windowRoot.importUrl
            instancing: instancingButton.checked ? instancing : null
            onBoundsChanged: helper.updateBounds(bounds)
        }
        //! [runtimeloader]

        //! [bounds]
        Model {
            parent: importNode
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "red"
            }
            opacity: 0.2
            visible: visualizeButton.checked && importNode.status === RuntimeLoader.Success
            position: helper.boundsCenter
            scale: Qt.vector3d(helper.boundsSize.x / 100,
                               helper.boundsSize.y / 100,
                               helper.boundsSize.z / 100)
        }
        //! [bounds]

        //! [status report]
        Rectangle {
            id: messageBox
            visible: importNode.status !== RuntimeLoader.Success
            color: "red"
            width: parent.width * 0.8
            height: parent.height * 0.8
            anchors.centerIn: parent
            radius: Math.min(width, height) / 10
            opacity: 0.6
            Text {
                anchors.fill: parent
                font.pixelSize: 36
                text: "Status: " + importNode.errorString + "\nPress \"Import...\" to import a model"
                color: "white"
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
        //! [status report]
    }

    //! [camera control]
    OrbitCameraController {
        id: orbitController
        origin: orbitCameraNode
        camera: orbitCamera
        enabled: helper.orbitControllerEnabled
    }
    WasdController {
        id: wasdController
        controlledObject: wasdCamera
        enabled: !helper.orbitControllerEnabled
    }
    //! [camera control]

    Pane {
        width: parent.width
        contentHeight: controlsLayout.implicitHeight
        RowLayout {
            id: controlsLayout
            Button {
                id: importButton
                text: "Import..."
                onClicked: fileDialog.open()
                focusPolicy: Qt.NoFocus
            }
            Button {
                id: resetButton
                text: "Reset view"
                onClicked: view3D.resetView()
                focusPolicy: Qt.NoFocus
            }
            Button {
                id: visualizeButton
                checkable: true
                text: "Visualize bounds"
                focusPolicy: Qt.NoFocus
            }
            Button {
                id: instancingButton
                checkable: true
                text: "Instancing"
                focusPolicy: Qt.NoFocus
            }
            Button {
                id: gridButton
                text: "Show grid"
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: false
            }
            Button {
                id: controllerButton
                text: helper.orbitControllerEnabled ? "Orbit" : "WASD"
                onClicked: helper.switchController(!helper.orbitControllerEnabled)
                focusPolicy: Qt.NoFocus
            }
            RowLayout {
                Label {
                    text: "Material Override"
                }
                ComboBox {
                    id: materialOverrideComboBox
                    textRole: "text"
                    valueRole: "value"
                    implicitContentWidthPolicy: ComboBox.WidestText
                    onActivated: env.debugSettings.materialOverride = currentValue

                    Component.onCompleted: materialOverrideComboBox.currentIndex = materialOverrideComboBox.indexOfValue(env.debugSettings.materialOverride)

                    model: [
                        { value: DebugSettings.None, text: "None"},
                        { value: DebugSettings.BaseColor, text: "Base Color"},
                        { value: DebugSettings.Roughness, text: "Roughness"},
                        { value: DebugSettings.Metalness, text: "Metalness"},
                        { value: DebugSettings.Diffuse, text: "Diffuse"},
                        { value: DebugSettings.Specular, text: "Specular"},
                        { value: DebugSettings.ShadowOcclusion, text: "Shadow Occlusion"},
                        { value: DebugSettings.Emission, text: "Emission"},
                        { value: DebugSettings.AmbientOcclusion, text: "Ambient Occlusion"},
                        { value: DebugSettings.Normals, text: "Normals"},
                        { value: DebugSettings.Tangents, text: "Tangents"},
                        { value: DebugSettings.Binormals, text: "Binormals"},
                        { value: DebugSettings.F0, text: "F0"}
                    ]
                }
            }
            CheckBox {
                text: "Wireframe"
                checked: env.debugSettings.wireframeEnabled
                onCheckedChanged: {
                    env.debugSettings.wireframeEnabled = checked
                }
            }
        }
    }
    FileDialog {
        id: fileDialog
        nameFilters: ["glTF files (*.gltf *.glb)", "All files (*)"]
        onAccepted: windowRoot.importUrl = file
        Settings {
            id: fileDialogSettings
            category: "QtQuick3D.Examples.RuntimeLoader"
            property alias folder: fileDialog.folder
        }
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: view3D
                id: dbg
                visible: false
            }
        }
    }
}
