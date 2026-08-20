// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import QtPositioning

Item {
    width:100
    height:100
    // General-purpose elements for the test:
    Plugin { id: testPlugin; name: "qmlgeo.test.plugin"; allowExperimental: true }


    property var coordinateList: []
    property int coordinateCount: 0
    property int animationDuration: 100

    Map {id: map
         plugin: testPlugin
         width: 100
         height: 100

         Behavior on center {
             id: centerBehavior
             enabled: false
             CoordinateAnimation {
                 id: coordinateAnimation
                 duration: animationDuration
             }
         }

         onCenterChanged: {
             if (!coordinateList) {
                 coordinateList = []
             }

             coordinateList[coordinateCount] = {'latitude': center.latitude, 'longitude': center.longitude}
             coordinateCount++
         }
    }

    function toMercator(coord) {
        var pi = Math.PI
        var lon = coord.longitude / 360.0 + 0.5;

        var lat = coord.latitude;
        lat = 0.5 - (Math.log(Math.tan((pi / 4.0) + (pi / 2.0) * lat / 180.0)) / pi) / 2.0;
        lat = Math.max(0.0, lat);
        lat = Math.min(1.0, lat);

        return {'latitude': lat, 'longitude': lon};
    }

    TestCase {
        when: windowShown && map.mapReady
        name: "CoordinateAnimation"

        function test_coordinate_animation() {

            coordinateList = []
            coordinateCount = 0

            var from = {'latitude': 58.0, 'longitude': 12.0}
            var to = {'latitude': 62.0, 'longitude': 24.0}


            var fromMerc = toMercator(from)
            var toMerc = toMercator(to)

            var delta = (toMerc.latitude - fromMerc.latitude) / (toMerc.longitude - fromMerc.longitude)

            // Set from coordinate with animation disabled.
            map.center = QtPositioning.coordinate(from.latitude, from.longitude)

            // Expect only one update
            compare(coordinateList.length, 1)

            // Set to coordinate with animation enabled
            centerBehavior.enabled = true
            map.center = QtPositioning.coordinate(to.latitude, to.longitude)
            wait(animationDuration)
            tryCompare(coordinateAnimation,"running",false)

            //check correct start position
            compare(coordinateList[0].latitude, from.latitude)
            compare(coordinateList[0].longitude, from.longitude)

            //check correct end position
            compare(coordinateList[coordinateList.length - 1].latitude, to.latitude)
            compare(coordinateList[coordinateList.length - 1].longitude, to.longitude)

            var i
            var lastLatitude
            for (i in coordinateList) {
                var coordinate = coordinateList[i]
                var mercCoordinate = toMercator(coordinate)

                //check that coordinates from the animation is along a straight line between from and to
                var estimatedLatitude = fromMerc.latitude + (mercCoordinate.longitude - fromMerc.longitude) * delta
                verify(mercCoordinate.latitude - estimatedLatitude < 0.00000000001);

                //check that each step has moved in the right direction
                if (lastLatitude) {
                    verify(coordinate.latitude > lastLatitude)
                }
                lastLatitude = coordinate.latitude
            }
        }
    }
}
