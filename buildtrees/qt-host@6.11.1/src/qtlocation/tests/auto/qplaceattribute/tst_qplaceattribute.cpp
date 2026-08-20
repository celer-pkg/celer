// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <qplaceattribute.h>

QT_USE_NAMESPACE

class tst_QPlaceAttribute : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceAttribute();

private Q_SLOTS:
    void constructorTest();
    void operatorsTest();
    void isEmptyTest();
};

tst_QPlaceAttribute::tst_QPlaceAttribute()
{
}

void tst_QPlaceAttribute::constructorTest()
{
    QPlaceAttribute testObj;

    QPlaceAttribute *testObjPtr = new QPlaceAttribute(testObj);
    QVERIFY2(testObjPtr != NULL, "Copy constructor - null");
    QVERIFY2(testObjPtr->label() == QString(), "Copy constructor - wrong label");
    QVERIFY2(testObjPtr->text() == QString(), "Copy constructor - wrong text");
    QVERIFY2(*testObjPtr == testObj, "Copy constructor - compare");
    delete testObjPtr;
}

void tst_QPlaceAttribute::operatorsTest()
{
    QPlaceAttribute testObj;
    testObj.setLabel(QStringLiteral("label"));
    QPlaceAttribute testObj2;
    testObj2 = testObj;
    QVERIFY2(testObj == testObj2, "Not copied correctly");
    testObj2.setText(QStringLiteral("text"));
    QVERIFY2(testObj != testObj2, "Object should be different");
}

void tst_QPlaceAttribute::isEmptyTest()
{
    QPlaceAttribute attribute;

    QVERIFY(attribute.isEmpty());

    attribute.setLabel(QStringLiteral("label"));
    QVERIFY(!attribute.isEmpty());
    attribute.setLabel(QString());
    QVERIFY(attribute.isEmpty());

    attribute.setText(QStringLiteral("text"));
    QVERIFY(!attribute.isEmpty());
    attribute.setText(QString());
    QVERIFY(attribute.isEmpty());
}

QTEST_APPLESS_MAIN(tst_QPlaceAttribute);

#include "tst_qplaceattribute.moc"
