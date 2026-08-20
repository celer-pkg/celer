// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtGraphs
import SingletonThemeTest

Item {
    id: top
    height: 300
    width: 900

    property var bars3d: null
    property var scatter3d: null
    property var surface3d: null

    Rectangle {
        id: createdbackground
        color: "#00ffff"
        anchors.fill: parent
    }

    Rectangle {
        id: baritem
        x: 0
        width: top.width / 3
        height: top.height
        color: "red"
        visible: false

        Bars3D {
            id: bars3ddecl
            anchors.fill: parent
            anchors.margins: 10
            theme: SingletonTheme.sharedtheme
            measureFps: true

            Bar3DSeries {
                dataProxy: ItemModelBarDataProxy {
                    itemModel: ListModel {
                        ListElement{ year: "2026"; city: "Liminka"; expenses: "600"; }
                        ListElement{ year: "2026"; city: "Oulu"; expenses: "4200"; }
                    }
                    rowRole: "city"
                    columnRole: "year"
                    valueRole: "expenses"
                }
            }
        }
    }

    Rectangle {
        id: scatteritem
        x: top.width / 3
        width: top.width / 3
        height: top.height
        color: "green"
        visible: false

        Scatter3D {
            id: scatter3ddecl
            anchors.fill: parent
            anchors.margins: 10
            theme: SingletonTheme.sharedtheme
            measureFps: true

            Scatter3DSeries {
                dataProxy: ItemModelScatterDataProxy {
                    itemModel: ListModel {
                        ListElement{ xPos: "2.754"; yPos: "1.455"; zPos: "3.362"; }
                        ListElement{ xPos: "3.164"; yPos: "2.022"; zPos: "4.348"; }
                    }
                    xPosRole: "xPos"
                    yPosRole: "yPos"
                    zPosRole: "zPos"
                }
            }
        }
    }

    Rectangle {
        id: surfaceitem
        x: 2 * top.width / 3
        width: top.width / 3
        height: top.height
        color: "blue"
        visible: false

        Surface3D {
            id: surface3ddecl
            anchors.fill: parent
            anchors.margins: 10
            theme: SingletonTheme.sharedtheme
            measureFps: true

            Surface3DSeries {
                ItemModelSurfaceDataProxy {
                    itemModel: ListModel {
                        ListElement{ row: "1"; column: "1"; y: "1"; }
                        ListElement{ row: "1"; column: "2"; y: "2"; }
                        ListElement{ row: "2"; column: "1"; y: "3"; }
                        ListElement{ row: "2"; column: "2"; y: "4"; }
                    }

                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "y"
                }
            }
        }
    }

    function createGraphs() {
        bars3d = Qt.createQmlObject(`
                                    import QtQuick
                                    import QtGraphs
                                    Bars3D {
                                    x: 10
                                    y: 10
                                    width: parent.width / 3 - 20
                                    height: parent.height - 20
                                    measureFps: true
                                    Bar3DSeries {
                                        dataProxy: ItemModelBarDataProxy {
                                            itemModel: ListModel {
                                                ListElement{ year: "2026"; city: "Liminka"; expenses: "600"; }
                                                ListElement{ year: "2026"; city: "Oulu"; expenses: "4200"; }
                                                }
                                            rowRole: "city"
                                            columnRole: "year"
                                            valueRole: "expenses"
                                        }
                                    }
                                    }`, top)
        bars3d.theme = SingletonTheme.sharedtheme

        scatter3d = Qt.createQmlObject(`
                                       import QtQuick
                                       import QtGraphs
                                       Scatter3D {
                                       x: parent.width / 3 + 10
                                       y: 10
                                       width: parent.width / 3 - 20
                                       height: parent.height - 20
                                       measureFps: true
                                       Scatter3DSeries {
                                           dataProxy: ItemModelScatterDataProxy {
                                               itemModel: ListModel {
                                                   ListElement{ xPos: "2.754"; yPos: "1.455"; zPos: "3.362"; }
                                                   ListElement{ xPos: "3.164"; yPos: "2.022"; zPos: "4.348"; }
                                               }
                                               xPosRole: "xPos"
                                               yPosRole: "yPos"
                                               zPosRole: "zPos"
                                           }
                                       }
                                       }`, top)
        scatter3d.theme = SingletonTheme.sharedtheme

        surface3d = Qt.createQmlObject(`
                                       import QtQuick
                                       import QtGraphs
                                       Surface3D {
                                       x: 2 * parent.width / 3 + 10
                                       y: 10
                                       width: parent.width / 3 - 20
                                       height: parent.height - 20
                                       measureFps: true
                                       Surface3DSeries {
                                           ItemModelSurfaceDataProxy {
                                               itemModel: ListModel {
                                                   ListElement{ row: "1"; column: "1"; y: "1"; }
                                                   ListElement{ row: "1"; column: "2"; y: "2"; }
                                                   ListElement{ row: "2"; column: "1"; y: "3"; }
                                                   ListElement{ row: "2"; column: "2"; y: "4"; }
                                               }

                                               rowRole: "row"
                                               columnRole: "column"
                                               yPosRole: "y"
                                           }
                                       }
                                       }`, top)
        surface3d.theme = SingletonTheme.sharedtheme

        bars3d.visible = true
        scatter3d.visible = true
        surface3d.visible = true
    }

    function destroyGraphs() {
        bars3d.visible = false
        scatter3d.visible = false
        surface3d.visible = false
        delete bars3d
        delete scatter3d
        delete surface3d
        bars3d = null
        scatter3d = null
        surface3d = null
    }

    TestCase {
        name: "Singleton Theme unmodified"
        when: windowShown

        function test_unmodified() {
            compare(SingletonTheme.sharedtheme.backgroundVisible, true)
            compare(SingletonTheme.sharedtheme.gridVisible, true)
            compare(SingletonTheme.sharedtheme.labelFont.pointSize, 60)
            compare(SingletonTheme.sharedtheme.labelTextColor, "#aeaeae")
            compare(SingletonTheme.sharedtheme.labelsVisible, true)
            compare(SingletonTheme.sharedtheme.plotAreaBackgroundVisible, true)
            compare(SingletonTheme.sharedtheme.theme, GraphsTheme.Theme.MixSeries)
        }
    }

    TestCase {
        name: "Singleton Theme declarative"
        when: windowShown

        function init() {
            // code to be executed before each test function
            createdbackground.visible = false
            baritem.visible = true
            scatteritem.visible = true
            surfaceitem.visible = true
        }

        function test_initial_bars3d() {
            compare(bars3ddecl.theme.axisX.labelTextColor, "#aeaeae")
            compare(bars3ddecl.theme.axisX.mainColor, "#aeabab")
            compare(bars3ddecl.theme.axisX.mainWidth, 2)
            compare(bars3ddecl.theme.axisX.subColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisX.subWidth, 1)
            compare(bars3ddecl.theme.axisY.labelTextColor, "#aeaeae")
            compare(bars3ddecl.theme.axisY.mainColor, "#aeabab")
            compare(bars3ddecl.theme.axisY.mainWidth, 2)
            compare(bars3ddecl.theme.axisY.subColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisY.subWidth, 1)
            compare(bars3ddecl.theme.axisZ.labelTextColor, "#aeaeae")
            compare(bars3ddecl.theme.axisZ.mainColor, "#aeabab")
            compare(bars3ddecl.theme.axisZ.mainWidth, 2)
            compare(bars3ddecl.theme.axisZ.subColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisZ.subWidth, 1)
            compare(bars3ddecl.theme.backgroundColor, "#262626")
            compare(bars3ddecl.theme.backgroundColor, "#262626")
            compare(bars3ddecl.theme.backgroundVisible, true)
            compare(bars3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(bars3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(bars3ddecl.theme.grid.labelTextColor, "#000000")
            compare(bars3ddecl.theme.grid.mainColor, "#aeabab")
            compare(bars3ddecl.theme.grid.mainWidth, 2)
            compare(bars3ddecl.theme.grid.subColor, "#6a6a6a")
            compare(bars3ddecl.theme.grid.subWidth, 1)
            compare(bars3ddecl.theme.gridVisible, true)
            compare(bars3ddecl.theme.labelBackgroundColor, "#2e2e2e")
            compare(bars3ddecl.theme.labelFont.pointSize, 60)
            compare(bars3ddecl.theme.labelTextColor, "#aeaeae")
            compare(bars3ddecl.theme.labelsVisible, true)
            compare(bars3ddecl.theme.multiHighlightColor, "#22d489")
            compare(bars3ddecl.theme.multiHighlightGradient, null)
            compare(bars3ddecl.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(bars3ddecl.theme.plotAreaBackgroundVisible, true)
            compare(bars3ddecl.theme.singleHighlightColor, "#dbeb00")
            compare(bars3ddecl.theme.singleHighlightGradient, null)
            compare(bars3ddecl.theme.theme, GraphsTheme.Theme.MixSeries)
        }

        function test_initial_scatter3d() {
            compare(scatter3ddecl.theme.axisX.labelTextColor, "#aeaeae")
            compare(scatter3ddecl.theme.axisX.mainColor, "#aeabab")
            compare(scatter3ddecl.theme.axisX.mainWidth, 2)
            compare(scatter3ddecl.theme.axisX.subColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisX.subWidth, 1)
            compare(scatter3ddecl.theme.axisY.labelTextColor, "#aeaeae")
            compare(scatter3ddecl.theme.axisY.mainColor, "#aeabab")
            compare(scatter3ddecl.theme.axisY.mainWidth, 2)
            compare(scatter3ddecl.theme.axisY.subColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisY.subWidth, 1)
            compare(scatter3ddecl.theme.axisZ.labelTextColor, "#aeaeae")
            compare(scatter3ddecl.theme.axisZ.mainColor, "#aeabab")
            compare(scatter3ddecl.theme.axisZ.mainWidth, 2)
            compare(scatter3ddecl.theme.axisZ.subColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisZ.subWidth, 1)
            compare(scatter3ddecl.theme.backgroundColor, "#262626")
            compare(scatter3ddecl.theme.backgroundColor, "#262626")
            compare(scatter3ddecl.theme.backgroundVisible, true)
            compare(scatter3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(scatter3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(scatter3ddecl.theme.grid.labelTextColor, "#000000")
            compare(scatter3ddecl.theme.grid.mainColor, "#aeabab")
            compare(scatter3ddecl.theme.grid.mainWidth, 2)
            compare(scatter3ddecl.theme.grid.subColor, "#6a6a6a")
            compare(scatter3ddecl.theme.grid.subWidth, 1)
            compare(scatter3ddecl.theme.gridVisible, true)
            compare(scatter3ddecl.theme.labelBackgroundColor, "#2e2e2e")
            compare(scatter3ddecl.theme.labelFont.pointSize, 60)
            compare(scatter3ddecl.theme.labelTextColor, "#aeaeae")
            compare(scatter3ddecl.theme.labelsVisible, true)
            compare(scatter3ddecl.theme.multiHighlightColor, "#22d489")
            compare(scatter3ddecl.theme.multiHighlightGradient, null)
            compare(scatter3ddecl.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(scatter3ddecl.theme.plotAreaBackgroundVisible, true)
            compare(scatter3ddecl.theme.singleHighlightColor, "#dbeb00")
            compare(scatter3ddecl.theme.singleHighlightGradient, null)
            compare(scatter3ddecl.theme.theme, GraphsTheme.Theme.MixSeries)
        }

        function test_initial_surface3d() {
            compare(surface3ddecl.theme.axisX.labelTextColor, "#aeaeae")
            compare(surface3ddecl.theme.axisX.mainColor, "#aeabab")
            compare(surface3ddecl.theme.axisX.mainWidth, 2)
            compare(surface3ddecl.theme.axisX.subColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisX.subWidth, 1)
            compare(surface3ddecl.theme.axisY.labelTextColor, "#aeaeae")
            compare(surface3ddecl.theme.axisY.mainColor, "#aeabab")
            compare(surface3ddecl.theme.axisY.mainWidth, 2)
            compare(surface3ddecl.theme.axisY.subColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisY.subWidth, 1)
            compare(surface3ddecl.theme.axisZ.labelTextColor, "#aeaeae")
            compare(surface3ddecl.theme.axisZ.mainColor, "#aeabab")
            compare(surface3ddecl.theme.axisZ.mainWidth, 2)
            compare(surface3ddecl.theme.axisZ.subColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisZ.subWidth, 1)
            compare(surface3ddecl.theme.backgroundColor, "#262626")
            compare(surface3ddecl.theme.backgroundColor, "#262626")
            compare(surface3ddecl.theme.backgroundVisible, true)
            compare(surface3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(surface3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(surface3ddecl.theme.grid.labelTextColor, "#000000")
            compare(surface3ddecl.theme.grid.mainColor, "#aeabab")
            compare(surface3ddecl.theme.grid.mainWidth, 2)
            compare(surface3ddecl.theme.grid.subColor, "#6a6a6a")
            compare(surface3ddecl.theme.grid.subWidth, 1)
            compare(surface3ddecl.theme.gridVisible, true)
            compare(surface3ddecl.theme.labelBackgroundColor, "#2e2e2e")
            compare(surface3ddecl.theme.labelFont.pointSize, 60)
            compare(surface3ddecl.theme.labelTextColor, "#aeaeae")
            compare(surface3ddecl.theme.labelsVisible, true)
            compare(surface3ddecl.theme.multiHighlightColor, "#22d489")
            compare(surface3ddecl.theme.multiHighlightGradient, null)
            compare(surface3ddecl.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(surface3ddecl.theme.plotAreaBackgroundVisible, true)
            compare(surface3ddecl.theme.singleHighlightColor, "#dbeb00")
            compare(surface3ddecl.theme.singleHighlightGradient, null)
            compare(surface3ddecl.theme.theme, GraphsTheme.Theme.MixSeries)
        }
    }

    TestCase {
        name: "Singleton Theme declarative"
        when: windowShown

        function init() {
            // code to be executed before each test function
            colorSchemeThemeSpy.clear()
            createdbackground.visible = false
            baritem.visible = true
            scatteritem.visible = true
            surfaceitem.visible = true
        }

        function cleanup() {
            // code to be executed after each test function
            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Dark
        }

        function test_colorscheme() {
            // Grab intial graphs
            waitForRendering(bars3ddecl)
            let barorigimage = grabImage(baritem)
            let barorigcolor = barorigimage.pixel(30, 30)
            // barorigimage.save("bar-colorscheme-declarative-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(scatter3ddecl)
            let scatterorigimage = grabImage(scatteritem)
            let scatterorigcolor = scatterorigimage.pixel(30, 30)
            // scatterorigimage.save("scatter-colorscheme-declarative-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(surface3ddecl)
            let surfaceorigimage = grabImage(surfaceitem)
            let surfaceorigcolor = surfaceorigimage.pixel(30, 30)
            // surfaceorigimage.save("surface-colorscheme-declarative-origiimage.png") // Uncomment these lines if the test fails

            // Change color scheme
            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Light

            // Bars3D
            compare(bars3ddecl.theme.axisX.labelTextColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisX.mainColor, "#545151")
            compare(bars3ddecl.theme.axisX.subColor, "#afafaf")
            compare(bars3ddecl.theme.axisY.labelTextColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisY.mainColor, "#545151")
            compare(bars3ddecl.theme.axisY.subColor, "#afafaf")
            compare(bars3ddecl.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(bars3ddecl.theme.axisZ.mainColor, "#545151")
            compare(bars3ddecl.theme.axisZ.subColor, "#afafaf")
            compare(bars3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(bars3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(bars3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(bars3ddecl.theme.grid.labelTextColor, "#000000")
            compare(bars3ddecl.theme.grid.mainColor, "#545151")
            compare(bars3ddecl.theme.grid.subColor, "#afafaf")
            compare(bars3ddecl.theme.labelBackgroundColor, "#e7e7e7")
            compare(bars3ddecl.theme.labelTextColor, "#6a6a6a")
            compare(bars3ddecl.theme.multiHighlightColor, "#22d47b")
            compare(bars3ddecl.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(bars3ddecl.theme.singleHighlightColor, "#ccdc00")

            // Scatter3D
            compare(scatter3ddecl.theme.axisX.labelTextColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisX.mainColor, "#545151")
            compare(scatter3ddecl.theme.axisX.subColor, "#afafaf")
            compare(scatter3ddecl.theme.axisY.labelTextColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisY.mainColor, "#545151")
            compare(scatter3ddecl.theme.axisY.subColor, "#afafaf")
            compare(scatter3ddecl.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(scatter3ddecl.theme.axisZ.mainColor, "#545151")
            compare(scatter3ddecl.theme.axisZ.subColor, "#afafaf")
            compare(scatter3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(scatter3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(scatter3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(scatter3ddecl.theme.grid.labelTextColor, "#000000")
            compare(scatter3ddecl.theme.grid.mainColor, "#545151")
            compare(scatter3ddecl.theme.grid.subColor, "#afafaf")
            compare(scatter3ddecl.theme.labelBackgroundColor, "#e7e7e7")
            compare(scatter3ddecl.theme.labelTextColor, "#6a6a6a")
            compare(scatter3ddecl.theme.multiHighlightColor, "#22d47b")
            compare(scatter3ddecl.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(scatter3ddecl.theme.singleHighlightColor, "#ccdc00")

            // Surface3D
            compare(surface3ddecl.theme.axisX.labelTextColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisX.mainColor, "#545151")
            compare(surface3ddecl.theme.axisX.subColor, "#afafaf")
            compare(surface3ddecl.theme.axisY.labelTextColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisY.mainColor, "#545151")
            compare(surface3ddecl.theme.axisY.subColor, "#afafaf")
            compare(surface3ddecl.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(surface3ddecl.theme.axisZ.mainColor, "#545151")
            compare(surface3ddecl.theme.axisZ.subColor, "#afafaf")
            compare(surface3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(surface3ddecl.theme.backgroundColor, "#f2f2f2")
            compare(surface3ddecl.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(surface3ddecl.theme.grid.labelTextColor, "#000000")
            compare(surface3ddecl.theme.grid.mainColor, "#545151")
            compare(surface3ddecl.theme.grid.subColor, "#afafaf")
            compare(surface3ddecl.theme.labelBackgroundColor, "#e7e7e7")
            compare(surface3ddecl.theme.labelTextColor, "#6a6a6a")
            compare(surface3ddecl.theme.multiHighlightColor, "#22d47b")
            compare(surface3ddecl.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(surface3ddecl.theme.singleHighlightColor, "#ccdc00")

            // Signals
            compare(colorSchemeThemeSpy.count, 1)

            // Grab and check changed graphs
            waitForRendering(bars3ddecl)
            tryVerify(() => {
                          let barafterimage = grabImage(baritem)
                          let pass = !barafterimage.equals(barorigimage)
                            && (barafterimage.pixel(30, 30) != barorigcolor)
                          if (!pass)
                            barafterimage.save("bar-colorscheme-declarative-afterimage.png")
                          return pass
                      })

            waitForRendering(scatter3ddecl)
            tryVerify(() => {
                          let scatterafterimage = grabImage(scatteritem)
                          let pass = !scatterafterimage.equals(scatterorigimage)
                            && (scatterafterimage.pixel(30, 30) != scatterorigcolor)
                          if (!pass)
                            scatterafterimage.save("scatter-colorscheme-declarative-afterimage.png")
                          return pass
                      })

            waitForRendering(surface3ddecl)
            tryVerify(() => {
                          let surfaceafterimage = grabImage(surfaceitem)
                          let pass = !surfaceafterimage.equals(surfaceorigimage)
                            && (surfaceafterimage.pixel(30, 30) != surfaceorigcolor)
                          if (!pass)
                            surfaceafterimage.save("surface-colorscheme-declarative-afterimage.png")
                          return pass
                      })

            // Change color scheme back
            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Dark

            // Grab and check returned graphs
            waitForRendering(bars3ddecl)
            tryVerify(() => {
                          let barbackimage = grabImage(baritem)
                          let pass = barbackimage.equals(barorigimage)
                            && (barbackimage.pixel(30, 30) == barorigcolor)
                          if (!pass)
                            barbackimage.save("bar-colorscheme-declarative-backimage.png")
                          return pass
                      })

            waitForRendering(scatter3ddecl)
            tryVerify(() => {
                          let scatterbackimage = grabImage(scatteritem)
                          let pass = scatterbackimage.equals(scatterorigimage)
                            && (scatterbackimage.pixel(30, 30) == scatterorigcolor)
                          if (!pass)
                            scatterbackimage.save("scatter-colorscheme-declarative-backimage.png")
                          return pass
                      })

            waitForRendering(surface3ddecl)
            tryVerify(() => {
                          let surfacebackimage = grabImage(surfaceitem)
                          let pass = surfacebackimage.equals(surfaceorigimage)
                            && (surfacebackimage.pixel(30, 30) == surfaceorigcolor)
                          if (!pass)
                            surfacebackimage.save("surface-colorscheme-declarative-backimage.png")
                          return pass
                      })
        }
    }

    TestCase {
        name: "Singleton Theme declarative"
        when: windowShown

        function init() {
            // code to be executed before each test function
            backgroundVisibleThemeSpy.clear()
            colorStyleThemeSpy.clear()
            gridVisibleThemeSpy.clear()
            labelsVisibleThemeSpy.clear()
            plotAreaBackgroundVisibleThemeSpy.clear()
            themeThemeSpy.clear()
            createdbackground.visible = false
            baritem.visible = true
            scatteritem.visible = true
            surfaceitem.visible = true
        }

        function cleanup() {
            // code to be executed after each test function
            SingletonTheme.sharedtheme.backgroundVisible = true
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.Uniform
            SingletonTheme.sharedtheme.gridVisible = true
            SingletonTheme.sharedtheme.labelsVisible = true
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = true
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.MixSeries
        }

        function test_properties() {
            // Grab intial graphs
            waitForRendering(bars3ddecl)
            let barorigimage = grabImage(baritem)
            let barorigcolor = barorigimage.pixel(30, 30)
            // barorigimage.save("bar-properties-declarative-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(scatter3ddecl)
            let scatterorigimage = grabImage(scatteritem)
            let scatterorigcolor = scatterorigimage.pixel(30, 30)
            // scatterorigimage.save("scatter-properties-declarative-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(scatter3ddecl)
            let surfaceorigimage = grabImage(surfaceitem)
            let surfaceorigcolor = surfaceorigimage.pixel(30, 30)
            // surfaceorigimage.save("surface-properties-declarative-origiimage.png") // Uncomment these lines if the test fails

            SingletonTheme.sharedtheme.backgroundVisible = false
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.ObjectGradient
            SingletonTheme.sharedtheme.gridVisible = false
            SingletonTheme.sharedtheme.labelsVisible = false
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = false
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.QtGreenNeon

            // Bars3D
            compare(bars3ddecl.theme.backgroundVisible, false)
            compare(bars3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(bars3ddecl.theme.gridVisible, false)
            compare(bars3ddecl.theme.labelsVisible, false)
            compare(bars3ddecl.theme.plotAreaBackgroundVisible, false)
            compare(bars3ddecl.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Scatter3D
            compare(scatter3ddecl.theme.backgroundVisible, false)
            compare(scatter3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(scatter3ddecl.theme.gridVisible, false)
            compare(scatter3ddecl.theme.labelsVisible, false)
            compare(scatter3ddecl.theme.plotAreaBackgroundVisible, false)
            compare(scatter3ddecl.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Surface3D
            compare(surface3ddecl.theme.backgroundVisible, false)
            compare(surface3ddecl.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(surface3ddecl.theme.gridVisible, false)
            compare(surface3ddecl.theme.labelsVisible, false)
            compare(surface3ddecl.theme.plotAreaBackgroundVisible, false)
            compare(surface3ddecl.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Signals
            compare(backgroundVisibleThemeSpy.count, 1)
            compare(colorStyleThemeSpy.count, 1)
            compare(gridVisibleThemeSpy.count, 1)
            compare(labelsVisibleThemeSpy.count, 1)
            compare(plotAreaBackgroundVisibleThemeSpy.count, 1)
            compare(themeThemeSpy.count, 1)

            // Grab and check changed graphs
            waitForRendering(bars3ddecl)
            tryVerify(() => {
                          let barafterimage = grabImage(baritem)
                          let pass = !barafterimage.equals(barorigimage)
                            && (barafterimage.pixel(30, 30) == baritem.color)
                          if (!pass)
                            barafterimage.save("bar-properties-declarative-afterimage.png")
                          return pass
                      })

            waitForRendering(scatter3ddecl)
            tryVerify(() => {
                          let scatterafterimage = grabImage(scatteritem)
                          let pass = !scatterafterimage.equals(scatterorigimage)
                            && (scatterafterimage.pixel(30, 30) == scatteritem.color)
                          if (!pass)
                            scatterafterimage.save("scatter-properties-declarative-afterimage.png")
                          return pass
                      })

            waitForRendering(surface3ddecl)
            tryVerify(() => {
                          let surfaceafterimage = grabImage(surfaceitem)
                          let pass = !surfaceafterimage.equals(surfaceorigimage)
                            && (surfaceafterimage.pixel(30, 30) == surfaceitem.color)
                          if (!pass)
                            surfaceafterimage.save("surface-properties-declarative-afterimage.png")
                          return pass
                      })

            // Change color scheme back
            SingletonTheme.sharedtheme.backgroundVisible = true
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.Uniform
            SingletonTheme.sharedtheme.gridVisible = true
            SingletonTheme.sharedtheme.labelsVisible = true
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = true
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.MixSeries

            // Grab and check returned graphs
            waitForRendering(bars3ddecl)
            tryVerify(() => {
                          let barbackimage = grabImage(baritem)
                          let pass = barbackimage.equals(barorigimage)
                            && (barbackimage.pixel(30, 30) == barorigcolor)
                          if (!pass)
                            barbackimage.save("bar-properties-declarative-backimage.png")
                          return pass
                      })

            waitForRendering(scatter3ddecl)
            tryVerify(() => {
                          let scatterbackimage = grabImage(scatteritem)
                          let pass = scatterbackimage.equals(scatterorigimage)
                            && (scatterbackimage.pixel(30, 30) == scatterorigcolor)
                          if (!pass)
                            scatterbackimage.save("scatter-properties-declarative-backimage.png")
                          return pass
                      })

            waitForRendering(surface3ddecl)
            tryVerify(() => {
                          let surfacebackimage = grabImage(surfaceitem)
                          let pass = surfacebackimage.equals(surfaceorigimage)
                            && (surfacebackimage.pixel(30, 30) == surfaceorigcolor)
                          if (!pass)
                            surfacebackimage.save("surface-properties-declarative-backimage.png")
                          return pass
                      })
        }
    }

    TestCase {
        name: "Singleton Theme created"
        when: windowShown

        function init() {
            // code to be executed before each test function
            createdbackground.visible = true
            top.createGraphs()
            baritem.visible = false
            scatteritem.visible = false
            surfaceitem.visible = false
        }

        function cleanup() {
            // code to be executed after each test function
            top.destroyGraphs()
        }

        function test_intial_bars3d() {
            compare(top.bars3d.theme.axisX.labelTextColor, "#aeaeae")
            compare(top.bars3d.theme.axisX.mainColor, "#aeabab")
            compare(top.bars3d.theme.axisX.mainWidth, 2)
            compare(top.bars3d.theme.axisX.subColor, "#6a6a6a")
            compare(top.bars3d.theme.axisX.subWidth, 1)
            compare(top.bars3d.theme.axisY.labelTextColor, "#aeaeae")
            compare(top.bars3d.theme.axisY.mainColor, "#aeabab")
            compare(top.bars3d.theme.axisY.mainWidth, 2)
            compare(top.bars3d.theme.axisY.subColor, "#6a6a6a")
            compare(top.bars3d.theme.axisY.subWidth, 1)
            compare(top.bars3d.theme.axisZ.labelTextColor, "#aeaeae")
            compare(top.bars3d.theme.axisZ.mainColor, "#aeabab")
            compare(top.bars3d.theme.axisZ.mainWidth, 2)
            compare(top.bars3d.theme.axisZ.subColor, "#6a6a6a")
            compare(top.bars3d.theme.axisZ.subWidth, 1)
            compare(top.bars3d.theme.backgroundColor, "#262626")
            compare(top.bars3d.theme.backgroundColor, "#262626")
            compare(top.bars3d.theme.backgroundVisible, true)
            compare(top.bars3d.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(top.bars3d.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(top.bars3d.theme.grid.labelTextColor, "#000000")
            compare(top.bars3d.theme.grid.mainColor, "#aeabab")
            compare(top.bars3d.theme.grid.mainWidth, 2)
            compare(top.bars3d.theme.grid.subColor, "#6a6a6a")
            compare(top.bars3d.theme.grid.subWidth, 1)
            compare(top.bars3d.theme.gridVisible, true)
            compare(top.bars3d.theme.labelBackgroundColor, "#2e2e2e")
            compare(top.bars3d.theme.labelFont.pointSize, 60)
            compare(top.bars3d.theme.labelTextColor, "#aeaeae")
            compare(top.bars3d.theme.labelsVisible, true)
            compare(top.bars3d.theme.multiHighlightColor, "#22d489")
            compare(top.bars3d.theme.multiHighlightGradient, null)
            compare(top.bars3d.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(top.bars3d.theme.plotAreaBackgroundVisible, true)
            compare(top.bars3d.theme.singleHighlightColor, "#dbeb00")
            compare(top.bars3d.theme.singleHighlightGradient, null)
            compare(top.bars3d.theme.theme, GraphsTheme.Theme.MixSeries)
        }

        function test_initial_scatter3d() {
            compare(top.scatter3d.theme.axisX.labelTextColor, "#aeaeae")
            compare(top.scatter3d.theme.axisX.mainColor, "#aeabab")
            compare(top.scatter3d.theme.axisX.mainWidth, 2)
            compare(top.scatter3d.theme.axisX.subColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisX.subWidth, 1)
            compare(top.scatter3d.theme.axisY.labelTextColor, "#aeaeae")
            compare(top.scatter3d.theme.axisY.mainColor, "#aeabab")
            compare(top.scatter3d.theme.axisY.mainWidth, 2)
            compare(top.scatter3d.theme.axisY.subColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisY.subWidth, 1)
            compare(top.scatter3d.theme.axisZ.labelTextColor, "#aeaeae")
            compare(top.scatter3d.theme.axisZ.mainColor, "#aeabab")
            compare(top.scatter3d.theme.axisZ.mainWidth, 2)
            compare(top.scatter3d.theme.axisZ.subColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisZ.subWidth, 1)
            compare(top.scatter3d.theme.backgroundColor, "#262626")
            compare(top.scatter3d.theme.backgroundColor, "#262626")
            compare(top.scatter3d.theme.backgroundVisible, true)
            compare(top.scatter3d.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(top.scatter3d.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(top.scatter3d.theme.grid.labelTextColor, "#000000")
            compare(top.scatter3d.theme.grid.mainColor, "#aeabab")
            compare(top.scatter3d.theme.grid.mainWidth, 2)
            compare(top.scatter3d.theme.grid.subColor, "#6a6a6a")
            compare(top.scatter3d.theme.grid.subWidth, 1)
            compare(top.scatter3d.theme.gridVisible, true)
            compare(top.scatter3d.theme.labelBackgroundColor, "#2e2e2e")
            compare(top.scatter3d.theme.labelFont.pointSize, 60)
            compare(top.scatter3d.theme.labelTextColor, "#aeaeae")
            compare(top.scatter3d.theme.labelsVisible, true)
            compare(top.scatter3d.theme.multiHighlightColor, "#22d489")
            compare(top.scatter3d.theme.multiHighlightGradient, null)
            compare(top.scatter3d.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(top.scatter3d.theme.plotAreaBackgroundVisible, true)
            compare(top.scatter3d.theme.singleHighlightColor, "#dbeb00")
            compare(top.scatter3d.theme.singleHighlightGradient, null)
            compare(top.scatter3d.theme.theme, GraphsTheme.Theme.MixSeries)
        }

        function test_initial_surface3d() {
            compare(top.surface3d.theme.axisX.labelTextColor, "#aeaeae")
            compare(top.surface3d.theme.axisX.mainColor, "#aeabab")
            compare(top.surface3d.theme.axisX.mainWidth, 2)
            compare(top.surface3d.theme.axisX.subColor, "#6a6a6a")
            compare(top.surface3d.theme.axisX.subWidth, 1)
            compare(top.surface3d.theme.axisY.labelTextColor, "#aeaeae")
            compare(top.surface3d.theme.axisY.mainColor, "#aeabab")
            compare(top.surface3d.theme.axisY.mainWidth, 2)
            compare(top.surface3d.theme.axisY.subColor, "#6a6a6a")
            compare(top.surface3d.theme.axisY.subWidth, 1)
            compare(top.surface3d.theme.axisZ.labelTextColor, "#aeaeae")
            compare(top.surface3d.theme.axisZ.mainColor, "#aeabab")
            compare(top.surface3d.theme.axisZ.mainWidth, 2)
            compare(top.surface3d.theme.axisZ.subColor, "#6a6a6a")
            compare(top.surface3d.theme.axisZ.subWidth, 1)
            compare(top.surface3d.theme.backgroundColor, "#262626")
            compare(top.surface3d.theme.backgroundColor, "#262626")
            compare(top.surface3d.theme.backgroundVisible, true)
            compare(top.surface3d.theme.colorScheme, GraphsTheme.ColorScheme.Dark)
            compare(top.surface3d.theme.colorStyle, GraphsTheme.ColorStyle.Uniform)
            compare(top.surface3d.theme.grid.labelTextColor, "#000000")
            compare(top.surface3d.theme.grid.mainColor, "#aeabab")
            compare(top.surface3d.theme.grid.mainWidth, 2)
            compare(top.surface3d.theme.grid.subColor, "#6a6a6a")
            compare(top.surface3d.theme.grid.subWidth, 1)
            compare(top.surface3d.theme.gridVisible, true)
            compare(top.surface3d.theme.labelBackgroundColor, "#2e2e2e")
            compare(top.surface3d.theme.labelFont.pointSize, 60)
            compare(top.surface3d.theme.labelTextColor, "#aeaeae")
            compare(top.surface3d.theme.labelsVisible, true)
            compare(top.surface3d.theme.multiHighlightColor, "#22d489")
            compare(top.surface3d.theme.multiHighlightGradient, null)
            compare(top.surface3d.theme.plotAreaBackgroundColor, "#1f1f1f")
            compare(top.surface3d.theme.plotAreaBackgroundVisible, true)
            compare(top.surface3d.theme.singleHighlightColor, "#dbeb00")
            compare(top.surface3d.theme.singleHighlightGradient, null)
            compare(top.surface3d.theme.theme, GraphsTheme.Theme.MixSeries)
        }
    }

    TestCase {
        name: "Singleton Theme created and modified"
        when: windowShown

        function init() {
            // code to be executed before each test function
            createdbackground.visible = true
            top.createGraphs()
            backgroundVisibleThemeSpy.clear()
            colorStyleThemeSpy.clear()
            gridVisibleThemeSpy.clear()
            labelsVisibleThemeSpy.clear()
            plotAreaBackgroundVisibleThemeSpy.clear()
            themeThemeSpy.clear()
            colorSchemeThemeSpy.clear()
            baritem.visible = false
            scatteritem.visible = false
            surfaceitem.visible = false
        }

        function cleanup() {
            // code to be executed after each test function
            top.destroyGraphs()
            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Dark
        }

        function test_colorscheme() {
            // Grab intial graphs
            waitForRendering(top.bars3d)
            let barorigimage = grabImage(top.bars3d)
            let barorigcolor = barorigimage.pixel(30, 30)
            // barorigimage.save("bar-colorscheme-created-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(top.scatter3d)
            let scatterorigimage = grabImage(top.scatter3d)
            let scatterorigcolor = scatterorigimage.pixel(30, 30)
            // scatterorigimage.save("scatter-colorscheme-created-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(top.surface3d)
            let surfaceorigimage = grabImage(top.surface3d)
            let surfaceorigcolor = surfaceorigimage.pixel(30, 30)
            // surfaceorigimage.save("surface-colorscheme-created-origiimage.png") // Uncomment these lines if the test fails

            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Light

            // Bars3D
            compare(top.bars3d.theme.axisX.labelTextColor, "#6a6a6a")
            compare(top.bars3d.theme.axisX.mainColor, "#545151")
            compare(top.bars3d.theme.axisX.subColor, "#afafaf")
            compare(top.bars3d.theme.axisY.labelTextColor, "#6a6a6a")
            compare(top.bars3d.theme.axisY.mainColor, "#545151")
            compare(top.bars3d.theme.axisY.subColor, "#afafaf")
            compare(top.bars3d.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(top.bars3d.theme.axisZ.mainColor, "#545151")
            compare(top.bars3d.theme.axisZ.subColor, "#afafaf")
            compare(top.bars3d.theme.backgroundColor, "#f2f2f2")
            compare(top.bars3d.theme.backgroundColor, "#f2f2f2")
            compare(top.bars3d.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(top.bars3d.theme.grid.labelTextColor, "#000000")
            compare(top.bars3d.theme.grid.mainColor, "#545151")
            compare(top.bars3d.theme.grid.subColor, "#afafaf")
            compare(top.bars3d.theme.labelBackgroundColor, "#e7e7e7")
            compare(top.bars3d.theme.labelTextColor, "#6a6a6a")
            compare(top.bars3d.theme.multiHighlightColor, "#22d47b")
            compare(top.bars3d.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(top.bars3d.theme.singleHighlightColor, "#ccdc00")

            // Scatter3D
            compare(top.scatter3d.theme.axisX.labelTextColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisX.mainColor, "#545151")
            compare(top.scatter3d.theme.axisX.subColor, "#afafaf")
            compare(top.scatter3d.theme.axisY.labelTextColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisY.mainColor, "#545151")
            compare(top.scatter3d.theme.axisY.subColor, "#afafaf")
            compare(top.scatter3d.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(top.scatter3d.theme.axisZ.mainColor, "#545151")
            compare(top.scatter3d.theme.axisZ.subColor, "#afafaf")
            compare(top.scatter3d.theme.backgroundColor, "#f2f2f2")
            compare(top.scatter3d.theme.backgroundColor, "#f2f2f2")
            compare(top.scatter3d.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(top.scatter3d.theme.grid.labelTextColor, "#000000")
            compare(top.scatter3d.theme.grid.mainColor, "#545151")
            compare(top.scatter3d.theme.grid.subColor, "#afafaf")
            compare(top.scatter3d.theme.labelBackgroundColor, "#e7e7e7")
            compare(top.scatter3d.theme.labelTextColor, "#6a6a6a")
            compare(top.scatter3d.theme.multiHighlightColor, "#22d47b")
            compare(top.scatter3d.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(top.scatter3d.theme.singleHighlightColor, "#ccdc00")

            // Surface3D
            compare(top.surface3d.theme.axisX.labelTextColor, "#6a6a6a")
            compare(top.surface3d.theme.axisX.mainColor, "#545151")
            compare(top.surface3d.theme.axisX.subColor, "#afafaf")
            compare(top.surface3d.theme.axisY.labelTextColor, "#6a6a6a")
            compare(top.surface3d.theme.axisY.mainColor, "#545151")
            compare(top.surface3d.theme.axisY.subColor, "#afafaf")
            compare(top.surface3d.theme.axisZ.labelTextColor, "#6a6a6a")
            compare(top.surface3d.theme.axisZ.mainColor, "#545151")
            compare(top.surface3d.theme.axisZ.subColor, "#afafaf")
            compare(top.surface3d.theme.backgroundColor, "#f2f2f2")
            compare(top.surface3d.theme.backgroundColor, "#f2f2f2")
            compare(top.surface3d.theme.colorScheme, GraphsTheme.ColorScheme.Light)
            compare(top.surface3d.theme.grid.labelTextColor, "#000000")
            compare(top.surface3d.theme.grid.mainColor, "#545151")
            compare(top.surface3d.theme.grid.subColor, "#afafaf")
            compare(top.surface3d.theme.labelBackgroundColor, "#e7e7e7")
            compare(top.surface3d.theme.labelTextColor, "#6a6a6a")
            compare(top.surface3d.theme.multiHighlightColor, "#22d47b")
            compare(top.surface3d.theme.plotAreaBackgroundColor, "#fcfcfc")
            compare(top.surface3d.theme.singleHighlightColor, "#ccdc00")

            // Signals
            compare(colorSchemeThemeSpy.count, 1)

            // Grab and check changed graphs
            waitForRendering(top.bars3d)
            tryVerify(() => {
                          let barafterimage = grabImage(top.bars3d)
                          let pass = !barafterimage.equals(barorigimage)
                            && (barafterimage.pixel(30, 30) != barorigcolor)
                          if (!pass)
                            barafterimage.save("bar-colorscheme-created-afterimage.png")
                          return pass
                      })

            waitForRendering(top.scatter3d)
            tryVerify(() => {
                          let scatterafterimage = grabImage(top.scatter3d)
                          let pass = !scatterafterimage.equals(scatterorigimage)
                            && (scatterafterimage.pixel(30, 30) != scatterorigcolor)
                          if (!pass)
                            scatterafterimage.save("scatter-colorscheme-created-afterimage.png")
                          return pass
                      })

            waitForRendering(top.surface3d)
            tryVerify(() => {
                          let surfaceafterimage = grabImage(top.surface3d)
                          let pass = !surfaceafterimage.equals(surfaceorigimage)
                            && (surfaceafterimage.pixel(30, 30) != surfaceorigcolor)
                          if (!pass)
                            surfaceafterimage.save("surface-colorscheme-created-afterimage.png")
                          return pass
                      })

            // Change color scheme back
            SingletonTheme.sharedtheme.colorScheme = GraphsTheme.ColorScheme.Dark

            // Grab and check returned graphs
            waitForRendering(top.bars3d)
            tryVerify(() => {
                          let barbackimage = grabImage(top.bars3d)
                          let pass = barbackimage.equals(barorigimage)
                            && (barbackimage.pixel(30, 30) == barorigcolor)
                          if (!pass)
                            barbackimage.save("bar-colorscheme-created-backimage.png")
                          return pass
                      })

            waitForRendering(top.scatter3d)
            tryVerify(() => {
                          let scatterbackimage = grabImage(top.scatter3d)
                          let pass = scatterbackimage.equals(scatterorigimage)
                            && (scatterbackimage.pixel(30, 30) == scatterorigcolor)
                          if (!pass)
                            scatterbackimage.save("scatter-colorscheme-created-backimage.png")
                          return pass
                      })

            waitForRendering(top.surface3d)
            tryVerify(() => {
                          let surfacebackimage = grabImage(top.surface3d)
                          let pass = surfacebackimage.equals(surfaceorigimage)
                            && (surfacebackimage.pixel(30, 30) == surfaceorigcolor)
                          if (!pass)
                            surfacebackimage.save("surface-colorscheme-created-backimage.png")
                          return pass
                      })
        }
    }

    TestCase {
        name: "Singleton Theme created and modified"
        when: windowShown

        function init() {
            // code to be executed before each test function
            createdbackground.visible = true
            top.createGraphs()
        }

        function cleanup() {
            // code to be executed after each test function
            top.destroyGraphs()
            SingletonTheme.sharedtheme.backgroundVisible = true
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.Uniform
            SingletonTheme.sharedtheme.gridVisible = true
            SingletonTheme.sharedtheme.labelsVisible = true
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = true
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.MixSeries
        }

        function test_properties() {
            // Grab intial graphs
            waitForRendering(top.bars3d)
            let barorigimage = grabImage(top.bars3d)
            let barorigcolor = barorigimage.pixel(30, 30)
            // barorigimage.save("bar-properties-created-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(top.scatter3d)
            let scatterorigimage = grabImage(top.scatter3d)
            let scatterorigcolor = scatterorigimage.pixel(30, 30)
            // scatterorigimage.save("scatter-properties-created-origiimage.png") // Uncomment these lines if the test fails

            waitForRendering(top.surface3d)
            let surfaceorigimage = grabImage(top.surface3d)
            let surfaceorigcolor = surfaceorigimage.pixel(30, 30)
            // surfaceorigimage.save("surface-properties-created-origiimage.png") // Uncomment these lines if the test fails

            SingletonTheme.sharedtheme.backgroundVisible = false
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.ObjectGradient
            SingletonTheme.sharedtheme.gridVisible = false
            SingletonTheme.sharedtheme.labelsVisible = false
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = false
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.QtGreenNeon

            // Bars3D
            compare(top.bars3d.theme.backgroundVisible, false)
            compare(top.bars3d.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(top.bars3d.theme.gridVisible, false)
            compare(top.bars3d.theme.labelsVisible, false)
            compare(top.bars3d.theme.plotAreaBackgroundVisible, false)
            compare(top.bars3d.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Scatter3D
            compare(top.scatter3d.theme.backgroundVisible, false)
            compare(top.scatter3d.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(top.scatter3d.theme.gridVisible, false)
            compare(top.scatter3d.theme.labelsVisible, false)
            compare(top.scatter3d.theme.plotAreaBackgroundVisible, false)
            compare(top.scatter3d.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Surface3D
            compare(top.surface3d.theme.backgroundVisible, false)
            compare(top.surface3d.theme.colorStyle, GraphsTheme.ColorStyle.ObjectGradient)
            compare(top.surface3d.theme.gridVisible, false)
            compare(top.surface3d.theme.labelsVisible, false)
            compare(top.surface3d.theme.plotAreaBackgroundVisible, false)
            compare(top.surface3d.theme.theme, GraphsTheme.Theme.QtGreenNeon)

            // Signals
            compare(backgroundVisibleThemeSpy.count, 1)
            compare(colorStyleThemeSpy.count, 1)
            compare(gridVisibleThemeSpy.count, 1)
            compare(labelsVisibleThemeSpy.count, 1)
            compare(plotAreaBackgroundVisibleThemeSpy.count, 1)
            compare(themeThemeSpy.count, 1)

            // Grab and check changed graphs
            waitForRendering(top.bars3d)
            tryVerify(() => {
                          let barafterimage = grabImage(top.bars3d)
                          let pass = !barafterimage.equals(barorigimage)
                            && (barafterimage.pixel(30, 30) == createdbackground.color)
                          if (!pass)
                            barafterimage.save("bar-properties-created-afterimage.png")
                          return pass
                      })

            waitForRendering(top.scatter3d)
            tryVerify(() => {
                          let scatterafterimage = grabImage(top.scatter3d)
                          let pass = !scatterafterimage.equals(scatterorigimage)
                            && (scatterafterimage.pixel(30, 30) == createdbackground.color)
                          if (!pass)
                            scatterafterimage.save("scatter-properties-created-afterimage.png")
                          return pass
                      })

            waitForRendering(top.surface3d)
            tryVerify(() => {
                          let surfaceafterimage = grabImage(top.surface3d)
                          let pass = !surfaceafterimage.equals(surfaceorigimage)
                            && (surfaceafterimage.pixel(30, 30) == createdbackground.color)
                          if (!pass)
                            surfaceafterimage.save("surface-properties-created-afterimage.png")
                          return pass
                      })

            // Change color scheme back
            SingletonTheme.sharedtheme.backgroundVisible = true
            SingletonTheme.sharedtheme.colorStyle = GraphsTheme.ColorStyle.Uniform
            SingletonTheme.sharedtheme.gridVisible = true
            SingletonTheme.sharedtheme.labelsVisible = true
            SingletonTheme.sharedtheme.plotAreaBackgroundVisible = true
            SingletonTheme.sharedtheme.theme = GraphsTheme.Theme.MixSeries

            // Grab and check returned graphs
            waitForRendering(top.bars3d)
            tryVerify(() => {
                          let barbackimage = grabImage(top.bars3d)
                          let pass = barbackimage.equals(barorigimage)
                            && (barbackimage.pixel(30, 30) == barorigcolor)
                          if (!pass)
                            barbackimage.save("bar-properties-created-backimage.png")
                          return pass
                      })

            waitForRendering(top.scatter3d)
            tryVerify(() => {
                          let scatterbackimage = grabImage(top.scatter3d)
                          let pass = scatterbackimage.equals(scatterorigimage)
                            && (scatterbackimage.pixel(30, 30) == scatterorigcolor)
                          if (!pass)
                            scatterbackimage.save("scatter-properties-created-backimage.png")
                          return pass
                      })

            waitForRendering(top.surface3d)
            tryVerify(() => {
                          let surfacebackimage = grabImage(top.surface3d)
                          let pass = surfacebackimage.equals(surfaceorigimage)
                            && (surfacebackimage.pixel(30, 30) == surfaceorigcolor)
                          if (!pass)
                            surfacebackimage.save("surface-properties-created-backimage.png")
                          return pass
                      })
        }
    }

    // Theme signaling
    SignalSpy {
        id: backgroundVisibleThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "backgroundVisibleChanged"
    }

    SignalSpy {
        id: colorStyleThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "colorStyleChanged"
    }

    SignalSpy {
        id: gridVisibleThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "gridVisibleChanged"
    }

    SignalSpy {
        id: labelsVisibleThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "labelsVisibleChanged"
    }

    SignalSpy {
        id: plotAreaBackgroundVisibleThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "plotAreaBackgroundVisibleChanged"
    }

    SignalSpy {
        id: themeThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "themeChanged"
    }

    SignalSpy {
        id: colorSchemeThemeSpy
        target: SingletonTheme.sharedtheme
        signalName: "colorSchemeChanged"
    }
}
