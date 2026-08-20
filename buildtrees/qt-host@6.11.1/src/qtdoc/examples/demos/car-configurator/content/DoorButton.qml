// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "doorIcon"
import QtQuick.Controls
import QtQuick.Studio.DesignEffects
import QtQuick3D

Rectangle {
    id: doorButton
    width: 48
    height: 48
    opacity: 0.7
    color: "#00000000"
    radius: 8
    scale: 1
    property bool isChecked: checkBox.checked
    property bool isRendered: true
    property alias designEffectBackgroundLayer: designEffect.backgroundLayer
    required property url downloadBase
    required property var rootItem
    required property PerspectiveCamera camera
    required property vector3d trackedWorldPosition

    function updateState() {
        var scenePosition = camera.mapToViewport(trackedWorldPosition)
        doorButton.x = scenePosition.x * rootItem.width
        doorButton.y = scenePosition.y * rootItem.height
    }

    Component.onCompleted: updateState()

    Connections {
        target: doorButton.camera
        function onSceneTransformChanged() { doorButton.updateState() }
    }

    CheckBox {
        id: checkBox
        opacity: 0
        text: qsTr(" ")
        anchors.fill: parent
    }

    Connections {
        target: doorButton.rootItem
        function onWidthChanged() { doorButton.updateState() }
        function onHeightChanged() { doorButton.updateState() }
    }

    Door_button {
        id: door_button
        downloadBase: doorButton.downloadBase
    }
    FrameAnimation {
        running: true
        onTriggered: doorButton.updateState()
    }

    DesignEffect {
        id: designEffect
        backgroundBlurRadius: 10
        backgroundBlurVisible: true
    }

    states: [
        State {
            name: "rendered"
            when: doorButton.isRendered
        },
        State {
            name: "NotRendered"
            when: !doorButton.isRendered

            PropertyChanges {
                target: doorButton
                opacity: 0.223
                scale: 0
            }
        }
    ]

    transitions: [
        Transition {
            id: transition
            ParallelAnimation {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 567
                    }

                    PropertyAnimation {
                        target: doorButton
                        property: "opacity"
                        duration: 431
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: doorButton
                        property: "scale"
                        duration: 1032
                    }
                }
            }
            to: "*"
            from: "*"
        }
    ]
}
