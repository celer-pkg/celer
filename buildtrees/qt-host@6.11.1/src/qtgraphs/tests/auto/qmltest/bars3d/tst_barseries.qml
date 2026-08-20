// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Bar3DSeries {
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

    Color {
        id: rowColor1
        color: "green"
    }

    Color {
        id: rowColor2
        color: "blue"
    }
    Color {
        id: rowColor3
        color: "red"
    }

    Bar3DSeries {
        id: initialized
        dataProxy: ItemModelBarDataProxy {
            itemModel: ListModel {
                ListElement {
                    year: "2012"
                    city: "Oulu"
                    expenses: "4200"
                }
                ListElement {
                    year: "2012"
                    city: "Rauma"
                    expenses: "2100"
                }
            }
            rowRole: "city"
            columnRole: "year"
            valueRole: "expenses"
        }
        meshAngle: 15.0
        selectedBar: Qt.point(0, 0)

        baseColor: "blue"
        baseGradient: gradient1
        colorStyle: GraphsTheme.ColorStyle.ObjectGradient
        itemLabelFormat: "%f"
        itemLabelVisible: false
        mesh: Abstract3DSeries.Mesh.Cone
        meshSmooth: true
        multiHighlightColor: "green"
        multiHighlightGradient: gradient2
        lightingMode: Abstract3DSeries.LightingMode.Unshaded
        name: "series1"
        singleHighlightColor: "red"
        singleHighlightGradient: gradient3
        userDefinedMesh: ":/customitem.mesh"
        visible: false
        rowColors: [rowColor1, rowColor2, rowColor3]
        valueColoringEnabled: true
    }

    ItemModelBarDataProxy {
        id: proxy1
        itemModel: ListModel {
            ListElement {
                year: "2012"
                city: "Oulu"
                expenses: "4200"
            }
            ListElement {
                year: "2012"
                city: "Rauma"
                expenses: "2100"
            }
            ListElement {
                year: "2012"
                city: "Helsinki"
                expenses: "7040"
            }
        }
        rowRole: "city"
        columnRole: "year"
        valueRole: "expenses"
    }

    Bar3DSeries {
        id: change
        dataProxy: proxy1
    }

    TestCase {
        name: "Bar3DSeries Initial"

        function test_1_initial() {
            compare(initial.dataProxy.rowCount, 0)
            compare(initial.invalidSelectionPosition, Qt.point(-1, -1))
            compare(initial.meshAngle, 0)
            compare(initial.selectedBar, Qt.point(-1, -1))
            compare(initial.rowColors.length, 0)
        }

        function test_2_initial_common() {
            // Common properties
            compare(initial.baseColor, "#000000")
            verify(!initial.baseGradient)
            compare(initial.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(initial.itemLabel, "")
            compare(initial.itemLabelFormat, "@valueLabel")
            compare(initial.itemLabelVisible, true)
            compare(initial.mesh, Abstract3DSeries.Mesh.BevelBar)
            compare(initial.meshRotation, Qt.quaternion(1, 0, 0, 0))
            compare(initial.meshSmooth, false)
            compare(initial.multiHighlightColor, "#000000")
            verify(!initial.multiHighlightGradient)
            compare(initial.lightingMode, Abstract3DSeries.LightingMode.Shaded)
            compare(initial.name, "")
            compare(initial.singleHighlightColor, "#000000")
            verify(!initial.singleHighlightGradient)
            compare(initial.type, Abstract3DSeries.SeriesType.Bar)
            compare(initial.userDefinedMesh, "")
            compare(initial.visible, true)
            compare(initial.valueColoringEnabled, false)
        }
    }

    TestCase {
        name: "Bar3DSeries Initialized"

        function test_1_initialized() {
            compare(initialized.dataProxy.rowCount, 2)
            fuzzyCompare(initialized.meshAngle, 15.0, 0.01)
            compare(initialized.selectedBar, Qt.point(0, 0))
            compare(initialized.rowColors.length, 3)
        }

        function test_2_initialized_common() {
            // Common properties
            compare(initialized.baseColor, "#0000ff")
            compare(initialized.baseGradient, gradient1)
            compare(initialized.colorStyle,
                    GraphsTheme.ColorStyle.ObjectGradient)
            compare(initialized.itemLabelFormat, "%f")
            compare(initialized.itemLabelVisible, false)
            compare(initialized.mesh, Abstract3DSeries.Mesh.Cone)
            compare(initialized.meshSmooth, true)
            compare(initialized.multiHighlightColor, "#008000")
            compare(initialized.multiHighlightGradient, gradient2)
            compare(initialized.lightingMode, Abstract3DSeries.LightingMode.Unshaded)
            compare(initialized.name, "series1")
            compare(initialized.singleHighlightColor, "#ff0000")
            compare(initialized.singleHighlightGradient, gradient3)
            compare(initialized.userDefinedMesh, ":/customitem.mesh")
            compare(initialized.visible, false)
            compare(initialized.valueColoringEnabled, true)
        }
    }

    TestCase {
        name: "Bar3DSeries Change"

        function test_1_change() {
            change.meshAngle = 15.0
            change.selectedBar = Qt.point(0, 0)
            change.rowColors = [rowColor1, rowColor2, rowColor3]
            change.rowLabels = ["vaasa", "Turku", "Kuusamo"]
            change.columnLabels = ["2010", "2011", "2013"]
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.dataProxy.rowCount, 3)
            fuzzyCompare(change.meshAngle, 15.0, 0.01)
            compare(change.selectedBar, Qt.point(0, 0))

            compare(meshAngleSpy.count, 1)
            compare(selectedBarSpy.count, 1)
            compare(rowColorsSpy.count, 3)
            // compare(dataProxySpy.count, 1) TODO: Fix failing test (QTBUG-137247)
            compare(rowLabelsSpy.count, 2)
            compare(columnLabelsSpy.count, 2)
        }

        function test_3_change_common() {
            change.baseColor = "blue"
            change.baseGradient = gradient1
            change.colorStyle = GraphsTheme.ColorStyle.ObjectGradient
            change.itemLabelFormat = "%f"
            change.itemLabelVisible = false
            change.mesh = Abstract3DSeries.Mesh.Cone
            change.meshSmooth = true
            change.multiHighlightColor = "green"
            change.multiHighlightGradient = gradient2
            change.lightingMode = Abstract3DSeries.LightingMode.Unshaded
            change.name = "series1"
            change.singleHighlightColor = "red"
            change.singleHighlightGradient = gradient3
            change.userDefinedMesh = ":/customitem.mesh"
            change.visible = false
            change.valueColoringEnabled = true

            compare(change.baseColor, "#0000ff")
            compare(change.baseGradient, gradient1)
            compare(change.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(change.itemLabelFormat, "%f")
            compare(change.itemLabelVisible, false)
            compare(change.mesh, Abstract3DSeries.Mesh.Cone)
            compare(change.meshSmooth, true)
            compare(change.multiHighlightColor, "#008000")
            compare(change.multiHighlightGradient, gradient2)
            compare(change.lightingMode, Abstract3DSeries.LightingMode.Unshaded)
            compare(change.name, "series1")
            compare(change.singleHighlightColor, "#ff0000")
            compare(change.singleHighlightGradient, gradient3)
            compare(change.userDefinedMesh, ":/customitem.mesh")
            compare(change.visible, false)
            compare(change.valueColoringEnabled, true)

            compare(baseColorSpy.count, 1)
            compare(baseGradientSpy.count, 1)
            compare(colorStyleSpy.count, 1)
            compare(itemLabelFormatSpy.count, 1)
            compare(itemLabelVisibleSpy.count, 1)
            compare(meshSpy.count, 1)
            compare(meshSmoothingSpy.count, 1)
            compare(singleHLSpy.count, 1)
            compare(singleGradientSpy.count, 1)
            compare(multiHLSpy.count, 1)
            compare(multiGradientSpy.count, 1)
            compare(lightingModeSpy.count, 1)
            compare(nameSpy.count, 1)
            compare(visibleSpy.count, 1)
            compare(userMeshSpy.count, 1)
        }

        function test_4_change_gradient_stop() {
            gradient1.stops[0].color = "yellow"
            compare(change.baseGradient.stops[0].color, "#ffff00")
        }

        function test_5_change_rowColors() {
            rowColor2.color = "purple"
            compare(change.rowColors[1].color, "#800080")
        }
    }

    SignalSpy {
        id: dataProxySpy
        target: change
        signalName: "dataProxyChanged"
    }

    SignalSpy {
        id: selectedBarSpy
        target: change
        signalName: "selectedBarChanged"
    }

    SignalSpy {
        id:meshAngleSpy
        target: change
        signalName: "meshAngleChanged"
    }

    SignalSpy {
        id: rowColorsSpy
        target: change
        signalName: "rowColorsChanged"
    }

    SignalSpy {
        id: rowLabelsSpy
        target: change
        signalName: "rowLabelsChanged"
    }

    SignalSpy {
        id: columnLabelsSpy
        target: change
        signalName: "columnLabelsChanged"
    }

    SignalSpy {
        id: nameSpy
        target: change
        signalName: "nameChanged"
    }

    SignalSpy {
        id: visibleSpy
        target: change
        signalName: "visibleChanged"
    }

    SignalSpy {
        id: baseColorSpy
        target: change
        signalName: "baseColorChanged"
    }

    SignalSpy {
        id: baseGradientSpy
        target: change
        signalName: "baseGradientChanged"
    }

    SignalSpy {
        id: colorStyleSpy
        target: change
        signalName: "colorStyleChanged"
    }

    SignalSpy {
        id: itemLabelFormatSpy
        target: change
        signalName: "itemLabelFormatChanged"
    }

    SignalSpy {
        id: itemLabelVisibleSpy
        target: change
        signalName: "itemLabelVisibleChanged"
    }

    SignalSpy {
        id: meshSpy
        target: change
        signalName: "meshChanged"
    }

    SignalSpy {
        id: meshSmoothingSpy
        target: change
        signalName: "meshSmoothChanged"
    }

    SignalSpy {
        id: singleHLSpy
        target: change
        signalName: "singleHighlightColorChanged"
    }

    SignalSpy {
        id: singleGradientSpy
        target: change
        signalName: "singleHighlightGradientChanged"
    }

    SignalSpy {
        id: multiHLSpy
        target: change
        signalName: "multiHighlightColorChanged"
    }

    SignalSpy {
        id: multiGradientSpy
        target: change
        signalName: "multiHighlightGradientChanged"
    }

    SignalSpy {
        id: lightingModeSpy
        target: change
        signalName: "lightingModeChanged"
    }

    SignalSpy {
        id: userMeshSpy
        target: change
        signalName: "userDefinedMeshChanged"
    }
}
