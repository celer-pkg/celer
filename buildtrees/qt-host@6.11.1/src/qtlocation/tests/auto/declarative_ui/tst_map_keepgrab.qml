// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import QtPositioning

Item {
    // General-purpose elements for the test:
    id: page
    width: 200
    height: 200
    Plugin { id: testPlugin; name: "qmlgeo.test.plugin"; allowExperimental: true }


    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: flickable.width * 4; contentHeight: flickable.height

        MapView {
            id: mapView
            x: flickable.width
            height: flickable.height - 10
            width: flickable.width - 10
            map.plugin: testPlugin

            property DragHandler __drag: mapView.map.data[2] // verified in init()
        }
    }

    SignalSpy { id: panActiveSpy; target: mapView.__drag; signalName: 'activeChanged' }
    SignalSpy { id: flickStartedSpy; target: flickable; signalName: 'flickStarted' }
    SignalSpy { id: flickEndedSpy; target: flickable; signalName: 'flickEnded' }


    TestCase {
        when: windowShown && mapView.map.mapReady
        name: "FlickableKeepGrabAndPreventSteal"

        function init()
        {
            mapView.map.zoomLevel = 1
            mapView.map.center = QtPositioning.coordinate(50,50)
            flickable.contentX = 0
            flickable.contentY = 0
            panActiveSpy.clear()
            flickStartedSpy.clear()
            flickEndedSpy.clear()
            // sanity check: verify that map's third child is the main DragHandler
            compare(mapView.__drag.minimumPointCount, 1)
        }

        function flick()
        {
            var i = 0
            mousePress(flickable, flickable.width - 1, 0)
            for (i = flickable.width; i > 0; i -= 5) {
                wait(5)
                mouseMove(flickable, i, 0, 0, Qt.LeftButton);
            }
            mouseRelease(flickable, i, 0)
        }

        function pan()
        {
            var i = 0
            mousePress(mapView, 0, 0)
            for (i = 0; i < flickable.width; i += 5) {
                wait(5)
                mouseMove(mapView, i, 0, 0, Qt.LeftButton);
            }
            mouseRelease(mapView, i, 0)
        }

        function test_flickable_preventsteal_data()
        {
            return [
                {tag: "flick,pan", stop: false},
                {tag: "flick,stop,pan", stop: true},
            ]
        }

        function test_flickable_preventsteal(data)
        {
            // This test will verify that once the user has started to flick the
            // outer flickable, the inner MapView will not steal the grab until the
            // flicking interaction has stopped.
            var center = QtPositioning.coordinate(mapView.map.center.latitude,mapView.map.center.longitude)
            flick() // flick flickable
            tryCompare(flickStartedSpy,"count",1)
            compare(flickable.flicking, true)
            compare(flickable.moving, true)

            if (data.stop) {
                // stop the flicking with a tap on top of the map
                mousePress(mapView, 0, 0)
                compare(flickable.dragging, false)
                compare(flickable.flicking, false)
                compare(flickable.moving, true)
                mouseRelease(mapView, 0, 0)
                compare(flickable.moving, false)
                compare(panActiveSpy.count, 0)
                // Try to pan the map. Since the flickable is no longer moving, it should succeed
                pan()
                compare(flickStartedSpy.count, 1)
                compare(flickable.flicking, false)
                compare(flickable.moving, false)
                tryCompare(panActiveSpy, "count", 2)
                // map should change
                verify(center !== mapView.map.center)
            } else {
                // Try to pan the map. Since the flickable is still moving, it will
                // not succeed. A drag on a descendant while the flickable is moving
                // will only add (or reduce) momentum to the ongoing flick.
                pan()
                compare(flickStartedSpy.count, 2)
                compare(flickable.flicking, true)
                compare(flickable.moving, true)
                tryCompare(panActiveSpy, "count", 0)
                // map shouldn't change
                verify(center === mapView.map.center)
            }
        }
    }
}
