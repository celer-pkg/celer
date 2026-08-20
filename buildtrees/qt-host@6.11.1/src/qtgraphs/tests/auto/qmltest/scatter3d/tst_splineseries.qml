// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Spline3DSeries {
        id: initial
    }

    Gradient {
        id: gradient1;
        stops: [
            GradientStop { color: "red"; position: 0 },
            GradientStop { color: "blue"; position: 1 }
        ]
    }

    Gradient {
        id: gradient2;
        stops: [
            GradientStop { color: "green"; position: 0 },
            GradientStop { color: "red"; position: 1 }
        ]
    }

    Gradient {
        id: gradient3;
        stops: [
            GradientStop { color: "gray"; position: 0 },
            GradientStop { color: "darkgray"; position: 1 }
        ]
    }

    Spline3DSeries {
        id: initialized
        dataProxy: ItemModelScatterDataProxy {
            itemModel: ListModel {
                ListElement{ xPos: "2.754"; yPos: "1.455"; zPos: "3.362"; }
                ListElement{ xPos: "3.164"; yPos: "2.022"; zPos: "4.348"; }
            }
            xPosRole: "xPos"
            yPosRole: "yPos"
            zPosRole: "zPos"
        }
        itemSize: 0.5
        selectedItem: 0

        baseColor: "blue"
        baseGradient: gradient1
        colorStyle: GraphsTheme.ColorStyle.ObjectGradient
        itemLabelFormat: "%f"
        itemLabelVisible: false
        mesh: Abstract3DSeries.Mesh.Minimal
        meshRotation: Qt.quaternion(1, 1, 1, 1)
        meshSmooth: true
        multiHighlightColor: "green"
        multiHighlightGradient: gradient2
        name: "series1"
        singleHighlightColor: "red"
        singleHighlightGradient: gradient3
        userDefinedMesh: ":/customitem.obj"
        visible: false

        //spline properties
        splineVisible: false
        splineLooping: true
        splineTension: 1.0
        splineKnotting: 1.0
        splineColor: "blue"
        splineResolution: 5
    }

    ItemModelScatterDataProxy {
        id: proxy1
        itemModel: ListModel {
            ListElement{ xPos: "2.754"; yPos: "1.455"; zPos: "3.362"; }
            ListElement{ xPos: "3.164"; yPos: "2.022"; zPos: "4.348"; }
            ListElement{ xPos: "4.564"; yPos: "1.865"; zPos: "1.346"; }
        }
        xPosRole: "xPos"
        yPosRole: "yPos"
        zPosRole: "zPos"
    }

    Spline3DSeries {
        id: change
        dataProxy: proxy1
    }

    Spline3DSeries {
        id: invalid
    }

    TestCase {
        name: "Spline3DSeries Initial"

        function test_1_initial() {
            compare(initial.dataProxy.itemCount, 0)
            compare(initial.invalidSelectionIndex, -1)
            compare(initial.itemSize, 0.0)
            compare(initial.selectedItem, -1)

            //Spline properties
            compare(initial.splineVisible, true)
            compare(initial.splineLooping, false)
            compare(initial.splineTension, 0.0)
            compare(initial.splineKnotting, 0.5)
            compare(initial.splineColor, "#ff0000")
            compare(initial.splineResolution, 10)
        }

        function test_2_initial_common() {
            // Common properties
            compare(initial.baseColor, "#000000")
            verify(!initial.baseGradient)
            compare(initial.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(initial.itemLabel, "")
            compare(initial.itemLabelFormat, "@xLabel, @yLabel, @zLabel")
            compare(initial.itemLabelVisible, true)
            compare(initial.mesh, Abstract3DSeries.Mesh.Sphere)
            compare(initial.meshRotation, Qt.quaternion(1, 0, 0, 0))
            compare(initial.meshSmooth, false)
            compare(initial.multiHighlightColor, "#000000")
            verify(!initial.multiHighlightGradient)
            compare(initial.name, "")
            compare(initial.singleHighlightColor, "#000000")
            verify(!initial.singleHighlightGradient)
            compare(initial.type, Abstract3DSeries.SeriesType.Scatter)
            compare(initial.userDefinedMesh, "")
            compare(initial.visible, true)
        }
    }

    TestCase {
        name: "Spline3DSeries Initialized"

        function test_1_initialized() {
            compare(initialized.dataProxy.itemCount, 2)
            compare(initialized.itemSize, 0.5)
            compare(initialized.selectedItem, 0)

            //spline properties
            compare(initialized.splineVisible, false)
            compare(initialized.splineLooping, true)
            compare(initialized.splineTension, 1.0)
            compare(initialized.splineKnotting, 1.0)
            compare(initialized.splineColor, "#0000ff")
            compare(initialized.splineResolution, 5)
        }

        function test_2_initialized_common() {
            // Common properties
            compare(initialized.baseColor, "#0000ff")
            compare(initialized.baseGradient, gradient1)
            compare(initialized.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(initialized.itemLabelFormat, "%f")
            compare(initialized.itemLabelVisible, false)
            compare(initialized.mesh, Abstract3DSeries.Mesh.Minimal)
            compare(initialized.meshRotation, Qt.quaternion(1, 1, 1, 1))
            compare(initialized.meshSmooth, true)
            compare(initialized.multiHighlightColor, "#008000")
            compare(initialized.multiHighlightGradient, gradient2)
            compare(initialized.name, "series1")
            compare(initialized.singleHighlightColor, "#ff0000")
            compare(initialized.singleHighlightGradient, gradient3)
            compare(initialized.userDefinedMesh, ":/customitem.obj")
            compare(initialized.visible, false)
        }
    }

    TestCase {
        name: "Spline3DSeries Change"

        function test_1_change() {
            change.itemSize = 0.5
            change.selectedItem = 0

            //spline properties
            change.splineVisible = false
            change.splineLooping = true
            change.splineTension = 1.0
            change.splineKnotting = 1.0
            change.splineColor = "green"
            change.splineResolution = 15
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.dataProxy.itemCount, 3)
            compare(change.itemSize, 0.5)
            compare(change.selectedItem, 0)

            //spline properties
            compare(change.splineVisible, false)
            compare(change.splineLooping, true)
            compare(change.splineTension, 1.0)
            compare(change.splineKnotting, 1.0)
            compare(change.splineColor, "#008000")
            compare(change.splineResolution, 15)
        }

        function test_3_change_common() {
            change.baseColor = "blue"
            change.baseGradient = gradient1
            change.colorStyle = GraphsTheme.ColorStyle.ObjectGradient
            change.itemLabelFormat = "%f"
            change.itemLabelVisible = false
            change.mesh = Abstract3DSeries.Mesh.Minimal
            change.meshRotation = Qt.quaternion(1, 1, 1, 1)
            change.meshSmooth = true
            change.multiHighlightColor = "green"
            change.multiHighlightGradient = gradient2
            change.name = "series1"
            change.singleHighlightColor = "red"
            change.singleHighlightGradient = gradient3
            change.userDefinedMesh = ":/customitem.obj"
            change.visible = false

            compare(change.baseColor, "#0000ff")
            compare(change.baseGradient, gradient1)
            compare(change.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(change.itemLabelFormat, "%f")
            compare(change.itemLabelVisible, false)
            compare(change.mesh, Abstract3DSeries.Mesh.Minimal)
            compare(change.meshRotation, Qt.quaternion(1, 1, 1, 1))
            compare(change.meshSmooth, true)
            compare(change.multiHighlightColor, "#008000")
            compare(change.multiHighlightGradient, gradient2)
            compare(change.name, "series1")
            compare(change.singleHighlightColor, "#ff0000")
            compare(change.singleHighlightGradient, gradient3)
            compare(change.userDefinedMesh, ":/customitem.obj")
            compare(change.visible, false)
        }

        function test_4_change_gradient_stop() {
            gradient1.stops[0].color = "yellow"
            compare(change.baseGradient.stops[0].color, "#ffff00")
        }
    }
    TestCase {
        name: "Spline3DSeries Invalid"

        function test_invalid() {
            invalid.itemSize = -1.0
            compare(invalid.itemSize, 0.0)
            invalid.itemSize = 1.1
            compare(invalid.itemSize, 0.0)

            invalid.splineTension = -1.0
            compare(invalid.splineTension, 0.0)
            invalid.splineKnotting = -1.0
            compare(invalid.splineKnotting, 0.5)
            invalid.splineResolution = 1
            compare(invalid.splineResolution, 10)
        }
    }
}
