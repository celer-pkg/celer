// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Model {
    id: root
    source: isSphere() ? "#Sphere" : "#Rectangle"
    property real size: 1
    property real contrast: 0.5

    property Node cameraNode: null
    property real scaleDistance: 100 // Cursor is scaled up when further away than this distance

    property real sf: size / 100 // private property

    opacity:  0.5

    scale: Qt.vector3d(sf, sf, sf)

    enum CursorStyle {
        Hidden,
        Flat,
        Sphere
    }
    property int cursorStyle: XrCursor.CursorStyle.Flat

    function isSphere() : bool {
        return cursorStyle === XrCursor.CursorStyle.Sphere
    }

    CustomMaterial {
        id: targetMaterial
        shadingMode: CustomMaterial.Unshaded
        cullMode: Material.BackFaceCulling
        vertexShader: "shaders/cursor.vert"
        fragmentShader: "shaders/cursor.frag"

        property real opacity: root.cursorStyle === XrCursor.CursorStyle.Hidden ? 0.0 : root.opacity
        property real contrast: root.contrast
    }

    PrincipledMaterial {
        id: sphereMaterial
        baseColor: "white"
    }

    materials: isSphere() ? sphereMaterial : targetMaterial

    function setPositionAndOrientation(scenePos : vector3d, sceneNormal : vector3d) {
        if (isSphere()) {
            position = scenePos
        } else {
            const newPos = scenePos.minus(sceneNormal.normalized().times(-0.1))
            rotation = Quaternion.lookAt(newPos, scenePos,
                                         Qt.vector3d(0, 0, -1), Qt.vector3d(0, 1, 0))
            position = newPos
        }
        if (cameraNode && scaleDistance) {
            const distance = scenePos.minus(cameraNode.position).length()
            const distanceScale = Math.max(1, distance / root.scaleDistance)
            sf = size * distanceScale / 100
        }
    }
}
