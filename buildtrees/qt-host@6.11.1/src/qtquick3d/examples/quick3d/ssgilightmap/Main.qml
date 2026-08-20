// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

import QtQuick.Controls
import QtQuick.Layouts

import "assets"

ApplicationWindow {
    width: 1480
    height: 900
    visible: true
    title: qsTr("QtQuick3D SSGI Lightmap Demo")
    id: window

    View3D {
        id: view
        anchors.fill: parent

        property Loader3D currentLoader: loaderCornell
        property SceneBase currentScene
        property url currentSource: "assets/CornellBox/CornellBox.qml"
        onCurrentSourceChanged: {
            updateLoadedScene()
        }

        environment: ExtendedSceneEnvironment {
            id: env
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        function updateLoadedScene() {
            posAnim.stop()
            rotAnim.stop()
            let loaders = [loaderCornell, loaderBedroom, loaderSponza]
            for (let i = 0; i < loaders.length; i++) {
                let loader = loaders[i]
                let item = loader.item
                if (item === null) {
                    continue
                }

                if (loader.source === view.currentSource) {
                    env.lightmapper = item.lightmapper
                    env.backgroundMode = item.backgroundMode
                    env.lightProbe = item.lightProbe
                    env.ssgiEnabled = buttonSSGI.checked
                    cameraNode.position = item.cameraPositions[item.cameraIndex]
                    cameraNode.eulerRotation = item.cameraRotations[item.cameraIndex]
                    item.enableLightmaps = buttonBaked.checked
                    env.ssgiIndirectLightBoost = item.ssgiIndirectLightBoost
                    env.probeExposure = item.probeExposure
                    item.visible = true
                    view.currentLoader = loader
                    view.currentScene = loader.item as SceneBase
                } else {
                    item.enableLightmaps = false
                    item.visible = false
                }
            }
        }

        Loader3D {
            id: loaderCornell
            asynchronous: true
            source: "assets/CornellBox/CornellBox.qml"
            onStatusChanged: () => {
                                 if (status === Loader3D.Ready)
                                 view.updateLoadedScene()
                             }
        }

        Loader3D {
            id: loaderBedroom
            asynchronous: true
            source: "assets/Bedroom/Bedroom.qml"
            onStatusChanged: () => {
                                 if (status === Loader3D.Ready)
                                 view.updateLoadedScene()
                             }
        }

        Loader3D {
            id: loaderSponza
            asynchronous: true
            source: "assets/Sponza/Sponza.qml"
            onStatusChanged: () => {
                                 if (status === Loader3D.Ready)
                                 view.updateLoadedScene()
                             }
        }

        WasdController {
            controlledObject: cameraNode
        }

        PerspectiveCamera {
            id: cameraNode
            z: 300
            y: 100
        }
    }

    RowLayout {
        id: buttonRow
        spacing: 12
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        IconButton {
            id: buttonSSGI
            text: "SSGI"
            conflictingButtons: [buttonOff, buttonBaked]
            iconSource: "qrc:/assets/icons/ssgi.svg"
            onCheckedChanged: {
                if (checked) {
                    env.ssgiEnabled = true
                    if (view.currentLoader.item) {
                        view.currentLoader.item.enableLightmaps = false
                    }
                }
            }
        }

        IconButton {
            id: buttonBaked
            checked: true
            text: "Baked"
            conflictingButtons: [buttonOff, buttonSSGI]
            iconSource: "qrc:/assets/icons/baked.svg"
            onCheckedChanged: {
                if (checked) {
                    env.ssgiEnabled = false
                    if (view.currentLoader.item) {
                        view.currentLoader.item.enableLightmaps = true
                    }
                }
            }
        }

        IconButton {
            id: buttonOff
            text: "Off"
            iconSource: "qrc:/assets/icons/off.svg"
            conflictingButtons: [buttonBaked, buttonSSGI]
            onCheckedChanged: {
                if (checked) {
                    env.ssgiEnabled = false
                    if (view.currentLoader.item) {
                        view.currentLoader.item.enableLightmaps = false
                    }
                }
            }
        }

        Rectangle {
            implicitWidth: 1
            implicitHeight: 60
            color: "#aaa"
            opacity: 0.4
        } // separator

        IconButton {
            id: demoButton
            text: "Demo"
            iconSource: "qrc:/assets/icons/reset-camera.svg"
            toggable: true
            onClicked: {
                if (demoButton.checked) {
                    window.nextCamera()
                }
            }
        }

        IconButton {
            text: "Next View"
            iconSource: "qrc:/assets/icons/right-arrow.svg"
            checkable: false
            onClicked: {
                window.nextCamera()
            }
        }

        Rectangle {
            implicitWidth: 1
            implicitHeight: 60
            color: "#aaa"
            opacity: 0.4
        } // separator

        IconButton {
            id: buttonBedroom
            text: "Bedroom"
            iconSource: "qrc:/assets/icons/bedroom.svg"
            conflictingButtons: [buttonCornell, buttonSponza]
            onCheckedChanged: {
                if (checked)
                    view.currentSource = "assets/Bedroom/Bedroom.qml"
            }
        }

        IconButton {
            id: buttonCornell
            text: "Cornell"
            iconSource: "qrc:/assets/icons/cornell.svg"
            checked: true
            conflictingButtons: [buttonBedroom, buttonSponza]
            onCheckedChanged: {
                if (checked)
                    view.currentSource = "assets/CornellBox/CornellBox.qml"
            }
        }

        IconButton {
            id: buttonSponza
            text: "Sponza"
            iconSource: "qrc:/assets/icons/sponza.svg"
            conflictingButtons: [buttonCornell, buttonBedroom]
            onCheckedChanged: {
                if (checked)
                    view.currentSource = "assets/Sponza/Sponza.qml"
            }
        }
    }

    function nextCamera() {
        if (!view.currentScene)
            return

        posAnim.stop()
        rotAnim.stop()

        let item = view.currentScene
        item.cameraIndex = (item.cameraIndex + 1) % item.cameraPositions.length
        posAnim.to = item.cameraPositions[item.cameraIndex]
        rotAnim.to = item.cameraRotations[item.cameraIndex]

        posAnim.start()
        rotAnim.start()
    }

    Timer {
        id: demoTimer
        interval: 7500
        repeat: true
        running: demoButton.checked
        onTriggered: {
            window.nextCamera()
        }
    }

    // Animations
    Vector3dAnimation {
        id: posAnim
        target: cameraNode
        property: "position"
        duration: demoButton.checked ? 2000 : 1000
        easing.type: Easing.InOutQuad
    }

    Vector3dAnimation {
        id: rotAnim
        target: cameraNode
        property: "eulerRotation"
        duration: demoButton.checked ? 2000 : 1000
        easing.type: Easing.InOutQuad
    }
}
