// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtGraphs

Item {
    id: mainView
    width: 1280
    height: 1024

    ListModel {
        id: data

        property int pointsPerSide: 15
        onPointsPerSideChanged: initalizeData()

        function initalizeData() {
            console.log("initalizing data")
            data.clear()
            for (let i = 0; i < pointsPerSide; i++) {
                for (let j = 0; j < pointsPerSide; j++) {
                    data.append({"xPos": i, "yPos":0, "zPos":j, "rotation":"", "scale":""})
                }
            }
        }
        function updateQuaternions(pos) {
            for (let i = 0; i < data.count; i++) {
                let pointPos = Qt.vector3d(data.get(i).xPos,
                0,
                data.get(i).zPos)


                var forward = pos.minus(pointPos).normalized()
                var forwardVector = Qt.vector3d(0,1,0)
                var dot = forwardVector.dotProduct(forward)
                var rotAngle = Math.acos(dot) / (2 * Math.PI);
                var rotAxis = forwardVector.crossProduct(forward).normalized()

                data.setProperty(i, "rotation", generateQuaternion(rotAxis, rotAngle))

                var dist = pos.minus(pointPos).length()
                data.setProperty(i, "yPos", (Math.sqrt(data.count) -dist))
                data.setProperty(i, "scale", generateScale(1, dist + 0.1, 1))
            }
        }

        function generateQuaternion(axis, angle) {
            return "@" + angle * 360 + "," + axis.x + ","
                    + axis.y + "," + axis.z;
        }

        function generateScale(x, y, z) {
            return x + "," + y + "," + z
        }

        Component.onCompleted: {
            initalizeData()
            updateQuaternions(Qt.vector3d(5,0,5))
        }
    }

    Gradient {
        id: scatterGradient
        GradientStop { position: 0.0; color: "blue" }
        GradientStop { id: middleGradient; position: 0.50; color: "green" }
        GradientStop { position: 1.0; color: "red" }
    }

    GraphsTheme {
        id: theme
            theme: GraphsTheme.Theme.OrangeSeries
            colorStyle: GraphsTheme.ColorStyle.RangeGradient
            baseGradients: [scatterGradient]
    }

    Scatter3D {
        id: scatterGraph
        anchors.fill:parent
        theme: theme


        Scatter3DSeries {
            id: scatterSeries

            itemSize: 0.1
            mesh: Abstract3DSeries.Mesh.Arrow

            onSelectedItemChanged: {
                if (selectedItem == -1)
                    return
                var i = selectedItem
                let pointPos = Qt.vector3d(data.get(i).xPos,
                0,
                data.get(i).zPos)
                data.updateQuaternions(pointPos)
            }
            ItemModelScatterDataProxy {
                id: scatterProxy
                itemModel: data
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
                rotationRole: "rotation"
                scaleRole: "scale"
            }
        }
    }
}
