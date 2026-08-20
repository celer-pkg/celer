// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: teleporter
    required property var rayPicker //any object that has implemented rayPick(pos, dir)
    required property Node cameraOrigin
    required property Node camera
    required property Node beamHandle
    required property CharacterController characterController
    property real cameraSnapRotation: 30
    property real xStickValue: 0
    property real yStickValue: 0
    property alias screenVisibility: screenValueFader.value
    property bool targetValid: false
    property color rayHitColor: "green"
    property color rayMissColor: "red"
    property int blinkSpeed: 150
    property real movementChangedThreshold: 5
    property real xzMovementTeleportationThreshold: 30
    property real yMovementTeleportationThreshold: 10

    property bool teleporting: false
    property bool rotating: false

    function teleportTo(position, byCharacterController = false) {
        teleporter.teleporting = true
        let offset = camera.scenePosition.minus(cameraOrigin.scenePosition)
        let cameraOriginPosition = position.minus(offset)
        cameraOriginPosition.y = position.y
        let characterControllerHeight = teleporter.characterController.scale.y * 50 + //half of the controller height
            50 * teleporter.characterController.scale.x //half of the cap of the controller capsule shape
        if (byCharacterController)
            cameraOriginPosition.y -= characterControllerHeight

        screenValueFader.blink(()=>{
                                   teleporter.doTeleportation(cameraOriginPosition,
                                                              //putting back the height of the character controller
                                                              Qt.vector3d(position.x, position.y + (byCharacterController ? 0 : characterControllerHeight), position.z))
                               },()=>{
                                   characterControllerLastYPosition = characterController.scenePosition.y
                                   teleporter.teleporting = false
                               }, teleporter.blinkSpeed)
    }

    function rotateBy(degrees) {
        teleporter.rotating = true
        let r = Quaternion.fromEulerAngles(0, degrees, 0)
        let origin = Qt.vector3d(camera.position.x, 0, camera.position.z)
        let mappedOrigin = cameraOrigin.rotation.times(origin).plus(cameraOrigin.position)
        let rotatedOrigin = r.times(origin)
        let mappedRO = cameraOrigin.rotation.times(rotatedOrigin).plus(cameraOrigin.position)
        let delta = mappedRO.minus(mappedOrigin)

        doRotation(cameraOrigin.rotation.times(r), cameraOrigin.position.minus(delta))
        teleporter.rotating = false
    }

    signal doTeleportation(var cameraOriginPosition, var characterControllerPosition)

    signal doRotation(var cameraOriginRotation, var cameraOriginPosition)

    readonly property bool xPlusRotation: xStickValue > 0.5
    onXPlusRotationChanged: {
        if (xPlusRotation)
            rotateBy(-cameraSnapRotation)
    }

    readonly property bool xMinusRotation: xStickValue < -0.5
    onXMinusRotationChanged: {
        if (xMinusRotation)
            rotateBy(cameraSnapRotation)
    }

    readonly property vector3d cameraPosition: camera.scenePosition
    onCameraPositionChanged: {
        var deltaXZ = teleporter.camera.scenePosition.minus(teleporter.characterController.scenePosition)
        deltaXZ.y = 0.0
        let deltaLen = deltaXZ.length()
        if (deltaLen >= teleporter.movementChangedThreshold &&
                teleporter.rotating == false &&
                teleporter.teleporting === false)
            characterController.movement = deltaXZ.times(10.)
        else
            characterController.movement = Qt.vector3d(0, 0, 0)

        if (deltaLen >= teleporter.xzMovementTeleportationThreshold &&
                teleporter.rotating == false &&
                teleporter.teleporting === false &&
                (characterController.collisions & CharacterController.Down) &&
                (characterController.collisions & CharacterController.Side) ) {
            teleportTo(characterControllerPosition, true)
        }
    }

    readonly property vector3d characterControllerPosition: teleporter.characterController.scenePosition
    property real characterControllerLastYPosition: 0
    onCharacterControllerPositionChanged: {
        let deltaY = characterControllerPosition.y - characterControllerLastYPosition
        if (Math.abs(deltaY) > teleporter.yMovementTeleportationThreshold &&
                teleporter.rotating === false &&
                teleporter.teleporting === false &&
                characterController.collisions == CharacterController.Down)
            teleportTo(characterControllerPosition, true)
    }

    ValueFader {
        id: screenValueFader
    }

    TargetIndicator {
        id: targetIndicator
    }

    BeamModel {
        id: beamModel
        color: teleporter.targetValid ? teleporter.rayHitColor : teleporter.rayMissColor
    }

    FrameAnimation {
        running: teleporter.yStickValue > 0.7
        onTriggered: {
            teleporter.updateTarget()
        }
        onRunningChanged: {
            if (running) {
                beamModel.show()
            }else {
                beamModel.hide()
                targetIndicator.hide()
                if (teleporter.targetValid)
                    teleporter.teleportTo(targetIndicator.scenePosition)
            }
        }
    }

    function updateTarget() : bool {
        // Not a pure gravity parabola: We want a flatter curve

        let beamPositions = [];
        let pos = beamHandle.scenePosition
        const dx = beamHandle.forward.x
        const dz = beamHandle.forward.z
        const a = Qt.vector3d(dx * 2, -4, dz * 2)
        let d = beamHandle.forward.times(50)
        let index = 0
        let hit = false
        let pickResult = null
        let penetrate = false
        let lastPos = Qt.vector3d(pos.x, pos.y, pos.z)

        let characterPos = characterController.scenePosition
        let handleToCharacterDir = pos.minus(characterPos)
        pickResult = teleporter.rayPicker.rayPick(characterPos, handleToCharacterDir.normalized())

        if (pickResult.objectHit && pickResult.distance <= handleToCharacterDir.length() + 25) {
            penetrate = true
            beamModel.hide()
        }else {
            beamModel.show()
            beamPositions.push(Qt.vector3d(pos.x, pos.y, pos.z))
            for (let i = 0; !hit && i < 50; ++i) {
                pos = pos.plus(d)
                d = d.plus(a)

                let lastPosToPos = pos.minus(lastPos)

                pickResult = teleporter.rayPicker.rayPick(lastPos, lastPosToPos.normalized())
                hit = pickResult.objectHit && pickResult.distance <= lastPosToPos.length() + 25

                beamPositions.push(Qt.vector3d(pos.x, pos.y, pos.z))
                lastPos = Qt.vector3d(pos.x, pos.y, pos.z)
            }
            beamModel.generate(beamPositions)
        }

        if (!penetrate && hit && pickResult.sceneNormal.normalized().y > 0.9) {
            teleporter.targetValid = true
            targetIndicator.moveTo(pickResult.scenePosition)
            targetIndicator.show()
        } else {
            teleporter.targetValid = false
            targetIndicator.hide()
        }

        return teleporter.targetValid
    }
}
