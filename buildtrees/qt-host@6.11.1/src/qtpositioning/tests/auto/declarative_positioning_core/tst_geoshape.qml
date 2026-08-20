// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtTest 1.0
import QtPositioning 5.2
import TestFactory 1.0

Item {
    id: testCase

    property var coordinate1: QtPositioning.coordinate(1, 1)
    property var coordinate2: QtPositioning.coordinate(2, 2)
    property var coordinate3: QtPositioning.coordinate(80, 80)

    property var emptyCircle: QtPositioning.circle()
    property var circle1: QtPositioning.circle(coordinate1, 200000)

    SignalSpy { id: circleChangedSpy; target: testCase; signalName: "emptyCircleChanged" }

    TestCase {
        name: "Bounding circle"
        function test_circle_defaults_and_setters() {
            circleChangedSpy.clear();
            compare (emptyCircle.radius, -1)
            compare (circle1.radius, 200000)

            emptyCircle.radius = 200
            compare(emptyCircle.radius, 200)

            emptyCircle.center = coordinate1
            compare(emptyCircle.center, coordinate1)

            emptyCircle.center = coordinate2
            compare(emptyCircle.center, coordinate2)

            emptyCircle = QtPositioning.circle(coordinate1, 200000)
            compare(emptyCircle.center, coordinate1)
            compare(emptyCircle.radius, 200000)
            // signal is triggered only when we update the whole object
            compare(circleChangedSpy.count, 1)

            compare(emptyCircle.contains(coordinate1), true);
            compare(emptyCircle.contains(coordinate2), true);
            compare(emptyCircle.contains(coordinate3), false);
        }
    }

    property var trace1 : [ QtPositioning.coordinate(43.773175, 11.255386),
                            QtPositioning.coordinate(43.773546 , 11.255372) ]
    property var trace2 : [ QtPositioning.coordinate(43.773175, 11.255386),
                            QtPositioning.coordinate(43.773546 , 11.255372),
                            QtPositioning.coordinate(43.77453 , 11.255734) ]


    // coordinate unit square
    property var bl: QtPositioning.coordinate(0, 0)
    property var tl: QtPositioning.coordinate(1, 0)
    property var tr: QtPositioning.coordinate(1, 1)
    property var br: QtPositioning.coordinate(0, 1)
    property var ntr: QtPositioning.coordinate(3, 3)

    property var invalid: QtPositioning.coordinate(100, 190)
    property var inside: QtPositioning.coordinate(0.5, 0.5)
    property var outside: QtPositioning.coordinate(2, 2)

    property var box: QtPositioning.rectangle(tl, br)

    property var coordinates: [bl, tl, tr, br]
    property var coordinates2: [bl, tl, tr, br, ntr]
    property var coordinates3: [tr]
    property var coordinates4: [invalid]
    property var coordinates5: []

    property var listBox: QtPositioning.rectangle(coordinates)
    property var listBox2: QtPositioning.rectangle(coordinates2)
    property var listBox3: QtPositioning.rectangle(coordinates3)
    property var listBox4: QtPositioning.rectangle(coordinates4)
    property var listBox5: QtPositioning.rectangle(coordinates5)

    property var widthBox: QtPositioning.rectangle(inside, 1, 1);

    // C++ auto test exists for basics of bounding box, testing here
    // only added functionality
    TestCase {
        name: "Bounding box"
        function test_box_defaults_and_setters() {
            compare (box.bottomRight.longitude, br.longitude) // sanity
            compare (box.contains(bl), true)
            compare (box.contains(inside), true)
            compare (box.contains(outside), false)
            box.topRight = ntr
            compare (box.contains(outside), true)

            compare (listBox.isValid, true)
            compare (listBox.contains(outside), false)
            compare (listBox2.contains(outside), true)
            compare (listBox3.isValid, true)
            compare (listBox3.isEmpty, true)
            compare (listBox4.isValid, false)
            compare (listBox5.isValid, false)

            compare (widthBox.contains(inside), true)
            compare (widthBox.contains(outside), false)
        }
    }

    TestCase {
        name: "Shape"

        function test_shape_comparison_data() {
            return [
                { tag: "invalid shape", shape1: QtPositioning.shape(), shape2: QtPositioning.shape(), result: true },
                { tag: "box equal", shape1: box, shape2: QtPositioning.rectangle(tl, br), result: true },
                { tag: "box not equal", shape1: box, shape2: QtPositioning.rectangle([inside, outside]), result: false },
                { tag: "box invalid shape", rect1: box, shape2: QtPositioning.shape(), result: false },
                { tag: "invalid rectangle", shape1: QtPositioning.rectangle(), shape2: QtPositioning.rectangle(), result: true },
                { tag: "invalid rectangle2", shape1: QtPositioning.rectangle(), shape2: QtPositioning.shape(), result: false },
                { tag: "circle1 equal", shape1: circle1, shape2: QtPositioning.circle(coordinate1, 200000), result: true },
                { tag: "circle1 not equal", shape1: circle1, shape2: QtPositioning.circle(coordinate2, 2000), result: false },
                { tag: "circle1 invalid shape", shape1: circle1, shape2: QtPositioning.shape(), result: false },
                { tag: "invalid circle", shape1: QtPositioning.circle(), shape2: QtPositioning.circle(), result: true },
                { tag: "invalid circle2", shape1: QtPositioning.circle(), shape2: QtPositioning.shape(), result: false }
            ]
        }

        function test_shape_comparison(data) {
            compare(data.shape1 === data.shape2, data.result)
            compare(data.shape1 !== data.shape2, !data.result)
            compare(data.shape1 == data.shape2, data.result)
            compare(data.shape1 != data.shape2, !data.result)
        }
    }

    TestCase {
        name: "Conversions"

        function test_shape_circle_conversions() {
            var circle = QtPositioning.shapeToCircle(QtPositioning.shape())
            verify(!circle.isValid)
            circle = QtPositioning.shapeToCircle(QtPositioning.circle())
            verify(!circle.isValid)
            circle = QtPositioning.shapeToCircle(QtPositioning.circle(tl, 10000))
            verify(circle.isValid)
            compare(circle.center, tl)
            compare(circle.radius, 10000)
            circle = QtPositioning.shapeToCircle(QtPositioning.rectangle())
            verify(!circle.isValid)
            circle = QtPositioning.shapeToCircle(QtPositioning.rectangle(tl, br))
            verify(!circle.isValid)
            circle = QtPositioning.shapeToCircle(listBox)
            verify(!circle.isValid)
        }

        function test_shape_rectangle_conversions() {
            var rectangle = QtPositioning.shapeToRectangle(QtPositioning.shape())
            verify(!rectangle.isValid)
            rectangle = QtPositioning.shapeToRectangle(QtPositioning.circle())
            verify(!rectangle.isValid)
            rectangle = QtPositioning.shapeToRectangle(QtPositioning.circle(tl, 10000))
            verify(!rectangle.isValid)
            rectangle = QtPositioning.shapeToRectangle(QtPositioning.rectangle())
            verify(!rectangle.isValid)
            rectangle = QtPositioning.shapeToRectangle(QtPositioning.rectangle(tl, br))
            verify(rectangle.isValid)
            compare(rectangle.topLeft, tl)
            compare(rectangle.bottomRight, br)
            rectangle = QtPositioning.shapeToRectangle(listBox)
            verify(rectangle.isValid)
        }

        function test_shape_path_conversions() {
            var path = QtPositioning.shapeToPath(QtPositioning.shape())
            verify(!path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.circle())
            verify(!path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.circle(tl, 10000))
            verify(!path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.rectangle())
            verify(!path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.rectangle(tl, br))
            verify(!path.isValid)

            path = QtPositioning.shapeToPath(QtPositioning.path())
            verify(!path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.path(trace1, 1))
            verify(path.isValid)
            path = QtPositioning.shapeToPath(QtPositioning.path(trace2, 2))
            verify(path.isValid)
            verify(path !== QtPositioning.shapeToPath(QtPositioning.path(trace1, 1)))
            compare(path, QtPositioning.shapeToPath(QtPositioning.path(trace2, 2)))
        }

        function test_shape_polygon_conversions() {
            var polygon = QtPositioning.shapeToPolygon(QtPositioning.shape())
            verify(!polygon.isValid)
            polygon = QtPositioning.shapeToPolygon(QtPositioning.circle())
            verify(!polygon.isValid)
            polygon = QtPositioning.shapeToPolygon(QtPositioning.circle(tl, 10000))
            verify(polygon.isValid) // fixed, polygon copy constructor can now initialize from a circle.
            polygon = QtPositioning.shapeToPolygon(QtPositioning.rectangle())
            verify(!polygon.isValid)
            polygon = QtPositioning.shapeToPolygon(QtPositioning.rectangle(tl, br))
            verify(polygon.isValid) // fixed, polygon copy constructor can now initialize from a rectangle.

            polygon = QtPositioning.shapeToPolygon(QtPositioning.polygon())
            verify(!polygon.isValid)
            polygon = QtPositioning.shapeToPolygon(QtPositioning.polygon(trace1))
            verify(!polygon.isValid) // polygon needs 3 coords at least
            polygon = QtPositioning.shapeToPolygon(QtPositioning.polygon(trace2))
            verify(polygon.isValid)
            verify(polygon !== QtPositioning.shapeToPolygon(QtPositioning.polygon(trace1)))
            compare(polygon, QtPositioning.shapeToPolygon(QtPositioning.polygon(trace2)))
        }
    }

    TestCase {
        name: "Properties"
        Properties {
            id: properties
            coordinate: QtPositioning.coordinate(12.5, 23.6)
            coordinates: [
                { latitude: 1.1, longitude: 2.2 },
                { latitude: 3.3, longitude: 4.4, altitude: 5.5 }
            ]
            rectangle: box
            region: [
                {
                    topLeft: { latitude: 3.3, longitude: 2.2 },
                    bottomRight: { latitude: 1.1, longitude: 4.4 }
                },
                {
                    bottomLeft: { latitude: 1.1, longitude: 2.2 },
                    topRight: { latitude: 3.3, longitude: 4.4 }
                },
                {
                    center: { latitude: 5.5, longitude: 6.6 },
                    width: 0.5,
                    height: 0.1
                }
            ]
        }

        function test_initialization() {
            compare(properties.coordinate, QtPositioning.coordinate(12.5, 23.6))
            compare(properties.coordinates.length, 2)
            compare(properties.coordinates[0], QtPositioning.coordinate(1.1, 2.2))
            compare(properties.coordinates[1], QtPositioning.coordinate(3.3, 4.4, 5.5))

            compare(properties.rectangle, box)
            compare(properties.region.length, 3)
            var r = QtPositioning.rectangle(QtPositioning.coordinate(3.3, 2.2),
                                            QtPositioning.coordinate(1.1, 4.4))
            compare(properties.region[0], r)
            compare(properties.region[1], r)
            compare(properties.region[2], QtPositioning.rectangle(QtPositioning.coordinate(5.5, 6.6), 0.5, 0.1))
        }
    }

    TestCase {
        name: "GeoPath path"
        function test_qgeopath_path_operations() {
            var geopath = QtPositioning.path()

            geopath.path = trace2
            compare(geopath.path.length, trace2.length)

            geopath.path = trace2
            geopath.path[0].longitude = 11.0
            compare(geopath.path.length, trace2.length)
            compare(geopath.coordinateAt(0).latitude, trace2[0].latitude)
            expectFail("", "Longitude comparison fails")
            compare(geopath.coordinateAt(0).longitude, 11)
        }
    }

    TestCase {
        name: "GeoPolygon path"
        function test_qgeopolygon_path_operations() {
            var geopolygon = QtPositioning.polygon()

            geopolygon.perimeter = trace2
            compare(geopolygon.perimeter.length, trace2.length)

            geopolygon.perimeter = trace2
            compare(geopolygon.perimeter.length, trace2.length)
            compare(geopolygon.coordinateAt(0).latitude, trace2[0].latitude)
            compare(geopolygon.coordinateAt(0).longitude, trace2[0].longitude)
        }
    }

    Factory {
        id: factory
    }

    TestCase {
        name: "GeoShape factory"
        function test_geoshape_factory_construction() {
            // Both createShape() methods return QGeoShape. We check that
            // invokable methods and properties are called for correct
            // objects
            var c0 = QtPositioning.coordinate(1.0, 1.0)
            var c1 = QtPositioning.coordinate(1.0001, 1.0001)
            var c2 = QtPositioning.coordinate(0.5, 0.5)
            var circle = factory.createShape(c0, 100.0)
            verify(circle.contains(c1))
            verify(!circle.contains(c2))
            compare(circle.center, c0)

            var rectangle = factory.createShape(QtPositioning.coordinate(1.0, 0.0), QtPositioning.coordinate(0.0, 1.0))
            verify(rectangle.contains(c2))
            verify(!rectangle.contains(c1))
            compare(rectangle.center, c2)

            verify(rectangle.center !== circle.center)
        }
    }
}
