// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    property var empty: null
    property var basic: null
    property var common: null
    property var common_init: null

    function constructEmpty() {
        empty = Qt.createQmlObject(`
                                   import QtQuick 2.2
                                   import QtGraphs
                                   Bars3D {
                                   }`, top)
    }

    function constructBasic() {
        basic = Qt.createQmlObject(`
                                   import QtQuick 2.2
                                   import QtGraphs
                                   Bars3D {
                                   anchors.fill: parent
                                   multiSeriesUniform: true
                                   barThickness: 0.1
                                   barSpacing.width: 0.1
                                   barSpacing.height: 0.1
                                   barSeriesMargin.width: 0.3
                                   barSeriesMargin.height: 0.3
                                   barSpacingRelative: false
                                   floorLevel: 1.0
                                   }`, top)
        top.basic.anchors.fill = top
    }

    function constructCommon() {
        common = Qt.createQmlObject(`
                                    import QtQuick 2.2
                                    import QtGraphs
                                    Bars3D {
                                    anchors.fill: parent
                                    }`, top)
        common.anchors.fill = top
    }

    function constructCommonInit() {
        common_init = Qt.createQmlObject(`
                                         import QtQuick 2.2
                                         import QtGraphs
                                         Bars3D {
                                         anchors.fill: parent
                                         selectionMode: Graphs3D.SelectionFlag.None
                                         shadowQuality: Graphs3D.ShadowQuality.Low
                                         msaaSamples: 2
                                         theme: GraphsTheme { }
                                         renderingMode: Graphs3D.RenderingMode.Indirect
                                         measureFps: true
                                         orthoProjection: false
                                         aspectRatio: 3.0
                                         optimizationHint: Graphs3D.OptimizationHint.Default
                                         polar: false
                                         radialLabelOffset: 2
                                         horizontalAspectRatio: 0.2
                                         locale: Qt.locale("UK")
                                         margin: 0.2
                                         labelMargin: 0.2
                                         lightColor: "black"
                                         ambientLightStrength: 0.5
                                         lightStrength: 10.0
                                         shadowStrength: 50.0
                                         minCameraXRotation: 0.0
                                         maxCameraXRotation: 90.0
                                         minCameraYRotation: -90.0
                                         maxCameraYRotation: 0.0
                                         }`, top)
        common_init.anchors.fill = top
    }

    TestCase {
        name: "Bars3D Empty"
        when: windowShown

        function test_empty() {
            top.constructEmpty()
            compare(top.empty.width, 0, "width")
            compare(top.empty.height, 0, "height")
            compare(top.empty.multiSeriesUniform, false, "multiSeriesUniform")
            compare(top.empty.barThickness, 1.0, "barThickness")
            compare(top.empty.barSpacing, Qt.size(1.0, 1.0), "barSpacing")
            compare(top.empty.barSeriesMargin, Qt.size(0.0, 0.0), "barSeriesMargin")
            compare(top.empty.barSpacingRelative, true, "barSpacingRelative")
            compare(top.empty.seriesList.length, 0, "seriesList")
            compare(top.empty.selectedSeries, null, "selectedSeries")
            compare(top.empty.primarySeries, null, "primarySeries")
            compare(top.empty.floorLevel, 0.0, "floorLevel")
            compare(top.empty.columnAxis.orientation, Abstract3DAxis.AxisOrientation.X)
            compare(top.empty.rowAxis.orientation, Abstract3DAxis.AxisOrientation.Z)
            compare(top.empty.valueAxis.orientation, Abstract3DAxis.AxisOrientation.Y)
            compare(top.empty.columnAxis.type, Abstract3DAxis.AxisType.Category)
            compare(top.empty.rowAxis.type, Abstract3DAxis.AxisType.Category)
            compare(top.empty.valueAxis.type, Abstract3DAxis.AxisType.Value)
            waitForRendering(top)
        }
    }

    TestCase {
        name: "Bars3D Basic"
        when: windowShown

        function test_1_basic() {
            top.constructBasic()
            compare(top.basic.width, 150, "width")
            compare(top.basic.height, 150, "height")
            compare(top.basic.multiSeriesUniform, true, "multiSeriesUniform")
            compare(top.basic.barThickness, 0.1, "barThickness")
            compare(top.basic.barSpacing, Qt.size(0.1, 0.1), "barSpacing")
            compare(top.basic.barSeriesMargin, Qt.size(0.3, 0.3), "barSeriesMargin")
            compare(top.basic.barSpacingRelative, false, "barSpacingRelative")
            compare(top.basic.floorLevel, 1.0, "floorLevel")
            waitForRendering(top)
        }

        function test_2_basic_change() {
            top.basic.multiSeriesUniform = false
            top.basic.barThickness = 0.5
            top.basic.barSpacing = Qt.size(1.0, 0.0)
            top.basic.barSeriesMargin = Qt.size(0.5, 0.0)
            top.basic.barSpacingRelative = true
            top.basic.floorLevel = 0.2
            compare(top.basic.multiSeriesUniform, false, "multiSeriesUniform")
            compare(top.basic.barThickness, 0.5, "barThickness")
            compare(top.basic.barSpacing, Qt.size(1.0, 0.0), "barSpacing")
            compare(top.basic.barSeriesMargin, Qt.size(0.5, 0.0), "barSeriesMargin")
            compare(top.basic.barSpacingRelative, true, "barSpacingRelative")
            compare(top.basic.floorLevel, 0.2, "floorLevel")
            waitForRendering(top)
        }

        function test_3_basic_change_invalid() {
            top.basic.barThickness = -1
            top.basic.barSpacing = Qt.size(-1.0, -1.0)
            top.basic.barSeriesMargin = Qt.size(-1.0, -1.0)
            compare(top.basic.barThickness, 0.5, "barThickness")
            compare(top.basic.barSpacing, Qt.size(-1.0, -1.0), "barSpacing")
            compare(top.basic.barSeriesMargin, Qt.size(-1.0, -1.0),
                    "barSeriesMargin")
            waitForRendering(top)
        }
    }

    TestCase {
        name: "Bars3D Common"
        when: windowShown

        function test_1_common() {
            top.constructCommon()
            compare(top.common.selectionMode, Graphs3D.SelectionFlag.Item,
                    "selectionMode")
            compare(top.common.shadowQuality, Graphs3D.ShadowQuality.Medium,
                    "shadowQuality")
            compare(top.common.msaaSamples, 4, "msaaSamples")
            compare(top.common.theme.theme, GraphsTheme.Theme.QtGreen, "theme")
            compare(top.common.renderingMode, Graphs3D.RenderingMode.Indirect,
                    "renderingMode")
            compare(top.common.measureFps, false, "measureFps")
            compare(top.common.customItemList.length, 0, "customItemList")
            compare(top.common.orthoProjection, false, "orthoProjection")
            compare(top.common.selectedElement, Graphs3D.ElementType.None,
                    "selectedElement")
            compare(top.common.aspectRatio, 2.0, "aspectRatio")
            compare(top.common.optimizationHint, Graphs3D.OptimizationHint.Default,
                    "optimizationHint")
            compare(top.common.polar, false, "polar")
            compare(top.common.radialLabelOffset, 1, "radialLabelOffset")
            compare(top.common.horizontalAspectRatio, 0, "horizontalAspectRatio")
            compare(top.common.locale, Qt.locale("C"), "locale")
            compare(top.common.queriedGraphPosition, Qt.vector3d(0, 0, 0),
                    "queriedGraphPosition")
            compare(top.common.margin, -1, "margin")
            compare(top.common.labelMargin, 0.1, "labelMargin")
            compare(top.common.lightColor, "#ffffff", "lightColor")
            compare(top.common.ambientLightStrength, 0.25, "ambientLightStrength")
            compare(top.common.lightStrength, 5.0, "lightStrength")
            compare(top.common.shadowStrength, 25.0, "shadowStrength")
            compare(top.common.minCameraXRotation, -180.0, "minCameraXRotation")
            compare(top.common.maxCameraXRotation, 180.0, "maxCameraXRotation")
            compare(top.common.minCameraYRotation, 0.0, "minCameraYRotation")
            compare(top.common.maxCameraYRotation, 90.0, "maxCameraYRotation")
            compare(top.common.cameraTargetPosition, Qt.vector3d(0, 0, 0),
                    "cameraTargetPosition")
        }

        function test_2_change_common() {
            top.common.selectionMode = Graphs3D.SelectionFlag.Item
                    | Graphs3D.SelectionFlag.Row | Graphs3D.SelectionFlag.Slice
            top.common.shadowQuality = Graphs3D.ShadowQuality.SoftHigh
            compare(top.common.shadowQuality, Graphs3D.ShadowQuality.SoftHigh,
                    "shadowQuality")
            top.common.msaaSamples = 8
            compare(top.common.msaaSamples, 8, "msaaSamples")
            top.common.theme.theme = GraphsTheme.Theme.QtGreenNeon
            top.common.renderingMode = Graphs3D.RenderingMode.DirectToBackground
            top.common.measureFps = true
            top.common.orthoProjection = true
            top.common.aspectRatio = 1.0
            top.common.optimizationHint = Graphs3D.OptimizationHint.Default
            top.common.polar = true
            top.common.radialLabelOffset = 2
            top.common.horizontalAspectRatio = 1
            top.common.locale = Qt.locale("FI")
            top.common.margin = 1.0
            top.common.labelMargin = 1.0
            top.common.lightColor = "#ff0000"
            top.common.ambientLightStrength = 0.5
            top.common.lightStrength = 10.0
            top.common.shadowStrength = 50.0
            top.common.minCameraXRotation = 0
            top.common.maxCameraXRotation = 90
            top.common.minCameraYRotation = -90
            top.common.maxCameraYRotation = 0
            top.common.cameraTargetPosition = Qt.vector3d(1.0, 0.0, -1.0)
            compare(top.common.selectionMode,
                    Graphs3D.SelectionFlag.Item | Graphs3D.SelectionFlag.Row
                    | Graphs3D.SelectionFlag.Slice, "selectionMode")
            compare(top.common.shadowQuality, Graphs3D.ShadowQuality.None,
                    "shadowQuality") // Ortho disables shadows
            compare(top.common.msaaSamples, 0, "msaaSamples") // Rendering mode changes this to zero
            compare(top.common.theme.theme, GraphsTheme.Theme.QtGreenNeon, "theme")
            compare(top.common.renderingMode, Graphs3D.RenderingMode.DirectToBackground,
                    "renderingMode")
            compare(top.common.measureFps, true, "measureFps")
            compare(top.common.orthoProjection, true, "orthoProjection")
            compare(top.common.aspectRatio, 1.0, "aspectRatio")
            compare(top.common.optimizationHint, Graphs3D.OptimizationHint.Default,
                    "optimizationHint")
            compare(top.common.polar, true, "polar")
            compare(top.common.radialLabelOffset, 2, "radialLabelOffset")
            compare(top.common.horizontalAspectRatio, 1, "horizontalAspectRatio")
            compare(top.common.locale, Qt.locale("FI"), "locale")
            compare(top.common.margin, 1.0, "margin")
            compare(top.common.labelMargin, 1.0, "labelMargin")
            compare(top.common.lightColor, "#ff0000", "lightColor")
            compare(top.common.ambientLightStrength, 0.5, "ambientLightStrength")
            compare(top.common.lightStrength, 10.0, "lightStrength")
            compare(top.common.shadowStrength, 50.0, "shadowStrength")
            compare(top.common.minCameraXRotation, 0.0, "minCameraXRotation")
            compare(top.common.maxCameraXRotation, 90.0, "maxCameraXRotation")
            compare(top.common.minCameraYRotation, -90.0, "minCameraYRotation")
            compare(top.common.maxCameraYRotation, 0.0, "maxCameraYRotation")
            compare(top.common.cameraTargetPosition, Qt.vector3d(1.0, 0.0, -1.0),
                    "cameraTargetPosition")
        }

        function test_3_change_invalid_common() {
            top.common.selectionMode = Graphs3D.SelectionFlag.Row
                    | Graphs3D.SelectionFlag.Column | Graphs3D.SelectionFlag.Slice
            top.common.theme.theme = -2
            top.common.renderingMode = -1
            top.common.measureFps = false
            top.common.orthoProjection = false
            top.common.aspectRatio = -1.0
            top.common.polar = false
            top.common.horizontalAspectRatio = -2
            compare(top.common.selectionMode,
                    Graphs3D.SelectionFlag.Item | Graphs3D.SelectionFlag.Row
                    | Graphs3D.SelectionFlag.Slice, "selectionMode")
            compare(top.common.theme.theme, GraphsTheme.Theme.QtGreenNeon, "theme")
            compare(top.common.renderingMode,Graphs3D.RenderingMode.DirectToBackground,
                    "renderingMode")
            compare(top.common.aspectRatio, 1.0, "aspectRatio")
            compare(top.common.horizontalAspectRatio, 1, "horizontalAspectRatio")

            top.constructCommon()

            top.common.ambientLightStrength = -1.0
            compare(top.common.ambientLightStrength, 0.25, "ambientLightStrength")
            top.common.ambientLightStrength = -1.1
            compare(top.common.ambientLightStrength, 0.25, "ambientLightStrength")
            top.common.lightStrength = 11.0
            compare(top.common.lightStrength, 5.0, "lightStrength")
            top.common.lightStrength = -1.0
            compare(top.common.lightStrength, 5.0, "lightStrength")
            top.common.shadowStrength = 101.0
            compare(top.common.shadowStrength, 25.0, "shadowStrength")
            top.common.shadowStrength = -1.0
            compare(top.common.shadowStrength, 25.0, "shadowStrength")
            top.common.cameraTargetPosition = Qt.vector3d(2.0, 1.0, -2.0)
            compare(top.common.cameraTargetPosition, Qt.vector3d(1.0, 1.0, -1.0),
                    "cameraTargetPosition")
        }

        function test_4_common_initialized() {
            top.constructCommonInit()

            compare(top.common_init.selectionMode, Graphs3D.SelectionFlag.None,
                    "selectionMode")
            tryCompare(top.common_init, "shadowQuality", Graphs3D.ShadowQuality.Low)
            compare(top.common_init.msaaSamples, 2, "msaaSamples")
            compare(top.common_init.theme.theme,
                    GraphsTheme.Theme.QtGreen, "theme")
            compare(top.common_init.renderingMode, Graphs3D.RenderingMode.Indirect,
                    "renderingMode")
            compare(top.common_init.measureFps, true, "measureFps")
            compare(top.common_init.customItemList.length, 0, "customItemList")
            compare(top.common_init.orthoProjection, false, "orthoProjection")
            compare(top.common_init.aspectRatio, 3.0, "aspectRatio")
            compare(top.common_init.optimizationHint,
                    Graphs3D.OptimizationHint.Default, "optimizationHint")
            compare(top.common_init.polar, false, "polar")
            compare(top.common_init.radialLabelOffset, 2, "radialLabelOffset")
            compare(top.common_init.horizontalAspectRatio, 0.2,
                    "horizontalAspectRatio")
            compare(top.common_init.locale, Qt.locale("UK"), "locale")
            compare(top.common_init.margin, 0.2, "margin")
            compare(top.common_init.labelMargin, 0.2, "labelMargin")
            compare(top.common_init.lightColor, "#000000", "lightColor")
            compare(top.common_init.ambientLightStrength, 0.5,
                    "ambientLightStrength")
            compare(top.common_init.lightStrength, 10.0, "lightStrength")
            compare(top.common_init.shadowStrength, 50.0, "shadowStrength")
            compare(top.common_init.minCameraXRotation, 0.0, "minCameraXRotation")
            compare(top.common_init.maxCameraXRotation, 90.0, "maxCameraXRotation")
            compare(top.common_init.minCameraYRotation, -90.0, "minCameraYRotation")
            compare(top.common_init.maxCameraYRotation, 0.0, "maxCameraYRotation")
        }
    }
}
