// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtLocation/QPlace>
#include <QtLocation/QPlaceDetailsReply>

QT_USE_NAMESPACE

class TestDetailsReply : public QPlaceDetailsReply
{
    Q_OBJECT
public:
    TestDetailsReply(QObject *parent) : QPlaceDetailsReply(parent){}
    ~TestDetailsReply() {}
    void setPlace(const QPlace &place) { QPlaceDetailsReply::setPlace(place); }
};

class tst_QPlaceDetailsReply : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceDetailsReply();

private Q_SLOTS:
    void constructorTest();
    void typeTest();
    void placeTest();
};

tst_QPlaceDetailsReply::tst_QPlaceDetailsReply()
{
}

void tst_QPlaceDetailsReply::constructorTest()
{
    TestDetailsReply *reply = new TestDetailsReply(this);
    QVERIFY(reply->parent() == this);
    delete reply;
}

void tst_QPlaceDetailsReply::typeTest()
{
    TestDetailsReply *reply = new TestDetailsReply(this);
    QCOMPARE(reply->type(), QPlaceReply::DetailsReply);
    delete reply;
}

void tst_QPlaceDetailsReply::placeTest()
{
    TestDetailsReply *reply = new TestDetailsReply(this);
    QPlace place;
    place.setName(QStringLiteral("Gotham City"));
    reply->setPlace(place);

    QCOMPARE(reply->place(), place);
    delete reply;
}

QTEST_APPLESS_MAIN(tst_QPlaceDetailsReply)

#include "tst_qplacedetailsreply.moc"
