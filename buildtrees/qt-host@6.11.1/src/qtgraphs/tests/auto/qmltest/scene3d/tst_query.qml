// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest 1.1

Item {
    id: top
    height: 150
    width: 150

    Scatter3D {
        id: invalid
        height: 150
        width: 150
        Scatter3DSeries {
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {x:"0"; y:"0"; z:"0"}
                    ListElement {x:"0"; y:"0"; z:"1"}
                    ListElement {x:"1"; y:"0"; z:"0"}
                    ListElement {x:"1"; y:"0"; z:"1"}
                }
                xPosRole: "x"
                yPosRole: "y"
                zPosRole: "z"
            }
            itemSize: 1.0
        }
    }

    Scatter3D {
        id: zero
        anchors.fill: parent
        Scatter3DSeries {
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {x:"0"; y:"0"; z:"0"}
                    ListElement {x:"0"; y:"0"; z:"1"}
                    ListElement {x:"1"; y:"0"; z:"0"}
                    ListElement {x:"1"; y:"0"; z:"1"}
                }
                xPosRole: "x"
                yPosRole: "y"
                zPosRole: "z"
            }
            itemSize: 1.0
        }
    }

    Scatter3D {
        id: valid
        height: 150
        width: 150
        property int middleX: width * 0.5
        property int middleY: height * 0.5
        measureFps: true // force rendering

        Scatter3DSeries {
            ItemModelScatterDataProxy {
                itemModel: ListModel {
                    ListElement {xPos:"0"; yPos:"0"; zPos:"0"}
                }
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
            }
            itemSize: 1.0
        }
    }

    TestCase {
        name: "Scene invalid query"
        function test_invalid_query() {
            waitForRendering(top)
            compare(invalid.scene.graphPositionQuery, Qt.point(-1, -1))
            compare(invalid.queriedGraphPosition, Qt.vector3d(0, 0, 0))
        }
    }

    TestCase {
        name: "Scene zero query"
        function test_zero_query() {
            zero.scene.graphPositionQuery = Qt.point(0, 0)
            waitForRendering(top)
            compare(zero.scene.graphPositionQuery, Qt.point(0, 0))
            compare(zero.queriedGraphPosition, Qt.vector3d(0, 0, 0))
        }
    }

    TestCase {
        name: "Scene valid query"
        when: valid.currentFps > 0
        function test_valid_query() {
            valid.scene.graphPositionQuery = Qt.point(valid.middleX + 1, valid.middleY)
            waitForPolish(top)
            waitForRendering(top)
            compare(graphPositionQuerySpy.count, 1)
            compare(valid.scene.graphPositionQuery, Qt.point(valid.middleX + 1, valid.middleY))
            verify(valid.queriedGraphPosition.fuzzyEquals(Qt.vector3d(2.68778, 0, 233.333)))
        }
    }

    SignalSpy {
        id: graphPositionQuerySpy
        target: valid.scene
        signalName: "graphPositionQueryChanged"
    }
}
