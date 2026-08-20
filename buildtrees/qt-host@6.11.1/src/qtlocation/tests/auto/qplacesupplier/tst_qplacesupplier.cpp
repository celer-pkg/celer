// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <qplaceicon.h>
#include <qplacesupplier.h>

QT_USE_NAMESPACE

class tst_QPlaceSupplier : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceSupplier();

private Q_SLOTS:
    void constructorTest();
    void nameTest();
    void supplierIdTest();
    void urlTest();
    void iconTest();
    void operatorsTest();
    void isEmptyTest();
};

tst_QPlaceSupplier::tst_QPlaceSupplier()
{
}

void tst_QPlaceSupplier::constructorTest()
{
    QPlaceSupplier testObj;
    Q_UNUSED(testObj);

    QPlaceSupplier *testObjPtr = new QPlaceSupplier(testObj);
    QVERIFY2(testObjPtr != NULL, "Copy constructor - null");
    QVERIFY2(*testObjPtr == testObj, "Copy constructor - compare");
    delete testObjPtr;
}

void tst_QPlaceSupplier::nameTest()
{
    QPlaceSupplier testObj;
    QVERIFY2(testObj.name() == QString(), "Wrong default value");
    testObj.setName("testText");
    QVERIFY2(testObj.name() == "testText", "Wrong value returned");
}

void tst_QPlaceSupplier::supplierIdTest()
{
    QPlaceSupplier testObj;
    QVERIFY2(testObj.supplierId() == QString(), "Wrong default value");
    testObj.setSupplierId("testText");
    QVERIFY2(testObj.supplierId() == "testText", "Wrong value returned");
}

void tst_QPlaceSupplier::urlTest()
{
    QPlaceSupplier testObj;
    const QUrl testUrl = QUrl::fromEncoded("http://example.com/testUrl");
    QVERIFY2(testObj.url() == QUrl(), "Wrong default value");
    testObj.setUrl(testUrl);
    QVERIFY2(testObj.url() == testUrl, "Wrong value returned");
}

void tst_QPlaceSupplier::iconTest()
{
    QPlaceSupplier testObj;
    QVERIFY(testObj.icon().isEmpty());
    QPlaceIcon icon;
    QVariantMap iconParams;
    iconParams.insert(QPlaceIcon::SingleUrl, QUrl::fromEncoded("http://example.com/icon.png"));
    icon.setParameters(iconParams);
    testObj.setIcon(icon);
    QCOMPARE(testObj.icon(), icon);
    QCOMPARE(testObj.icon().url(), QUrl::fromEncoded("http://example.com/icon.png"));

    testObj.setIcon(QPlaceIcon());
    QVERIFY(testObj.icon().isEmpty());
    QCOMPARE(testObj.icon().url(), QUrl());
}

void tst_QPlaceSupplier::operatorsTest()
{
    QPlaceSupplier testObj;
    testObj.setName(QStringLiteral("Acme"));
    QPlaceIcon icon;
    QVariantMap iconParams;
    iconParams.insert(QPlaceIcon::SingleUrl, QUrl::fromEncoded("http://example.com/icon.png"));
    icon.setParameters(iconParams);
    testObj.setIcon(icon);
    testObj.setSupplierId(QStringLiteral("34292"));

    QPlaceSupplier testObj2;
    testObj2 = testObj;
    QVERIFY2(testObj == testObj2, "Not copied correctly");
    testObj2.setSupplierId(QStringLiteral("testValue2"));
    QVERIFY2(testObj != testObj2, "Object should be different");
}

void tst_QPlaceSupplier::isEmptyTest()
{
    QPlaceIcon icon;
    QVariantMap iconParametersMap;
    iconParametersMap.insert(QStringLiteral("Para"), QStringLiteral("meter"));
    icon.setParameters(iconParametersMap);
    QVERIFY(!icon.isEmpty());

    QPlaceSupplier supplier;

    QVERIFY(supplier.isEmpty());

    // name
    supplier.setName(QStringLiteral("Name"));
    QVERIFY(!supplier.isEmpty());
    supplier.setName(QString());
    QVERIFY(supplier.isEmpty());

    // supplierId
    supplier.setSupplierId(QStringLiteral("1"));
    QVERIFY(!supplier.isEmpty());
    supplier.setSupplierId(QString());
    QVERIFY(supplier.isEmpty());

    // url
    supplier.setUrl(QUrl(QStringLiteral("www.example.com")));
    QVERIFY(!supplier.isEmpty());
    supplier.setUrl(QUrl());
    QVERIFY(supplier.isEmpty());

    // icon
    supplier.setIcon(icon);
    QVERIFY(!supplier.isEmpty());
    supplier.setIcon(QPlaceIcon());
    QVERIFY(supplier.isEmpty());
}

QTEST_APPLESS_MAIN(tst_QPlaceSupplier);

#include "tst_qplacesupplier.moc"
