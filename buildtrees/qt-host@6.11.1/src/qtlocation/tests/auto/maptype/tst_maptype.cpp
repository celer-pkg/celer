// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QVariantMap>

#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QGeoMapType)

class tst_MapType : public QObject
{
    Q_OBJECT

public:
    tst_MapType();

private Q_SLOTS:
    void constructorTest();
    void comparison();
    void comparison_data();
};

tst_MapType::tst_MapType() {}

void tst_MapType::constructorTest()
{
    QGeoCameraCapabilities capabilities;
    capabilities.setMinimumZoomLevel(0.0);
    capabilities.setMaximumZoomLevel(20.0);
    capabilities.setSupportsBearing(true);
    capabilities.setSupportsTilting(true);
    capabilities.setMinimumTilt(0);
    capabilities.setMaximumTilt(60);
    capabilities.setMinimumFieldOfView(20);
    capabilities.setMaximumFieldOfView(90);
    const QByteArray pluginName = "tst_MapType";
    QGeoMapType *testObjPtr = new QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street map"),
                                              QStringLiteral("map description"), true, true, 1, pluginName, capabilities);
    QVERIFY(testObjPtr);
    QCOMPARE(testObjPtr->style(), QGeoMapType::StreetMap);
    QCOMPARE(testObjPtr->name(), QStringLiteral("street map"));
    QCOMPARE(testObjPtr->description(), QStringLiteral("map description"));
    QVERIFY(testObjPtr->mobile());
    QVERIFY(testObjPtr->night());
    QCOMPARE(testObjPtr->mapId(), 1);
    QCOMPARE(testObjPtr->pluginName(), pluginName);
    QCOMPARE(testObjPtr->cameraCapabilities(), capabilities);
    QCOMPARE(testObjPtr->metadata(), QVariantMap());
    delete testObjPtr;

    testObjPtr = new QGeoMapType();
    QCOMPARE(testObjPtr->style(), QGeoMapType::NoMap);
    QVERIFY2(testObjPtr->name().isEmpty(), "Wrong default value");
    QVERIFY2(testObjPtr->description().isEmpty(), "Wrong default value");
    QVERIFY2(!testObjPtr->mobile(), "Wrong default value");
    QVERIFY2(!testObjPtr->night(), "Wrong default value");
    QCOMPARE(testObjPtr->mapId(), 0);
    QCOMPARE(testObjPtr->pluginName(), QByteArrayLiteral(""));
    QCOMPARE(testObjPtr->cameraCapabilities(), QGeoCameraCapabilities());
    delete testObjPtr;
}

void tst_MapType::comparison_data()
{
    QTest::addColumn<QGeoMapType>("type1");
    QTest::addColumn<QGeoMapType>("type2");
    QTest::addColumn<bool>("expected");

    const QByteArray pluginName = "tst_MapType";
    QGeoCameraCapabilities capabilities;
    capabilities.setMinimumZoomLevel(0.0);
    capabilities.setMaximumZoomLevel(20.0);
    capabilities.setSupportsBearing(true);
    capabilities.setSupportsTilting(true);
    capabilities.setMinimumTilt(0);
    capabilities.setMaximumTilt(60);
    capabilities.setMinimumFieldOfView(20);
    capabilities.setMaximumFieldOfView(90);
    QGeoCameraCapabilities capabilities2 = capabilities;
    capabilities2.setMaximumFieldOfView(80);

    QTest::newRow("null") << QGeoMapType() << QGeoMapType() << true;

    QTest::newRow("equal") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                          QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                           << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                          QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                           << true;

    QTest::newRow("style") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                          QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                           << QGeoMapType(QGeoMapType::TerrainMap, QStringLiteral("street name"),
                                          QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                           << false;

    QTest::newRow("name") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                         QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                          << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("different name"),
                                         QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                          << false;

    QTest::newRow("description") << QGeoMapType(QGeoMapType::StreetMap,
                                                QStringLiteral("street name"),
                                                QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                                 << QGeoMapType(QGeoMapType::StreetMap,
                                                QStringLiteral("street name"),
                                                QStringLiteral("different desc"), false, false, 42, pluginName, capabilities)
                                 << false;

    QTest::newRow("mobile") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                           QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                            << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                           QStringLiteral("street desc"), true, false, 42, pluginName, capabilities)
                            << false;

    QTest::newRow("night") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                           QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                            << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                           QStringLiteral("street desc"), false, true, 42, pluginName, capabilities)
                            << false;

    QTest::newRow("id") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                        << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 99, pluginName, capabilities)
                        << false;

    QTest::newRow("plugin_name") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                        << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, QByteArrayLiteral("abc"), capabilities)
                        << false;

    QTest::newRow("camera_capabilities") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                        << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities2)
                        << false;

    QVariantMap metadata;
    metadata["foo"] = 42;
    QTest::newRow("metadata") << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities)
                        << QGeoMapType(QGeoMapType::StreetMap, QStringLiteral("street name"),
                                       QStringLiteral("street desc"), false, false, 42, pluginName, capabilities, metadata)
                        << false;
}

void tst_MapType::comparison()
{
    QFETCH(QGeoMapType, type1);
    QFETCH(QGeoMapType, type2);
    QFETCH(bool, expected);

    QCOMPARE(type1 == type2, expected);
    QCOMPARE(type1 != type2, !expected);
}

QTEST_APPLESS_MAIN(tst_MapType)

#include "tst_maptype.moc"
