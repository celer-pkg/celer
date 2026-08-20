// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import QtPositioning
import QtLocation.Test

Item {
    // General-purpose elements for the test:
    id: page
    width: 120
    height: 120
    Plugin { id: testPlugin; name: "qmlgeo.test.plugin"; allowExperimental: true }

    property variant coordinate: QtPositioning.coordinate(10, 11)

    MouseArea {
        id: mouseAreaBottom
        anchors.fill: parent
        visible: false
    }

    MapView {
        id: mapView
        map.plugin: testPlugin
        map.center: coordinate;
        map.zoomLevel: 9;
        anchors.fill: page
        x:0; y:0

        property real flickStartedLatitude
        property real flickStartedLongitude

        property PinchHandler __pinch: mapView.map.data[0] // verified in init()
        property DragHandler __drag: mapView.map.data[2]

        MouseArea {
            id: mouseAreaTop
            anchors.fill: parent
            visible: false
        }
    }

    SignalSpy {id: centerSpy; target: mapView.map; signalName: 'centerChanged'}
    SignalSpy {id: panActiveSpy; target: mapView.__drag; signalName: 'activeChanged'}
    SignalSpy {id: flickStartedSpy; target: mapView.__drag; signalName: 'flickStarted'}
    SignalSpy {id: flickFinishedSpy; target: mapView.__drag; signalName: 'flickEnded'}
    SignalSpy {id: mouseAreaTopSpy; target: mouseAreaTop; signalName: 'onPressed'}
    SignalSpy {id: mouseAreaBottomSpy; target: mouseAreaBottom; signalName: 'onPressed'}

    TestCase {
        when: windowShown && mapView.map.mapReady
        name: "MapFlick"

        function init()
        {
            if (Qt.platform.os === "windows" && (LocationTestHelper.x86Bits() === 32))
                skip("QTBUG-59503")
            mapView.map.zoomLevel = 9 // or flicking diagonally won't work
            centerSpy.clear()
            panActiveSpy.clear()
            flickStartedSpy.clear()
            flickFinishedSpy.clear()
            mouseAreaTopSpy.clear()
            mouseAreaBottomSpy.clear()
            mouseAreaBottom.visible = false
            mouseAreaTop.visible = false

            // sanity check: verify that map's first child is the PinchHandler
            verify(mapView.__pinch.hasOwnProperty("maximumRotation"))
            // sanity check: verify that map's third child is the main DragHandler
            compare(mapView.__drag.minimumPointCount, 1)
            mapView.__drag.onFlickStarted.connect(function() {
                mapView.flickStartedLatitude = mapView.map.center.latitude
                mapView.flickStartedLongitude = mapView.map.center.longitude
            })
            compare(mapView.__pinch.active, false)
            compare(mapView.__drag.active, false)
        }

        function flick_down()
        {
            mapView.map.center.latitude = 10
            mapView.map.center.longitude = 11
            mousePress(page, 0, 50)
            for (var i = 0; i < 50; i += 5) {
                wait(25)
                mouseMove(page, 0, (50 + i), 0, Qt.LeftButton);
            }
            mouseRelease(page, 0, 100)

            verify(mapView.map.center.latitude > 10) // latitude increases we are going 'up/north' (moving mouse down)
            var moveLatitude = mapView.map.center.latitude // store lat and check that flick continues

            tryCompare(flickStartedSpy, "count", 1)
            tryCompare(panActiveSpy, "count", 2)
            tryCompare(flickFinishedSpy, "count", 1)

            verify(mapView.map.center.latitude > moveLatitude)
            compare(mapView.map.center.longitude, 11) // should remain the same
        }

        function test_flick_down()
        {
            flick_down()
        }

        function test_flick_down_with_filtering()
        {
            mouseAreaTop.visible = true
            mouseAreaBottom.visible = true
            flick_down()
            tryCompare(mouseAreaTopSpy, "count", 1)
            tryCompare(mouseAreaBottomSpy, "count",0)
        }

        function flick_up()
        {
            mapView.map.center.latitude = 70
            mapView.map.center.longitude = 11
            mousePress(page, 10, 95)
            for (var i = 45; i > 0; i -= 5) {
                wait(25)
                mouseMove(page, 10, (50 + i), 0, Qt.LeftButton);
            }
            mouseRelease(page, 10, 50)
            var moveLatitude = mapView.map.center.latitude // store lat and check that flick continues
            tryCompare(flickStartedSpy, "count", 1)
            tryCompare(panActiveSpy, "count", 2)
            tryCompare(flickFinishedSpy, "count", 1)
            verify(moveLatitude < 70)
            verify(mapView.map.center.latitude < moveLatitude)
            compare(mapView.map.center.longitude, 11) // should remain the same
        }

        function test_flick_up()
        {
            flick_up()
        }

        function test_flick_up_with_filtering()
        {
            mouseAreaTop.visible = true
            mouseAreaBottom.visible = true
            flick_up()
            tryCompare(mouseAreaTopSpy, "count", 1)
            tryCompare(mouseAreaBottomSpy, "count",0)
        }

        function test_flick_diagonal()
        {
            mapView.map.center.latitude = 50
            mapView.map.center.longitude = 50
            var pos = 5
            //console.log("start", mapView.map.center.latitude, mapView.map.center.longitude)
            mousePress(page, pos, pos)
            for (var i = pos; i < 50; i += 5) {
                pos = i
                wait(25)
                mouseMove(page, pos, pos, 0, Qt.LeftButton);
            }
            mouseRelease(page, pos, pos)
            //console.log("flick started", mapView.flickStartedLatitude, mapView.flickStartedLongitude)
            verify(mapView.flickStartedLatitude > 50)
            verify(mapView.flickStartedLongitude < 50)
            tryCompare(flickStartedSpy, "count", 1)
            tryCompare(panActiveSpy, "count", 2)
            tryCompare(flickFinishedSpy, "count", 2)
            //console.log("after flick", mapView.map.center.latitude, mapView.map.center.longitude)
            verify(mapView.map.center.latitude > mapView.flickStartedLatitude)
            verify(mapView.map.center.longitude < mapView.flickStartedLongitude)
        }

        /*
            Regression test for QTBUG-67580 where touching the map
            (as opposing to clicking it with a mouse) would cause a crash.
            So this test is merely about surviving a tap.
         */
        function test_touch()
        {
            touchEvent(mapView).press(0).commit();
            wait(25);
            touchEvent(mapView).release(0).commit();
        }
    }
}
