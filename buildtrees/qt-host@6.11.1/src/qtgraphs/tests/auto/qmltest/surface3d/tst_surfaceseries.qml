// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Surface3DSeries {
        id: initial
    }

    Gradient {
        id: gradient1
        stops: [
            GradientStop {
                color: "red"
                position: 0
            },
            GradientStop {
                color: "blue"
                position: 1
            }
        ]
    }

    Gradient {
        id: gradient2
        stops: [
            GradientStop {
                color: "green"
                position: 0
            },
            GradientStop {
                color: "red"
                position: 1
            }
        ]
    }

    Gradient {
        id: gradient3
        stops: [
            GradientStop {
                color: "gray"
                position: 0
            },
            GradientStop {
                color: "darkgray"
                position: 1
            }
        ]
    }

    Surface3DSeries {
        id: initialized
        dataProxy: ItemModelSurfaceDataProxy {
            itemModel: ListModel {
                ListElement {
                    longitude: "20"
                    latitude: "10"
                    pop_density: "4.75"
                }
                ListElement {
                    longitude: "21"
                    latitude: "10"
                    pop_density: "3.00"
                }
            }
            rowRole: "longitude"
            columnRole: "latitude"
            yPosRole: "pop_density"
        }
        drawMode: Surface3DSeries.DrawSurface
        shading: Surface3DSeries.Shading.Smooth
        selectedPoint: Qt.point(0, 0)
        textureFile: ":\customtexture.jpg"
        wireframeColor: "red"

        baseColor: "blue"
        baseGradient: gradient1
        colorStyle: GraphsTheme.ColorStyle.ObjectGradient
        itemLabelFormat: "%f"
        itemLabelVisible: false
        mesh: Abstract3DSeries.Mesh.Cube
        meshRotation: Qt.quaternion(1, 1, 1, 1)
        meshSmooth: true
        multiHighlightColor: "green"
        multiHighlightGradient: gradient2
        name: "series1"
        singleHighlightColor: "red"
        singleHighlightGradient: gradient3
        userDefinedMesh: ":/customitem.mesh"
        visible: false
    }

    ItemModelSurfaceDataProxy {
        id: proxy1
        itemModel: ListModel {
            ListElement {
                longitude: "20"
                latitude: "10"
                pop_density: "4.75"
            }
            ListElement {
                longitude: "21"
                latitude: "10"
                pop_density: "3.00"
            }
            ListElement {
                longitude: "22"
                latitude: "10"
                pop_density: "1.24"
            }
        }
        rowRole: "longitude"
        columnRole: "latitude"
        yPosRole: "pop_density"
    }

    Surface3DSeries {
        id: change
        dataProxy: proxy1
    }

    TestCase {
        name: "Surface3DSeries Initial"

        function test_1_initial() {
            compare(initial.dataProxy.rowCount, 0)
            compare(initial.invalidSelectionPosition, Qt.point(-1, -1))
            compare(initial.drawMode, Surface3DSeries.DrawSurfaceAndWireframe)
            compare(initial.shading, Surface3DSeries.Shading.Flat)
            compare(initial.flatShadingSupported, true)
            compare(initial.selectedPoint, Qt.point(-1, -1))
            compare(initial.wireframeColor, "#000000")
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
            compare(initial.type, Abstract3DSeries.SeriesType.Surface)
            compare(initial.userDefinedMesh, "")
            compare(initial.visible, true)
        }
    }

    TestCase {
        name: "Surface3DSeries Initialized"

        function test_1_initialized() {
            compare(initialized.dataProxy.rowCount, 2)
            compare(initialized.drawMode, Surface3DSeries.DrawSurface)
            compare(initialized.shading, Surface3DSeries.Shading.Smooth)
            compare(initialized.selectedPoint, Qt.point(0, 0))
            compare(initialized.textureFile, ":\customtexture.jpg")
            compare(initialized.wireframeColor, "#ff0000")
        }

        function test_2_initialized_common() {
            // Common properties
            compare(initialized.baseColor, "#0000ff")
            compare(initialized.baseGradient, gradient1)
            compare(initialized.colorStyle,
                    GraphsTheme.ColorStyle.ObjectGradient)
            compare(initialized.itemLabelFormat, "%f")
            compare(initialized.itemLabelVisible, false)
            compare(initialized.mesh, Abstract3DSeries.Mesh.Cube)
            compare(initialized.meshRotation, Qt.quaternion(1, 1, 1, 1))
            compare(initialized.meshSmooth, true)
            compare(initialized.multiHighlightColor, "#008000")
            compare(initialized.multiHighlightGradient, gradient2)
            compare(initialized.name, "series1")
            compare(initialized.singleHighlightColor, "#ff0000")
            compare(initialized.singleHighlightGradient, gradient3)
            compare(initialized.userDefinedMesh, ":/customitem.mesh")
            compare(initialized.visible, false)
        }
    }

    TestCase {
        name: "Surface3DSeries Change"

        function test_1_change() {
            change.drawMode = Surface3DSeries.DrawSurface
            change.shading = Surface3DSeries.Shading.Smooth
            change.selectedPoint = Qt.point(0, 0)
            change.textureFile = ":\customtexture.jpg"
            change.wireframeColor = "green"
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.dataProxy.rowCount, 3)
            compare(change.drawMode, Surface3DSeries.DrawSurface)
            compare(change.shading, Surface3DSeries.Shading.Smooth)
            compare(change.selectedPoint, Qt.point(0, 0))
            compare(change.textureFile, ":\customtexture.jpg")
            compare(change.wireframeColor, "#008000")

            // Signals
            compare(dataProxySpy.count, 1)
            compare(selectedPointSpy.count, 1)
            compare(flatShadingSupportedSpy.count, 0)
            compare(drawModeSpy.count, 1)
            compare(shadingSpy.count, 1)
            compare(textureSpy.count, 1)
            compare(textureFileSpy.count, 1)
            compare(wireFrameColorSpy.count, 1)
            compare(dataArraySpy.count, 1)
        }

        function test_3_change_common() {
            change.baseColor = "blue"
            change.baseGradient = gradient1
            change.colorStyle = GraphsTheme.ColorStyle.ObjectGradient
            change.itemLabelFormat = "%f"
            change.itemLabelVisible = false
            change.mesh = Abstract3DSeries.Mesh.Cube
            change.meshRotation = Qt.quaternion(1, 1, 1, 1)
            change.meshSmooth = true
            change.multiHighlightColor = "green"
            change.multiHighlightGradient = gradient2
            change.name = "series1"
            change.singleHighlightColor = "red"
            change.singleHighlightGradient = gradient3
            change.userDefinedMesh = ":/customitem.mesh"
            change.visible = false

            compare(change.baseColor, "#0000ff")
            compare(change.baseGradient, gradient1)
            compare(change.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(change.itemLabelFormat, "%f")
            compare(change.itemLabelVisible, false)
            compare(change.mesh, Abstract3DSeries.Mesh.Cube)
            compare(change.meshRotation, Qt.quaternion(1, 1, 1, 1))
            compare(change.meshSmooth, true)
            compare(change.multiHighlightColor, "#008000")
            compare(change.multiHighlightGradient, gradient2)
            compare(change.name, "series1")
            compare(change.singleHighlightColor, "#ff0000")
            compare(change.singleHighlightGradient, gradient3)
            compare(change.userDefinedMesh, ":/customitem.mesh")
            compare(change.visible, false)
        }

        function test_4_change_gradient_stop() {
            gradient1.stops[0].color = "yellow"
            compare(change.baseGradient.stops[0].color, "#ffff00")
        }
    }
    SignalSpy {
        id: dataProxySpy
        target: change
        signalName: "dataProxyChanged"
    }

    SignalSpy {
        id: selectedPointSpy
        target: change
        signalName: "selectedPointChanged"
    }

    SignalSpy {
        id: flatShadingSupportedSpy
        target: change
        signalName: "flatShadingSupportedChanged"
    }

    SignalSpy {
        id: drawModeSpy
        target: change
        signalName: "drawModeChanged"
    }

    SignalSpy {
        id: shadingSpy
        target: change
        signalName: "shadingChanged"
    }

    SignalSpy {
        id: textureSpy
        target: change
        signalName: "textureChanged"
    }

    SignalSpy {
        id: textureFileSpy
        target: change
        signalName: "textureFileChanged"
    }

    SignalSpy {
        id: wireFrameColorSpy
        target: change
        signalName: "wireframeColorChanged"
    }

    SignalSpy {
        id: dataArraySpy
        target: change
        signalName: "dataArrayChanged"
    }
}
