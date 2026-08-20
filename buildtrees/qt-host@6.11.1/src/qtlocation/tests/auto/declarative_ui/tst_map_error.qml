// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import QtPositioning

Item {
    id: page
    x: 0; y: 0;
    width: 200
    height: 100
    property variant coordinate: QtPositioning.coordinate(20, 20)

    Plugin {
        id: errorPlugin; name: "qmlgeo.test.plugin"; allowExperimental: true
        parameters: [
            PluginParameter { name: "error"; value: "1"},
            PluginParameter { name: "errorString"; value: "This error was expected. No worries !"}
        ]
    }

    MapView {
        id: map_error_plugin;
        property alias mouseClickedSpy: mouseClickedSpy1
        x: 0; y: 0; width: 100; height: 100; map.plugin: errorPlugin;

        MouseArea {
            id: mouseArea1
            objectName: "mouseArea"
            x: 25; y: 25; width: 50; height: 50;
            preventStealing: true
        }

        SignalSpy {id: mouseClickedSpy1; target: mouseArea1; signalName: "clicked"}
    }

    MapView {
        id: map_no_plugin;
        property alias mouseClickedSpy: mouseClickedSpy2
        x: 100; y: 0; width: 100; height: 100;

        MouseArea {
            id: mouseArea2
            objectName: "mouseArea"
            x: 25; y: 25; width: 50; height: 50;
            preventStealing: true
        }

        SignalSpy {id: mouseClickedSpy2; target: mouseArea2; signalName: "clicked"}
    }

    TestCase {
        name: "MapErrorHandling"
        when: windowShown

        function init() {
            map_error_plugin.map.zoomLevel = 0
            map_no_plugin.map.zoomLevel = 0
            map_error_plugin.map.center = QtPositioning.coordinate(0, 0)
            map_no_plugin.map.center = QtPositioning.coordinate(0, 0)
            map_error_plugin.mouseClickedSpy.clear()
            map_no_plugin.mouseClickedSpy.clear()
        }

        function map_clicked(map)
        {
            mouseClick(map, 5, 5)
            mouseClick(map, 50, 50)
            mouseClick(map, 50, 50)
            mouseClick(map, 50, 50)
            tryCompare(map.mouseClickedSpy, "count", 3)
        }

        function test_map_clicked_wiht_no_plugin()
        {
            map_clicked(map_no_plugin)
        }

        function test_map_clicked_with_error_plugin()
        {
            map_clicked(map_error_plugin)
        }

        function test_map_no_supportedMapTypes()
        {
            compare(map_no_plugin.map.supportedMapTypes.length , 0)
            compare(map_error_plugin.map.supportedMapTypes.length , 0)
        }

        function test_map_set_zoom_level()
        {
            map_no_plugin.map.zoomLevel = 9
            compare(map_no_plugin.map.zoomLevel,9)
            map_error_plugin.map.zoomLevel = 9
            compare(map_error_plugin.map.zoomLevel,9)
        }

        function test_map_set_center()
        {
            map_no_plugin.map.center = coordinate
            verify(map_no_plugin.map.center === coordinate)
            map_error_plugin.map.center = coordinate
            verify(map_error_plugin.map.center === coordinate)
        }

        function test_map_no_mapItems()
        {
            compare(map_no_plugin.map.mapItems.length , 0)
            compare(map_error_plugin.map.mapItems.length , 0)
        }

        function test_map_error()
        {
            compare(map_no_plugin.map.error , 0)
            compare(map_no_plugin.map.errorString , "")
            compare(map_error_plugin.map.error , 1)
            compare(map_error_plugin.map.errorString ,"This error was expected. No worries !")
        }

        function test_map_toCoordinate()
        {
            map_no_plugin.map.center = coordinate
            compare(map_no_plugin.map.toCoordinate(Qt.point(50,50)).isValid,false)
            map_error_plugin.map.center = coordinate
            compare(map_error_plugin.map.toCoordinate(Qt.point(50,50)).isValid,false)
        }

        function test_map_fromCoordinate()
        {
            verify(isNaN(map_error_plugin.map.fromCoordinate(coordinate).x))
            verify(isNaN(map_error_plugin.map.fromCoordinate(coordinate).y))
            verify(isNaN(map_no_plugin.map.fromCoordinate(coordinate).x))
            verify(isNaN(map_no_plugin.map.fromCoordinate(coordinate).y))
        }

        function test_map_pan()
        {
            map_no_plugin.map.center = coordinate
            map_no_plugin.map.pan(20,20)
            verify(map_no_plugin.map.center === coordinate)
            map_error_plugin.map.center = coordinate
            map_error_plugin.map.pan(20,20)
            verify(map_error_plugin.map.center === coordinate)
        }

        function test_map_prefetchData()
        {
            map_error_plugin.map.prefetchData()
            map_no_plugin.map.prefetchData()
        }

        function test_map_fitViewportToMapItems()
        {
            map_error_plugin.map.fitViewportToMapItems()
            map_no_plugin.map.fitViewportToMapItems()
        }

        function test_map_setVisibleRegion()
        {
            map_no_plugin.map.visibleRegion = QtPositioning.circle(coordinate,1000)
            verify(map_no_plugin.map.center != coordinate)
            verify(map_no_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,0)) == true)
            verify(map_no_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,90)) == true)
            verify(map_no_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,180)) == true)
            verify(map_no_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,270)) == true)
            map_error_plugin.map.visibleRegion = QtPositioning.circle(coordinate,1000)
            verify(map_error_plugin.map.center != coordinate)
            verify(map_error_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,0)) == true)
            verify(map_error_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,90)) == true)
            verify(map_error_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,180)) == true)
            verify(map_error_plugin.map.visibleRegion.contains(coordinate.atDistanceAndAzimuth(1000,270)) == true)
        }

        function test_map_activeMapType()
        {
            compare(map_no_plugin.map.supportedMapTypes.length, 0)
            compare(map_no_plugin.map.activeMapType.style, MapType.NoMap)
            compare(map_error_plugin.map.supportedMapTypes.length, 0)
            compare(map_error_plugin.map.activeMapType.style, MapType.NoMap)
        }
    }
}
