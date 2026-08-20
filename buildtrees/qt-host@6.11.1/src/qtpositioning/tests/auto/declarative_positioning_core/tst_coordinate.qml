// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtTest 1.0
import QtPositioning 6.2

Item {
    id: item

    property var empty: QtPositioning.coordinate()
    property var base: QtPositioning.coordinate(1.0, 1.0, 5.0)
    property var zero: QtPositioning.coordinate(0, 0)
    property var plusone: QtPositioning.coordinate(0, 1)
    property var minusone: QtPositioning.coordinate(0, -1)
    property var north: QtPositioning.coordinate(3, 0)

    SignalSpy { id: coordSpy; target: item; signalName: "baseChanged" }

    property var inside: QtPositioning.coordinate(0.5, 0.5)
    property var outside: QtPositioning.coordinate(2, 2)
    property var tl: QtPositioning.coordinate(1, 0)
    property var br: QtPositioning.coordinate(0, 1)
    property var box: QtPositioning.rectangle(tl, br)


    Address {
        id: validTestAddress
        street: "Brandl St"
        streetNumber: "53"
        city: "Eight Mile Plains"
        country: "Australia"
        countryCode: "AUS"
    }

    Location {
        id: testLocation
        coordinate: inside
        boundingShape: box
        address: validTestAddress
    }

    Location {
        id: invalidLocation
    }


    Item {
        id: coordinateItem
        property variant coordinate
        property int animationDuration: 100
        property var coordinateList: []
        property int coordinateCount: 0

        CoordinateAnimation {
            id: coordinateAnimation
            target: coordinateItem
            property: "coordinate"
            duration: coordinateItem.animationDuration
        }
        onCoordinateChanged: {
            if (!coordinateList) {
                coordinateList = []
            }
            coordinateList[coordinateCount] = QtPositioning.coordinate(coordinate.latitude,coordinate.longitude)
            coordinateCount++
        }

        SignalSpy { id: coordinateAnimationStartSpy; target: coordinateAnimation; signalName: "started" }
        SignalSpy { id: coordinateAnimationStopSpy; target: coordinateAnimation; signalName: "stopped" }
        SignalSpy { id: coordinateAnimationDirectionSpy; target: coordinateAnimation; signalName: "directionChanged" }
    }

    TestCase {
        name: "GeoLocation"

        function test_Location_complete()
        {
            compare (testLocation.coordinate.longitude, inside.longitude)
            compare (testLocation.coordinate.latitude, inside.latitude)

            compare (testLocation.boundingShape.contains(inside), true)
            compare (testLocation.boundingShape.contains(outside), false)
            var shapeRectangle = testLocation.boundingShape.boundingGeoRectangle()
            compare (shapeRectangle.bottomRight.longitude, br.longitude)
            compare (shapeRectangle.bottomRight.latitude, br.latitude)
            compare (shapeRectangle.topLeft.longitude, tl.longitude)
            compare (shapeRectangle.topLeft.latitude, tl.latitude)

            compare (testLocation.address.country, "Australia")
            compare (testLocation.address.countryCode, "AUS")
            compare (testLocation.address.city, "Eight Mile Plains")
            compare (testLocation.address.street, "Brandl St")
            compare (testLocation.address.streetNumber, "53")
        }

        function test_Location_invalid()
        {
            compare(invalidLocation.coordinate.isValid, false)
            compare(invalidLocation.boundingShape.isEmpty, true)
            compare(invalidLocation.boundingShape.isValid, false)
            compare(invalidLocation.address.city, "")
        }
    }

    TestCase {
        name: "Coordinate"

        function test_validity()
        {
            compare(empty.isValid, false)

            empty.longitude = 0.0;
            empty.latitude = 0.0;

            compare(empty.isValid, true)
        }

        function test_accessors()
        {
            compare(base.longitude, 1.0)
            compare(base.latitude, 1.0)
            compare(base.altitude, 5.0)

            coordSpy.clear()

            base.longitude = 2.0
            base.latitude = 3.0
            base.altitude = 6.0

            compare(base.longitude, 2.0)
            compare(base.latitude, 3.0)
            compare(base.altitude, 6.0)
            // changing individual properties does *not* trigger
            // change notification
            compare(coordSpy.count, 0)

            // updating the whole object *does* trigger change notification
            base = QtPositioning.coordinate(3.0, 4.0, 5.0)
            compare(base.latitude, 3.0)
            compare(base.longitude, 4.0)
            compare(base.altitude, 5.0)
            compare(coordSpy.count, 1)
        }

        function test_comparison_data()
        {
            return [
                { tag: "empty", coord1: empty, coord2: QtPositioning.coordinate(), result: true },
                { tag: "zero", coord1: zero, coord2: QtPositioning.coordinate(0, 0), result: true },
                { tag: "plusone", coord1: plusone, coord2: QtPositioning.coordinate(0, 1), result: true },
                { tag: "minusone", coord1: minusone, coord2: QtPositioning.coordinate(0, -1), result: true },
                { tag: "north", coord1: north, coord2: QtPositioning.coordinate(3, 0), result: true },
                { tag: "lat,long.alt", coord1: QtPositioning.coordinate(1.1, 2.2, 3.3), coord2: QtPositioning.coordinate(1.1, 2.2, 3.3), result: true },
                { tag: "not equal1", coord1: plusone, coord2: minusone, result: false },
                { tag: "not equal2", coord1: plusone, coord2: north, result: false }
            ]
        }

        function test_comparison(data)
        {
            compare(data.coord1 === data.coord2, data.result)
            compare(data.coord1 !== data.coord2, !data.result)
            compare(data.coord1 == data.coord2, data.result)
            compare(data.coord1 != data.coord2, !data.result)
        }

        function test_distance()
        {
            compare(zero.distanceTo(plusone), zero.distanceTo(minusone))
            compare(2*plusone.distanceTo(zero), plusone.distanceTo(minusone))
            compare(zero.distanceTo(plusone) > 0, true)
        }

        function test_azimuth()
        {
            compare(zero.azimuthTo(north), 0)
            compare(zero.azimuthTo(plusone), 90)
            compare(zero.azimuthTo(minusone), 270)
            compare(minusone.azimuthTo(plusone), 360 - plusone.azimuthTo(minusone))
        }

        function test_atDistanceAndAzimuth()
        {
            // 112km is approximately one degree of arc

            var coord_0d = zero.atDistanceAndAzimuth(112000, 0)
            compare(coord_0d.latitude > 0.95, true)
            compare(coord_0d.latitude < 1.05, true)
            compare(coord_0d.longitude < 0.05, true)
            compare(coord_0d.longitude > -0.05, true)
            compare(zero.distanceTo(coord_0d), 112000)
            compare(zero.azimuthTo(coord_0d), 0)

            var coord_90d = zero.atDistanceAndAzimuth(112000, 90)
            compare(coord_90d.longitude > 0.95, true)
            compare(coord_90d.longitude < 1.05, true)
            compare(coord_90d.latitude < 0.05, true)
            compare(coord_90d.latitude > -0.05, true)
            compare(zero.distanceTo(coord_90d), 112000)
            compare(zero.azimuthTo(coord_90d), 90)

            var coord_30d = zero.atDistanceAndAzimuth(20000, 30)
            compare(coord_30d.longitude > 0, true)
            compare(coord_30d.latitude > 0, true)
            compare(zero.distanceTo(coord_30d), 20000)
            compare(zero.azimuthTo(coord_30d), 30)

            var coord_30d2 = coord_30d.atDistanceAndAzimuth(200, 30)
            compare(zero.distanceTo(coord_30d2), 20200)
        }
    }

    TestCase {
        name: "CoordinateAnimation"

        function init()
        {
            coordinateAnimation.stop()
            coordinateAnimationStartSpy.clear()
            coordinateAnimationStopSpy.clear()
            coordinateAnimationDirectionSpy.clear()
            coordinateAnimation.from = QtPositioning.coordinate(50,50)
            coordinateAnimation.to = QtPositioning.coordinate(50,50)
            coordinateAnimation.direction = CoordinateAnimation.Shortest
            coordinateItem.coordinate = QtPositioning.coordinate(50,50)
            coordinateItem.coordinateList = []
            coordinateItem.coordinateCount = 0
        }

        function initTestCase()
        {
            compare(coordinateAnimation.direction, CoordinateAnimation.Shortest)
            compare(coordinateAnimationDirectionSpy.count,0)
            coordinateAnimation.direction = CoordinateAnimation.Shortest
            compare(coordinateAnimationDirectionSpy.count,0)
            coordinateAnimation.direction = CoordinateAnimation.West
            compare(coordinateAnimationDirectionSpy.count,1)
            coordinateAnimation.direction = CoordinateAnimation.East
            compare(coordinateAnimationDirectionSpy.count,2)
        }

        function toMercator(coord)
        {
            var p = QtPositioning.coordToMercator(coord)
            var lat = p.y
            var lon = p.x
            return {'latitude': lat, 'longitude': lon};
        }

        function coordinate_animation(from, to, movingEast)
        {
            var fromMerc = toMercator(from)
            var toMerc = toMercator(to)
            var delta = (toMerc.latitude - fromMerc.latitude) / (toMerc.longitude - fromMerc.longitude)

            compare(coordinateItem.coordinateList.length, 0);
            coordinateAnimation.from =  from
            coordinateAnimation.to = to
            coordinateAnimation.start()
            tryCompare(coordinateAnimationStartSpy,"count",1)
            tryCompare(coordinateAnimationStopSpy,"count",1)

            //check correct start position
            verify(coordinateItem.coordinateList.length != 0)
            compare(coordinateItem.coordinateList[0], from)
            //check correct end position
            compare(coordinateItem.coordinateList[coordinateItem.coordinateList.length - 1],to)

            var i
            var lastLongitude
            for (i in coordinateItem.coordinateList) {
                var coordinate = coordinateItem.coordinateList[i]
                var mercCoordinate = toMercator(coordinate)

                //check that coordinates from the animation is along a straight line between from and to
                var estimatedLatitude = fromMerc.latitude + (mercCoordinate.longitude - fromMerc.longitude) * delta
                verify(mercCoordinate.latitude - estimatedLatitude < 0.00000000001);

                //check that each step has moved in the right direction

                if (lastLongitude) {
                    var errorMessage = "movingEast: " + movingEast + "; From: " + from + "; To: " + to + "; i: " + i + "; crdList: " + coordinateItem.coordinateList
                    if (movingEast) {
                        if (coordinate.longitude < 0 && lastLongitude > 0)
                            verify(coordinate.longitude + 360 > lastLongitude, errorMessage)
                        else
                            verify(coordinate.longitude > lastLongitude, errorMessage)
                    } else {
                        if (coordinate.longitude > 0 && lastLongitude < 0)
                            verify(coordinate.longitude < lastLongitude + 360, errorMessage)
                        else
                            verify(coordinate.longitude < lastLongitude, errorMessage)
                    }
                }
                lastLongitude = coordinate.longitude
            }
        }

        function test_default_coordinate_animation(data)
        {
            //shortest
            coordinate_animation(data.from, data.to, data.east)
        }

        function test_default_coordinate_animation_data()
        {
            return [
                {
                    from: QtPositioning.coordinate(58.0, 12.0),
                    to: QtPositioning.coordinate(62.0, 24.0),
                    east: true
                },
                {
                    from: QtPositioning.coordinate(58.0, 24.0),
                    to: QtPositioning.coordinate(42.0, 12.0),
                    east: false
                },
                // cross 0
                {
                    from: QtPositioning.coordinate(30, 10),
                    to: QtPositioning.coordinate(20, -10),
                    east: false
                },
                {
                    from: QtPositioning.coordinate(30, -20),
                    to: QtPositioning.coordinate(20, 10),
                    east: true
                },
                // cross 180
                {
                    from: QtPositioning.coordinate(30, 170),
                    to: QtPositioning.coordinate(30, -170),
                    east: true
                },
                {
                    from: QtPositioning.coordinate(30, -170),
                    to: QtPositioning.coordinate(30, 170),
                    east: false
                },
            ]
        }

        function test_east_direction_coordinate_animation(data)
        {
            coordinateAnimation.direction = CoordinateAnimation.East
            coordinate_animation(data.from,
                                 data.to,
                                 true)
        }

        function test_east_direction_coordinate_animation_data()
        {
            return [
                { from: QtPositioning.coordinate(58.0,24.0), to: QtPositioning.coordinate(58.0,12.0) },
                { from: QtPositioning.coordinate(58.0,12.0), to: QtPositioning.coordinate(58.0,24.0) },
            ]
        }


        function test_west_direction_coordinate_animation(data)
        {
            coordinateAnimation.direction = CoordinateAnimation.West
            coordinate_animation(data.from,
                                 data.to,
                                 false)
        }

        function test_west_direction_coordinate_animation_data()
        {
            return [
                { from: QtPositioning.coordinate(58.0,24.0),to: QtPositioning.coordinate(58.0,12.0) },
                { from: QtPositioning.coordinate(58.0,12.0),to: QtPositioning.coordinate(58.0,24.0) },
            ]
        }


    }
}
