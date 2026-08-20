// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    GraphsView {
        id: initial
    }

    GraphsTheme {
        id: myTheme
        theme: GraphsTheme.Theme.QtGreenNeon
        colorScheme: Qt.Dark
    }

    GraphsTheme {
        id: newTheme
    }

    GraphsView {
        id: initialized
        height: top.height
        width: top.width
        theme: myTheme
        marginBottom: 5
        marginLeft: 10
        marginRight: 10
        marginTop: 5
        clipPlotArea: false

        axisX: BarCategoryAxis {
            id: axisXInitial
            categories: ["2002", "2003", "2004"]
        }

        axisY: ValueAxis {
            id: axisYInitial
            max: 4
        }

        BarSeries {
            id: barInitial
        }

        LineSeries {
            id: lineInitial
        }

        AreaSeries {
            id: areaInitial
        }
    }

    BarCategoryAxis {
        id: axisX
        categories: ["2012", "2013", "2014"]
    }

    ValueAxis {
        id: axisY
        max: 8
    }

    BarSeries {
        id: addedSeries
    }

    LineSeries {
        id: insertSeries
    }

    TestCase {
        name: "GraphsView Initial"

        function test_1_initial() {
            compare(initial.height, 0);
            compare(initial.width, 0);
            compare(initial.marginTop, 20);
            compare(initial.marginBottom, 20);
            compare(initial.marginLeft, 20);
            compare(initial.marginRight, 20);
            compare(initial.plotArea.x, 0);
            compare(initial.plotArea.y, 0);
            compare(initial.plotArea.width, 0);
            compare(initial.plotArea.height, 0);
            compare(initial.seriesList, []);
            compare(initial.axisX, null);
            compare(initial.axisY, null);
            compare(initial.panStyle, GraphsView.PanStyle.None);
            compare(initial.zoomStyle, GraphsView.ZoomStyle.None);
            // compare some of the contents of the initial theme, as theme itself cannot be
            compare(initial.theme.theme, GraphsTheme.Theme.QtGreen);
            compare(initial.theme.colorScheme, GraphsTheme.ColorScheme.Automatic);
            compare(initial.theme.seriesColors.length, 5);
            compare(initial.clipPlotArea, true);
        }

        function test_1_initial_change() {
            initial.height = 100;
            initial.width = 100;
            initial.marginTop = 10;
            initial.marginBottom = 11;
            initial.marginLeft = 12;
            initial.marginRight = 13;
            initial.theme = myTheme;
            initial.axisX = axisX;
            initial.axisY = axisY;
            initial.panStyle = GraphsView.PanStyle.Drag;
            initial.zoomStyle = GraphsView.ZoomStyle.Center;
            initial.addSeries(barInitial);
            initial.clipPlotArea = false;
            waitForRendering(top);

            compare(initial.height, 100);
            compare(initial.width, 100);
            compare(initial.marginTop, 10);
            compare(initial.marginBottom, 11);
            compare(initial.marginLeft, 12);
            compare(initial.marginRight, 13);
            verify(initial.plotArea.x !== 20);
            verify(initial.plotArea.y !== 20);
            verify(initial.plotArea.width !== 0);
            verify(initial.plotArea.height !== 0);
            compare(initial.axisX, axisX);
            compare(initial.axisY, axisY);
            compare(initial.panStyle, GraphsView.PanStyle.Drag);
            compare(initial.zoomStyle, GraphsView.ZoomStyle.Center);
            compare(initial.seriesList, [barInitial]);
            compare(initial.theme, myTheme);
            compare(initial.theme.theme, GraphsTheme.Theme.QtGreenNeon);
            compare(initial.theme.colorScheme, Qt.Dark);
            compare(initial.theme.seriesColors.length, 5);
            compare(initial.clipPlotArea, false);
        }

        function test_2_getDataPointCoordinate() {
            var point = lineInitial.dataPointCoordinatesAt(0, 0);
            compare(point.x, 0);
            compare(point.y, 8);
            var point = lineInitial.dataPointCoordinatesAt(1, 1);
            compare(point.x, 0.04);
            compare(point.y, 7.89873417721519);
            var point = lineInitial.dataPointCoordinatesAt(4, 4);
            compare(point.x, 0.16);
            compare(point.y, 7.594936708860759);
        }
    }

    TestCase {
        name: "GraphsView Initialized"

        function test_1_initialized() {
            compare(initialized.height, top.height);
            compare(initialized.width, top.width);
            compare(initialized.marginTop, 5);
            compare(initialized.marginBottom, 5);
            compare(initialized.marginLeft, 10);
            compare(initialized.marginRight, 10);
            compare(initialized.seriesList, [barInitial, lineInitial, areaInitial]);
            compare(initialized.theme, myTheme);
            compare(initialized.theme.theme, GraphsTheme.Theme.QtGreenNeon);
            compare(initialized.theme.colorScheme, Qt.Dark);
            compare(initialized.theme.seriesColors.length, 5);
            compare(initialized.axisX, axisXInitial);
            compare(initialized.axisY, axisYInitial);
            compare(initialized.clipPlotArea, false);
        }

        function test_2_initialized_change() {
            initialized.height = 100;
            initialized.width = 100;
            initialized.marginTop = 10;
            initialized.marginBottom = 11;
            initialized.marginLeft = 12;
            initialized.marginRight = 13;
            initialized.theme = newTheme;
            initialized.axisX = axisX;
            initialized.axisY = axisY;
            initialized.axisXSmoothing = 10;
            initialized.axisYSmoothing = 10;
            initialized.gridSmoothing = 10;

            initialized.shadowVisible = true;
            initialized.shadowColor = "#ff00ff";
            initialized.shadowXOffset = 10;
            initialized.shadowYOffset = 10;
            initialized.shadowSmoothing = 10;
            initialized.shadowBarWidth = 20;
            initialized.clipPlotArea = true;

            initialized.removeSeries(barInitial);
            initialized.removeSeries(1); // areaInitial

            waitForRendering(top);

            compare(initialized.height, 100);
            compare(initialized.width, 100);
            compare(initialized.marginTop, 10);
            compare(initialized.marginBottom, 11);
            compare(initialized.marginLeft, 12);
            compare(initialized.marginRight, 13);
            compare(initialized.axisX, axisX);
            compare(initialized.axisY, axisY);
            compare(initialized.seriesList, [lineInitial]);
            compare(initialized.theme, newTheme);
            compare(initialized.theme.theme, GraphsTheme.Theme.QtGreen);
            compare(initialized.theme.colorScheme, GraphsTheme.ColorScheme.Automatic);
            compare(initialized.theme.seriesColors.length, 5);
            compare(initialized.clipPlotArea, true);

            compare(themeSpy.count, 2);
            compare(marginTopSpy.count, 1);
            compare(marginBottomSpy.count, 1);
            compare(marginLeftSpy.count, 1);
            compare(marginRightSpy.count, 1);
            compare(axisXSmoothingSpy.count, 1);
            compare(axisYSmoothingSpy.count, 1);
            compare(gridSmoothingSpy.count, 1);
            compare(shadowVisibleSpy.count, 1);
            compare(shadowColorSpy.count, 1);
            compare(shadowBarWidthSpy.count, 1);
            compare(shadowXOffsetSpy.count, 1);
            compare(shadowYOffsetSpy.count, 1);
            compare(axisXSpy.count, 1);
            compare(axisYSpy.count, 1);
            compare(orientationSpy.count, 0);
            compare(clipPlotAreaSpy.count, 1);
        }

        function test_3_initialized_change_to_invalid() {
            initialized.theme = null;
            initialized.addSeries(areaInitial);
            initialized.addSeries(null);
            initialized.addSeries(myTheme);
            initialized.removeSeries(-1);
            initialized.removeSeries(15);

            waitForRendering(top);

            compare(initialized.seriesList, [lineInitial, areaInitial]);
            // Using default theme, so not null and not any of ours.
            verify(initialized.theme !== null);
            verify(initialized.theme !== myTheme);
            verify(initialized.theme !== newTheme);
        }

        function test_4_initialized_change_to_null() {
            initialized.axisX = null;
            initialized.axisY = null;

            waitForRendering(top);

            compare(initial.axisX, null);
            compare(initial.axisY, null);
        }

        function test_5_initialized_add_remove() {
            initialized.addSeries(addedSeries);
            initialized.insertSeries(0, insertSeries);

            waitForRendering(top);

            compare(initialized.seriesList, [insertSeries, lineInitial, areaInitial, addedSeries]);

            initialized.removeSeries(insertSeries);
            initialized.removeSeries(2);

            waitForRendering(top);

            compare(initialized.seriesList, [lineInitial, areaInitial]);

            let hasSerie = initialized.hasSeries(lineInitial);
            compare(hasSerie, true);
            hasSerie = initialized.hasSeries(addedSeries);
            compare(hasSerie, false);
        }

        SignalSpy {
            id: themeSpy
            target: initialized
            signalName: "themeChanged"
        }

        SignalSpy {
            id: marginTopSpy
            target: initialized
            signalName: "marginTopChanged"
        }

        SignalSpy {
            id: marginBottomSpy
            target: initialized
            signalName: "marginBottomChanged"
        }

        SignalSpy {
            id: marginLeftSpy
            target: initialized
            signalName: "marginLeftChanged"
        }

        SignalSpy {
            id: marginRightSpy
            target: initialized
            signalName: "marginRightChanged"
        }

        SignalSpy {
            id: axisXSmoothingSpy
            target: initialized
            signalName: "axisXSmoothingChanged"
        }

        SignalSpy {
            id: axisYSmoothingSpy
            target: initialized
            signalName: "axisYSmoothingChanged"
        }

        SignalSpy {
            id: gridSmoothingSpy
            target: initialized
            signalName: "gridSmoothingChanged"
        }

        SignalSpy {
            id: shadowVisibleSpy
            target: initialized
            signalName: "shadowVisibleChanged"
        }

        SignalSpy {
            id: shadowColorSpy
            target: initialized
            signalName: "shadowColorChanged"
        }

        SignalSpy {
            id: shadowBarWidthSpy
            target: initialized
            signalName: "shadowBarWidthChanged"
        }

        SignalSpy {
            id: shadowXOffsetSpy
            target: initialized
            signalName: "shadowXOffsetChanged"
        }

        SignalSpy {
            id: shadowYOffsetSpy
            target: initialized
            signalName: "shadowYOffsetChanged"
        }

        SignalSpy {
            id: shadowSmoothingSpy
            target: initialized
            signalName: "shadowSmoothingChanged"
        }

        SignalSpy {
            id: axisXSpy
            target: initialized
            signalName: "axisXChanged"
        }

        SignalSpy {
            id: axisYSpy
            target: initialized
            signalName: "axisYChanged"
        }

        SignalSpy {
            id: orientationSpy
            target: initialized
            signalName: "orientationChanged"
        }
        SignalSpy {
            id: clipPlotAreaSpy
            target: initialized
            signalName: "clipPlotAreaChanged"
        }
    }
}
