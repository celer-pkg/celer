// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioningQuick/private/qdeclarativegeolocation_p.h>

QT_USE_NAMESPACE

class tst_DeclarativeGeoLocation : public QObject
{
    Q_OBJECT

private slots:
    void locationProperty();
    void addressBinding();
    void coordinateBinding();
    void shapeBinding();
    void attributesBinding();
};

void tst_DeclarativeGeoLocation::locationProperty()
{
    // This test calls setLocation() with different preconditions, providing
    // coverage for different branches of the setLocation() method.

    QGeoAddress addr;
    addr.setCountryCode("DEU");
    addr.setCountry("Germany");
    addr.setCity("Berlin");
    addr.setStreet("Erich-Thilo-Str");
    addr.setStreetNumber("10");
    addr.setPostalCode("12489");

    const QGeoCoordinate c(52.43, 13.53);

    const QVariantMap attrs { { "string_proprty", "value" }, { "int_property", 5 } };

    QGeoLocation loc1;
    loc1.setAddress(addr);
    loc1.setCoordinate(c);
    loc1.setBoundingShape(QGeoCircle(c, 100));
    loc1.setExtendedAttributes(attrs);

    QDeclarativeGeoLocation location;
    qsizetype addrChangedCount = 0;
    qsizetype coordChangedCount = 0;
    qsizetype shapeChangedCount = 0;
    qsizetype attrChangedCount = 0;

    auto addrChangedHandler = location.bindableAddress().onValueChanged(
            [&addrChangedCount]() { ++addrChangedCount; });
    Q_UNUSED(addrChangedHandler)
    auto coordChangedHandler = location.bindableCoordinate().onValueChanged(
            [&coordChangedCount]() { ++coordChangedCount; });
    Q_UNUSED(coordChangedHandler)
    auto shapeChangedHandler = location.bindableBoundingShape().onValueChanged(
            [&shapeChangedCount]() { ++shapeChangedCount; });
    Q_UNUSED(shapeChangedHandler)
    auto attrChangedHandler = location.bindableExtendedAttributes().onValueChanged(
            [&attrChangedCount]() { ++attrChangedCount; });
    Q_UNUSED(attrChangedHandler)

    // By default an empty m_address is created in the default constructor.
    // So m_address contains a valid pointer, m_address->parent() == this.
    location.setLocation(loc1);
    QCOMPARE(addrChangedCount, 0); // the pointer didn't change
    QCOMPARE(coordChangedCount, 1);
    QCOMPARE(shapeChangedCount, 1);
    QCOMPARE(attrChangedCount, 1);
    QCOMPARE(location.location(), loc1);
    QCOMPARE(location.address()->parent(), &location);

    // m_address contains a nullptr
    location.setAddress(nullptr);
    QVERIFY(!location.address());
    addrChangedCount = 0;
    coordChangedCount = 0;
    shapeChangedCount = 0;
    attrChangedCount = 0;

    location.setLocation(loc1);
    QCOMPARE(addrChangedCount, 1); // only the pointer has changed
    QCOMPARE(coordChangedCount, 0);
    QCOMPARE(shapeChangedCount, 0);
    QCOMPARE(attrChangedCount, 0);
    QCOMPARE(location.location(), loc1);
    QCOMPARE(location.address()->parent(), &location);

    // m_address contains a valid pointer, m_address->parent() != this
    QGeoAddress addr1;
    addr1.setCountryCode("USA");
    addr1.setCountry("United States");
    addr1.setPostalCode("8900");
    addr1.setState("Oregon");
    addr1.setCity("Springfield");
    addr1.setDistrict("Pressboard Estates");
    addr1.setStreet("Evergreen Tce");
    addr1.setStreetNumber("742");

    QDeclarativeGeoAddress geoAddr(addr1);

    location.setAddress(&geoAddr);
    QVERIFY(location.address());
    QCOMPARE(location.address()->parent(), nullptr);
    addrChangedCount = 0;

    location.setLocation(loc1);
    QCOMPARE(addrChangedCount, 1); // only the pointer has changed
    QCOMPARE(coordChangedCount, 0);
    QCOMPARE(shapeChangedCount, 0);
    QCOMPARE(attrChangedCount, 0);
    QCOMPARE(location.location(), loc1);
    QCOMPARE(location.address()->parent(), &location);
}

void tst_DeclarativeGeoLocation::addressBinding()
{
    QDeclarativeGeoLocation location;
    QDeclarativeGeoAddress addr1;
    QDeclarativeGeoAddress addr2;
    QTestPrivate::testReadWritePropertyBasics<QDeclarativeGeoLocation, QDeclarativeGeoAddress *>(
            location, &addr1, &addr2, "address");
}

void tst_DeclarativeGeoLocation::coordinateBinding()
{
    QDeclarativeGeoLocation location;
    const QGeoCoordinate c1(2.0, 1.0);
    const QGeoCoordinate c2(1.0, 2.0);
    QTestPrivate::testReadWritePropertyBasics<QDeclarativeGeoLocation, QGeoCoordinate>(
            location, c1, c2, "coordinate");
}

void tst_DeclarativeGeoLocation::shapeBinding()
{
    QDeclarativeGeoLocation location;
    const QGeoCircle circle(QGeoCoordinate(2.0, 1.0), 10);
    const QGeoRectangle rectangle(QGeoCoordinate(2.0, 1.0), QGeoCoordinate(1.0, 2.0));
    QTestPrivate::testReadWritePropertyBasics<QDeclarativeGeoLocation, QGeoShape>(
            location, circle, rectangle, "boundingShape");
}

void tst_DeclarativeGeoLocation::attributesBinding()
{
    QDeclarativeGeoLocation location;
    const QVariantMap m1 { { "string_proprty", "value" }, { "int_property", 5 } };
    const QVariantMap m2 { { "int_property", 10 }, { "double_property", 15.5 } };
    QTestPrivate::testReadWritePropertyBasics<QDeclarativeGeoLocation, QVariantMap>(
            location, m1, m2, "extendedAttributes");
}

QTEST_APPLESS_MAIN(tst_DeclarativeGeoLocation)

#include "tst_qdeclarativegeolocation.moc"
