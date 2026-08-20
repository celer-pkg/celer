// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    id: root
    required property Node targetNode

    enum Axis {
        PositiveX,
        PositiveY,
        PositiveZ,
        NegativeX,
        NegativeY,
        NegativeZ
    }

    readonly property list<quaternion> rotations: [
        Qt.quaternion(Math.SQRT1_2, 0, Math.SQRT1_2, 0), // PositiveX
        Qt.quaternion(Math.SQRT1_2, -Math.SQRT1_2, 0, 0), // PositiveY
        Qt.quaternion(1, 0, 0, 0), // PositiveZ
        Qt.quaternion(Math.SQRT1_2, 0, -Math.SQRT1_2, 0), // NegativeX
        Qt.quaternion(Math.SQRT1_2, Math.SQRT1_2, 0, 0), // NegativeY
        Qt.quaternion(0, 0, 1, 0) // NegativeZ
    ]

    function quaternionForAxis(axis) {
        return rotations[axis]
    }

    signal axisClicked(int axis)
    signal ballMoved(vector2d velocity)

    QtObject {
        id: stylePalette
        property color white: "#fdf6e3"
        property color black: "#002b36"
        property color red: "#dc322f"
        property color green: "#859900"
        property color blue: "#268bd2"
        property color background: "#99002b36"
    }

    component LineRectangle : Rectangle {
        property vector2d startPoint: Qt.vector2d(0, 0)
        property vector2d endPoint: Qt.vector2d(0, 0)
        property real lineWidth: 5
        transformOrigin: Item.Left
        height: lineWidth

        readonly property vector2d offset: startPoint.plus(endPoint).times(0.5);

        width: startPoint.minus(endPoint).length()
        rotation: Math.atan2(endPoint.y - startPoint.y, endPoint.x - startPoint.x) * 180 / Math.PI
    }


    Rectangle {
        id: ballBackground
        anchors.centerIn: parent
        width: parent.width > parent.height ? parent.height : parent.width
        height: width
        radius: width / 2
        color: ballBackgroundHoverHandler.hovered ? stylePalette.background : "transparent"

        readonly property real subBallWidth: width / 5
        readonly property real subBallHalfWidth: subBallWidth * 0.5
        readonly property real subBallOffset: radius - subBallWidth / 2

        Item {
            anchors.centerIn: parent

            component SubBall : Rectangle {
                id: subBallRoot
                required property Node targetNode
                required property real offset

                property alias labelText: label.text
                property alias labelColor: label.color
                property alias labelVisible: label.visible
                property alias hovered: subBallHoverHandler.hovered
                property var initialPosition: Qt.vector3d(0, 0, 0)
                readonly property vector3d position: quaternionVectorMultiply(targetNode.sceneRotation.inverted(), initialPosition)

                signal tapped()

                function quaternionVectorMultiply(q, v) {
                    var qv = Qt.vector3d(q.x, q.y, q.z)
                    var uv = qv.crossProduct(v)
                    var uuv = qv.crossProduct(uv)
                    uv = uv.times(2.0 * q.scalar)
                    uuv = uuv.times(2.0)
                    return v.plus(uv).plus(uuv)
                }

                height: width
                radius: width / 2
                x: offset * position.x - width / 2
                y: offset * -position.y - height / 2
                z: position.z

                HoverHandler {
                    id: subBallHoverHandler
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: subBallRoot.tapped()
                }

                Text {
                    id: label
                    anchors.centerIn: parent
                }
            }

            SubBall {
                id: positiveX
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "X"
                labelColor: hovered ? stylePalette.white : stylePalette.black
                color: stylePalette.red
                initialPosition: Qt.vector3d(1, 0, 0)
                onTapped: {
                    let axis = OriginGizmo.Axis.PositiveX
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.PositiveX))
                        axis = OriginGizmo.Axis.NegativeX
                    root.axisClicked(axis)
                }
            }

            LineRectangle {
                endPoint: Qt.vector2d(positiveX.x + ballBackground.subBallHalfWidth, positiveX.y + ballBackground.subBallHalfWidth)
                color: stylePalette.red
                z: positiveX.z - 0.001
            }

            SubBall {
                id: positiveY
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "Y"
                labelColor: hovered ? stylePalette.white : stylePalette.black
                color: stylePalette.green
                initialPosition: Qt.vector3d(0, 1, 0)
                onTapped: {
                    let axis = OriginGizmo.Axis.PositiveY
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.PositiveY))
                        axis = OriginGizmo.Axis.NegativeY
                    root.axisClicked(axis)
                }
            }

            LineRectangle {
                endPoint: Qt.vector2d(positiveY.x + ballBackground.subBallHalfWidth, positiveY.y + ballBackground.subBallHalfWidth)
                color: stylePalette.green
                z: positiveY.z - 0.001
            }

            SubBall {
                id: positiveZ
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "Z"
                labelColor: hovered ? stylePalette.white : stylePalette.black
                color: stylePalette.blue
                initialPosition: Qt.vector3d(0, 0, 1)
                onTapped: {
                    let axis = OriginGizmo.Axis.PositiveZ
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.PositiveZ))
                        axis = OriginGizmo.Axis.NegativeZ
                    root.axisClicked(axis)
                }
            }

            LineRectangle {
                endPoint: Qt.vector2d(positiveZ.x + ballBackground.subBallHalfWidth, positiveZ.y + ballBackground.subBallHalfWidth)
                color: stylePalette.blue
                z: positiveZ.z - 0.001
            }

            SubBall {
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "-X"
                labelColor: stylePalette.white
                labelVisible: hovered
                color: Qt.rgba(stylePalette.red.r, stylePalette.red.g, stylePalette.red.b, z + 1 * 0.5)
                border.color: stylePalette.red
                border.width: 2
                initialPosition: Qt.vector3d(-1, 0, 0)
                onTapped: {
                    let axis = OriginGizmo.Axis.NegativeX
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.NegativeX))
                        axis = OriginGizmo.Axis.PositiveX
                    root.axisClicked(axis)
                }
            }

            SubBall {
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "-Y"
                labelColor: stylePalette.white
                labelVisible: hovered
                color: Qt.rgba(stylePalette.green.r, stylePalette.green.g, stylePalette.green.b, z + 1 * 0.5)
                border.color: stylePalette.green
                border.width: 2
                initialPosition: Qt.vector3d(0, -1, 0)
                onTapped: {
                    let axis = OriginGizmo.Axis.NegativeY
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.NegativeY))
                        axis = OriginGizmo.Axis.PositiveY
                    root.axisClicked(axis)
                }
            }

            SubBall {
                targetNode: root.targetNode
                width: ballBackground.subBallWidth
                offset: ballBackground.subBallOffset
                labelText: "-Z"
                labelColor: stylePalette.white
                labelVisible: hovered
                color: Qt.rgba(stylePalette.blue.r, stylePalette.blue.g, stylePalette.blue.b, z + 1 * 0.5)
                border.color: stylePalette.blue
                border.width: 2
                initialPosition: Qt.vector3d(0, 0, -1)
                onTapped: {
                    let axis = OriginGizmo.Axis.NegativeZ
                    if (root.targetNode.sceneRotation === root.quaternionForAxis(OriginGizmo.Axis.NegativeZ))
                        axis = OriginGizmo.Axis.PositiveZ
                    root.axisClicked(axis)
                }
            }
        }

        HoverHandler {
            id: ballBackgroundHoverHandler
            acceptedDevices: PointerDevice.Mouse
            cursorShape: Qt.PointingHandCursor
        }

        DragHandler {
            id: dragHandler
            target: null
            enabled: ballBackground.visible
            onCentroidChanged: {
                if (centroid.velocity.x > 0 && centroid.velocity.y > 0) {
                    root.ballMoved(centroid.velocity)
                }
            }
        }
    }
}
