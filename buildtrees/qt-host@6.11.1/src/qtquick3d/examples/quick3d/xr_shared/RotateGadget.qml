// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

pragma ComponentBehavior: Bound

AxisGadget {
    source: "qrc:/qt/qml/xr_shared/meshes/rotate_mesh.mesh"

    property quaternion originalRot
    property vector3d pressOffset

    property vector3d rotationPlaneNormal
    property vector3d rotationAxis

    function getAngleFromRay(pos: vector3d, dir: vector3d) : real {
        const origin = scenePosition
        const point3D = linePlaneIntersection(origin, rotationPlaneNormal, pos, dir)

        const dragOffset = point3D.minus(origin).normalized()

        const cos = dragOffset.dotProduct(pressOffset)
        const sin = dragOffset.crossProduct(pressOffset).dotProduct(rotationPlaneNormal)

        return -Math.atan2(sin, cos)
    }

    function linePlaneIntersection(planePoint: vector3d, planeNormal: vector3d, linePoint: vector3d, lineDirection: vector3d): vector3d {
        let dot = planeNormal.dotProduct(lineDirection)

        if (dot === 0) {
            console.log("This should not happen")
            return planePoint
        }
        const t =  (planeNormal.dotProduct(planePoint) - planeNormal.dotProduct(linePoint)) / dot
        return linePoint.plus(lineDirection.times(t))
    }

    onPressed: (pos, dir) => {
        originalRot = controlledObject.rotation
        rotationAxis = controlledObject.mapDirectionToScene(axisDirection).normalized()

        const dot = rotationAxis.dotProduct(dir) // protect against rotation gadget being edge on:
        rotationPlaneNormal = Math.abs(dot) < 0.2 ? dir.times(-1) : rotationAxis

        const origin = scenePosition
        pressOffset = linePlaneIntersection(origin, rotationPlaneNormal, pos, dir).minus(origin).normalized()
    }

    onMoved: (pos, dir) => {
        const angle = getAngleFromRay(pos, dir)
        controlledObject.rotation = originalRot.times(Quaternion.fromAxisAndAngle(axisDirection, angle * 180 / Math.PI))
    }
}
