// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
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
        empty = Qt.createQmlObject("
        import QtQuick
        import QtDataVisualization
        Surface3D {
        }", top)
    }

    function constructBasic() {
        basic = Qt.createQmlObject("
        import QtQuick
        import QtDataVisualization
        Surface3D {
            anchors.fill: parent
            flipHorizontalGrid: true
        }", top)
        basic.anchors.fill = top
    }

    function constructCommon() {
        common = Qt.createQmlObject("
        import QtQuick
        import QtDataVisualization
        Surface3D {
            anchors.fill: parent
        }", top)
        common.anchors.fill = top
    }

    function constructCommonInit() {
        common_init = Qt.createQmlObject("
        import QtQuick
        import QtDataVisualization
        Surface3D {
            anchors.fill: parent
            selectionMode: AbstractGraph3D.SelectionNone
            shadowQuality: AbstractGraph3D.ShadowQualityLow
            msaaSamples: 2
            theme: Theme3D { }
            renderingMode: AbstractGraph3D.RenderIndirect
            measureFps: true
            orthoProjection: false
            aspectRatio: 3.0
            optimizationHints: AbstractGraph3D.OptimizationStatic
            polar: false
            radialLabelOffset: 2
            horizontalAspectRatio: 0.2
            reflection: true
            reflectivity: 0.1
            locale: Qt.locale(\"UK\")
            margin: 0.2
        }", top)
        common_init.anchors.fill = top
    }

    TestCase {
        name: "Surface3D Empty"
        when: windowShown

        function test_empty() {
            constructEmpty()
            compare(empty.width, 0, "width")
            compare(empty.height, 0, "height")
            compare(empty.seriesList.length, 0, "seriesList")
            compare(empty.selectedSeries, null, "selectedSeries")
            compare(empty.flipHorizontalGrid, false, "flipHorizontalGrid")
            compare(empty.axisX.orientation, AbstractAxis3D.AxisOrientationX)
            compare(empty.axisZ.orientation, AbstractAxis3D.AxisOrientationZ)
            compare(empty.axisY.orientation, AbstractAxis3D.AxisOrientationY)
            compare(empty.axisX.type, AbstractAxis3D.AxisTypeValue)
            compare(empty.axisZ.type, AbstractAxis3D.AxisTypeValue)
            compare(empty.axisY.type, AbstractAxis3D.AxisTypeValue)
            waitForRendering(top)
        }
    }

    TestCase {
        name: "Surface3D Basic"
        when: windowShown

        function test_1_basic() {
            constructBasic()
            compare(basic.width, 150, "width")
            compare(basic.height, 150, "height")
            compare(basic.flipHorizontalGrid, true, "flipHorizontalGrid")
        }

        function test_2_change_basic() {
            basic.flipHorizontalGrid = false
            compare(basic.flipHorizontalGrid, false, "flipHorizontalGrid")
            waitForRendering(top)
        }
    }

    TestCase {
        name: "Surface3D Common"
        when: windowShown

        function test_1_common() {
            if (Qt.platform.os === "android")
                return;

            constructCommon()
            if (common.shadowsSupported === false)
                return;

            compare(common.selectionMode, AbstractGraph3D.SelectionItem, "selectionMode")
            compare(common.shadowQuality, AbstractGraph3D.ShadowQualityMedium, "shadowQuality")
            compare(common.msaaSamples, 4, "msaaSamples")
            compare(common.theme.type, Theme3D.ThemeQt, "theme")
            compare(common.renderingMode, AbstractGraph3D.RenderIndirect, "renderingMode")
            compare(common.measureFps, false, "measureFps")
            compare(common.customItemList.length, 0, "customItemList")
            compare(common.orthoProjection, false, "orthoProjection")
            compare(common.selectedElement, AbstractGraph3D.ElementNone, "selectedElement")
            compare(common.aspectRatio, 2.0, "aspectRatio")
            compare(common.optimizationHints, AbstractGraph3D.OptimizationDefault, "optimizationHints")
            compare(common.polar, false, "polar")
            compare(common.radialLabelOffset, 1, "radialLabelOffset")
            compare(common.horizontalAspectRatio, 0, "horizontalAspectRatio")
            compare(common.reflection, false, "reflection")
            compare(common.reflectivity, 0.5, "reflectivity")
            compare(common.locale, Qt.locale("C"), "locale")
            compare(common.queriedGraphPosition, Qt.vector3d(0, 0, 0), "queriedGraphPosition")
            compare(common.margin, -1, "margin")
            waitForRendering(top)
        }

        function test_2_change_common() {
            if (Qt.platform.os === "android" || common.shadowsSupported === false)
                return;

            common.selectionMode = AbstractGraph3D.SelectionItem | AbstractGraph3D.SelectionRow | AbstractGraph3D.SelectionSlice
            common.shadowQuality = AbstractGraph3D.ShadowQualitySoftHigh
            common.msaaSamples = 8
            compare(common.shadowQuality, AbstractGraph3D.ShadowQualitySoftHigh, "shadowQuality")
            compare(common.msaaSamples, 8, "msaaSamples")
            common.theme.type = Theme3D.ThemeRetro
            common.renderingMode = AbstractGraph3D.RenderDirectToBackground_NoClear
            common.measureFps = true
            common.orthoProjection = true
            common.aspectRatio = 1.0
            common.optimizationHints = AbstractGraph3D.OptimizationStatic
            common.polar = true
            common.radialLabelOffset = 2
            common.horizontalAspectRatio = 1
            common.reflection = true
            common.reflectivity = 1.0
            common.locale = Qt.locale("FI")
            common.margin = 1.0
            compare(common.selectionMode, AbstractGraph3D.SelectionItem | AbstractGraph3D.SelectionRow | AbstractGraph3D.SelectionSlice, "selectionMode")
            compare(common.shadowQuality, AbstractGraph3D.ShadowQualityNone, "shadowQuality") // Ortho disables shadows
            compare(common.msaaSamples, 0, "msaaSamples") // Rendering mode changes this to zero
            compare(common.theme.type, Theme3D.ThemeRetro, "theme")
            compare(common.renderingMode, AbstractGraph3D.RenderDirectToBackground_NoClear, "renderingMode")
            compare(common.measureFps, true, "measureFps")
            compare(common.orthoProjection, true, "orthoProjection")
            compare(common.aspectRatio, 1.0, "aspectRatio")
            compare(common.optimizationHints, AbstractGraph3D.OptimizationStatic, "optimizationHints")
            compare(common.polar, true, "polar")
            compare(common.radialLabelOffset, 2, "radialLabelOffset")
            compare(common.horizontalAspectRatio, 1, "horizontalAspectRatio")
            compare(common.reflection, true, "reflection")
            compare(common.reflectivity, 1.0, "reflectivity")
            compare(common.locale, Qt.locale("FI"), "locale")
            compare(common.margin, 1.0, "margin")
            waitForRendering(top)
        }

        function test_3_change_invalid_common() {
            if (Qt.platform.os === "android"  || common.shadowsSupported === false)
                return;

            common.selectionMode = AbstractGraph3D.SelectionRow | AbstractGraph3D.SelectionColumn | AbstractGraph3D.SelectionSlice
            common.theme.type = -2
            common.renderingMode = -1
            common.measureFps = false
            common.orthoProjection = false
            common.aspectRatio = -1.0
            common.polar = false
            common.horizontalAspectRatio = -2
            common.reflection = false
            common.reflectivity = -1.0
            compare(common.selectionMode, AbstractGraph3D.SelectionItem | AbstractGraph3D.SelectionRow | AbstractGraph3D.SelectionSlice, "selectionMode")
            compare(common.theme.type, Theme3D.ThemeRetro, "theme")
            compare(common.renderingMode, AbstractGraph3D.RenderDirectToBackground_NoClear, "renderingMode")
            compare(common.aspectRatio, 1.0, "aspectRatio")
            compare(common.horizontalAspectRatio, 1.0, "horizontalAspectRatio")
            compare(common.reflectivity, 1.0, "reflectivity")
            waitForRendering(top)
        }
    }

    TestCase {
        name: "Surface3D Common Initialized"
        when: windowShown

        function test_1_common_initialized() {
            if (Qt.platform.os === "android")
                return;

            constructCommonInit()
            if (common_init.shadowsSupported === false)
                return;

            compare(common_init.selectionMode, AbstractGraph3D.SelectionNone, "selectionMode")
            tryCompare(common_init, "shadowQuality", AbstractGraph3D.ShadowQualityLow)
            compare(common_init.msaaSamples, 2, "msaaSamples")
            compare(common_init.theme.type, Theme3D.ThemeUserDefined, "theme")
            compare(common_init.renderingMode, AbstractGraph3D.RenderIndirect, "renderingMode")
            compare(common_init.measureFps, true, "measureFps")
            compare(common_init.customItemList.length, 0, "customItemList")
            compare(common_init.orthoProjection, false, "orthoProjection")
            compare(common_init.aspectRatio, 3.0, "aspectRatio")
            compare(common_init.optimizationHints, AbstractGraph3D.OptimizationStatic, "optimizationHints")
            compare(common_init.polar, false, "polar")
            compare(common_init.radialLabelOffset, 2, "radialLabelOffset")
            compare(common_init.horizontalAspectRatio, 0.2, "horizontalAspectRatio")
            compare(common_init.reflection, true, "reflection")
            compare(common_init.reflectivity, 0.1, "reflectivity")
            compare(common_init.locale, Qt.locale("UK"), "locale")
            compare(common_init.margin, 0.2, "margin")
            waitForRendering(top)
        }
    }
}
