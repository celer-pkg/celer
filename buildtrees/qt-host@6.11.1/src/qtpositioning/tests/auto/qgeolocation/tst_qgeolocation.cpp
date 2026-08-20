// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_qgeolocation.h"
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoPolygon>

QT_USE_NAMESPACE

tst_QGeoLocation::tst_QGeoLocation()
{
}

void tst_QGeoLocation::initTestCase()
{

}

void tst_QGeoLocation::cleanupTestCase()
{

}

void tst_QGeoLocation::init()
{
}

void tst_QGeoLocation::cleanup()
{
}

void tst_QGeoLocation::constructor()
{
    QCOMPARE(m_location.address(), m_address);
    QCOMPARE(m_location.coordinate(), m_coordinate);
    QCOMPARE(m_location.boundingShape(), m_viewport);
    QCOMPARE(m_location.extendedAttributes(), m_extendedAttributes);
}

void tst_QGeoLocation::copy_constructor()
{
    auto qgeolocationcopy = std::make_unique<QGeoLocation>(m_location);
    QCOMPARE(m_location, *qgeolocationcopy);
}

void tst_QGeoLocation::move_constructor()
{
    QGeoAddress address;
    address.setCity("Berlin");
    address.setCountry("Germany");
    address.setCountryCode("DEU");
    address.setDistrict("Adlershof");
    address.setPostalCode("12489");
    address.setStreet("Erich-Thilo-Strasse");

    QGeoLocation location;
    location.setAddress(address);
    location.setCoordinate(QGeoCoordinate(1.3, 2.4, 3.1));
    location.setBoundingShape(QGeoCircle(QGeoCoordinate(1.3, 2.4), 100));

    QGeoLocation locationCopy = location;
    QCOMPARE(std::move(location), locationCopy);
}

void tst_QGeoLocation::move_assignment()
{
    QGeoAddress address;
    address.setCity("Berlin");
    address.setCountry("Germany");
    address.setCountryCode("DEU");
    address.setDistrict("Adlershof");
    address.setPostalCode("12489");
    address.setStreet("Erich-Thilo-Strasse");

    QGeoLocation location;
    location.setAddress(address);
    location.setCoordinate(QGeoCoordinate(1.3, 2.4, 3.1));
    location.setBoundingShape(QGeoCircle(QGeoCoordinate(1.3, 2.4), 100));

    QGeoLocation locationCopy = location;

    QGeoLocation otherLocation;
    otherLocation = std::move(location);
    QCOMPARE(otherLocation, locationCopy);

    // Check that (move)assignment to a moved-from object is fine
    location = std::move(locationCopy);
    QCOMPARE(location, otherLocation);
}

void tst_QGeoLocation::destructor()
{
    QGeoLocation *qgeolocationcopy;

    qgeolocationcopy = new QGeoLocation();
    delete qgeolocationcopy;

    qgeolocationcopy = new QGeoLocation(m_location);
    delete qgeolocationcopy;
}

void tst_QGeoLocation::address()
{
    m_address.setCity("Berlin");
    m_address.setCountry("Germany");
    m_address.setCountryCode("DEU");
    m_address.setDistrict("Mitte");
    m_address.setPostalCode("10115");
    m_address.setStreet("Invalidenstrasse");

    m_location.setAddress(m_address);

    QCOMPARE(m_location.address(),m_address);

    m_address.setPostalCode("10125");
    QVERIFY(m_location.address() != m_address);
}

void tst_QGeoLocation::coordinate()
{
    m_coordinate.setLatitude(13.3851);
    m_coordinate.setLongitude(52.5312);
    m_coordinate.setAltitude(134.23);

    m_location.setCoordinate(m_coordinate);

    QCOMPARE(m_location.coordinate(), m_coordinate);

    m_coordinate.setAltitude(0);
    QVERIFY(m_location.coordinate() != m_coordinate);
}

void tst_QGeoLocation::viewport()
{
    m_coordinate.setLatitude(13.3851);
    m_coordinate.setLongitude(52.5312);

    // rectangle bounding box
    QGeoRectangle qgeoboundingboxcopy(m_coordinate, 0.4, 0.4);
    m_location.setBoundingShape(qgeoboundingboxcopy);

    QCOMPARE(m_location.boundingShape(), qgeoboundingboxcopy);
    // test that QGeoShape::boundingGeoRectangle() matches with
    // the qgeoboundingboxcopy. It simplifies code porting
    QCOMPARE(m_location.boundingShape().boundingGeoRectangle(), qgeoboundingboxcopy);

    qgeoboundingboxcopy.setHeight(1);

    QVERIFY(m_location.boundingShape() != qgeoboundingboxcopy);

    // circle bounding box
    QGeoCircle circle(m_coordinate, 10);
    m_location.setBoundingShape(circle);

    QCOMPARE(m_location.boundingShape(), circle);

    auto point = QGeoCoordinate(0.5, 0.5);
    QVERIFY(!circle.contains(point));
    circle.extendCircle(point);

    QVERIFY(m_location.boundingShape() != circle);
    QVERIFY(!m_location.boundingShape().contains(point));

    // polygon bounding box
    const QList<QGeoCoordinate> points = {QGeoCoordinate(1.0, 1.0), QGeoCoordinate(1.0, 2.0), QGeoCoordinate(2.0, 2.0)};
    QGeoPolygon polygon(points);

    point = QGeoCoordinate(1.75, 1.25);
    QVERIFY(!polygon.contains(point));

    m_location.setBoundingShape(polygon);
    QCOMPARE(m_location.boundingShape(), polygon);

    polygon.addCoordinate(QGeoCoordinate(2.0, 1.0));
    QVERIFY(m_location.boundingShape() != polygon);
    QVERIFY(polygon.contains(point));
    QVERIFY(!m_location.boundingShape().contains(point));
}

void tst_QGeoLocation::extendedAttributes()
{
    m_extendedAttributes = QVariantMap({{ "foo" , 42 }});
    m_location.setExtendedAttributes(m_extendedAttributes);
    QCOMPARE(m_location.extendedAttributes(), m_extendedAttributes);

    m_extendedAttributes["foo"] = 41;
    QVERIFY(m_location.extendedAttributes() != m_extendedAttributes);
}

void tst_QGeoLocation::operators()
{
    QGeoAddress qgeoaddresscopy;
    qgeoaddresscopy.setCity("Berlin");
    qgeoaddresscopy.setCountry("Germany");
    qgeoaddresscopy.setCountryCode("DEU");

    QGeoCoordinate qgeocoordinatecopy (32.324 , 41.324 , 24.55);
    QVariantMap extendedAttributesCopy {{ "foo" , 42 }};

    m_address.setCity("Madrid");
    m_address.setCountry("Spain");
    m_address.setCountryCode("SPA");

    m_coordinate.setLatitude(21.3434);
    m_coordinate.setLongitude(38.43443);
    m_coordinate.setAltitude(634.21);

    m_extendedAttributes["foo"] = 43;

    m_location.setAddress(m_address);
    m_location.setCoordinate(m_coordinate);
    m_location.setExtendedAttributes(m_extendedAttributes);

    //Create a copy and see that they are the same
    QGeoLocation qgeolocationcopy(m_location);
    QVERIFY(m_location == qgeolocationcopy);
    QVERIFY(!(m_location != qgeolocationcopy));

    //Modify one and test if they are different
   qgeolocationcopy.setAddress(qgeoaddresscopy);
   QVERIFY(!(m_location == qgeolocationcopy));
   QVERIFY(m_location != qgeolocationcopy);
   qgeolocationcopy.setAddress(m_address);
   qgeolocationcopy.setCoordinate(qgeocoordinatecopy);
   QVERIFY(!(m_location == qgeolocationcopy));
   QVERIFY(m_location != qgeolocationcopy);
   qgeolocationcopy.setCoordinate(m_coordinate);
   qgeolocationcopy.setExtendedAttributes(extendedAttributesCopy);
   QVERIFY(!(m_location == qgeolocationcopy));
   QVERIFY(m_location != qgeolocationcopy);


    //delete qgeolocationcopy;
    //Asign and test that they are the same
    qgeolocationcopy = m_location;
    QVERIFY(m_location ==qgeolocationcopy);
    QVERIFY(!(m_location != qgeolocationcopy));
}

void tst_QGeoLocation::comparison()
{
    QFETCH(QString, dataField);

    QGeoLocation location;

    //set address
    QGeoAddress address;
    address.setStreet("21 jump st");
    address.setCountry("USA");
    location.setAddress(address);

    //set coordinate
    location.setCoordinate(QGeoCoordinate(5,10));

    //set viewport
    location.setBoundingShape(QGeoRectangle(QGeoCoordinate(5,5),0.4,0.4));

    QGeoLocation otherLocation(location);

    if (dataField == "no change") {
        QCOMPARE(location, otherLocation);
    } else {
        if (dataField == "address") {
            QGeoAddress otherAddress;
            otherAddress.setStreet("42 evergreen tce");
            otherAddress.setCountry("USA");
            otherLocation.setAddress(otherAddress);
        } else if (dataField == "coordinate") {
            otherLocation.setCoordinate(QGeoCoordinate(12,13));
        } else if (dataField == "viewport"){
            otherLocation.setBoundingShape(QGeoRectangle(QGeoCoordinate(1,2), 0.5,0.5));
        } else if (dataField == "extendedAttributes"){
            otherLocation.setExtendedAttributes(QVariantMap({{"foo", 44}}));
        } else {
            qFatal("Unknown data field to test");
        }

        QVERIFY(location != otherLocation);
    }
}

void tst_QGeoLocation::comparison_data()
{
    QTest::addColumn<QString> ("dataField");
    QTest::newRow("no change") << "no change";
    QTest::newRow("address") << "address";
    QTest::newRow("coordinate") << "coordinate";
    QTest::newRow("extendedAttributes") << "extendedAttributes";
}

void tst_QGeoLocation::isEmpty()
{
    QGeoAddress address;
    address.setCity(QStringLiteral("Braunschweig"));
    QVERIFY(!address.isEmpty());

    QGeoRectangle boundingBox;
    boundingBox.setTopLeft(QGeoCoordinate(1, -1));
    boundingBox.setBottomRight(QGeoCoordinate(-1, 1));
    QVERIFY(!boundingBox.isEmpty());

    QVariantMap extendedAttributes({{"foo", 11}});

    QGeoLocation location;

    QVERIFY(location.isEmpty());

    // address
    location.setAddress(address);
    QVERIFY(!location.isEmpty());
    location.setAddress(QGeoAddress());
    QVERIFY(location.isEmpty());

    // coordinate
    location.setCoordinate(QGeoCoordinate(1, 2));
    QVERIFY(!location.isEmpty());
    location.setCoordinate(QGeoCoordinate());
    QVERIFY(location.isEmpty());

    // bounding box
    location.setBoundingShape(boundingBox);
    QVERIFY(!location.isEmpty());
    location.setBoundingShape(QGeoRectangle());
    QVERIFY(location.isEmpty());

    // extended attributes
    location.setExtendedAttributes(extendedAttributes);
    QVERIFY(!location.isEmpty());
    location.setExtendedAttributes(QVariantMap());
    QVERIFY(location.isEmpty());
}

void tst_QGeoLocation::hashing()
{
    QFETCH(QGeoLocation, leftLocation);
    QFETCH(QGeoLocation, rightLocation);
    QFETCH(bool, result);

    const size_t leftHash = qHash(leftLocation);
    const size_t rightHash = qHash(rightLocation);
    QCOMPARE(leftHash == rightHash, result);
}

void tst_QGeoLocation::hashing_data()
{
    QTest::addColumn<QGeoLocation>("leftLocation");
    QTest::addColumn<QGeoLocation>("rightLocation");
    QTest::addColumn<bool>("result");

    QTest::newRow("empty") << QGeoLocation() << QGeoLocation() << true;

    QGeoAddress address;
    address.setCity("city");
    address.setPostalCode("1234");
    address.setDistrict("district");
    address.setStreet("street");
    address.setStreetNumber("12");

    QGeoLocation leftLocation;
    leftLocation.setAddress(address);
    leftLocation.setCoordinate(QGeoCoordinate(1, 1));
    leftLocation.setBoundingShape(QGeoCircle(QGeoCoordinate(1, 1), 10));

    // do not copy, so that they have different d_ptr's
    QGeoLocation rightLocation;
    rightLocation.setAddress(address);
    rightLocation.setCoordinate(QGeoCoordinate(1, 1));
    rightLocation.setBoundingShape(QGeoCircle(QGeoCoordinate(1, 1), 10));

    QTest::newRow("same locations") << leftLocation << rightLocation << true;

    QGeoLocation rightLocationCopy = rightLocation;
    rightLocationCopy.setBoundingShape(QGeoRectangle(QGeoCoordinate(2, 0), QGeoCoordinate(0, 2)));
    QTest::newRow("different shapes") << leftLocation << rightLocationCopy << false;

    rightLocationCopy = rightLocation;
    rightLocationCopy.setCoordinate(QGeoCoordinate(2, 1));
    QTest::newRow("different coordinates") << leftLocation << rightLocationCopy << false;

    rightLocationCopy = rightLocation;
    address.setState("state");
    rightLocationCopy.setAddress(address);
    QTest::newRow("different addresses") << leftLocation << rightLocationCopy << false;
}

QTEST_APPLESS_MAIN(tst_QGeoLocation);

