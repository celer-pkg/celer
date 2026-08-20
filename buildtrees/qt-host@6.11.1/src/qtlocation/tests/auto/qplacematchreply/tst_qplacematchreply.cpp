// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtLocation/QPlace>
#include <QtLocation/QPlaceMatchReply>
#include <QtLocation/QPlaceMatchRequest>

QT_USE_NAMESPACE

class TestMatchReply : public QPlaceMatchReply
{
    Q_OBJECT
public:
    TestMatchReply(QObject *parent) : QPlaceMatchReply(parent) {}
    TestMatchReply() {}

    void setPlaces(const QList<QPlace> &places) {
        QPlaceMatchReply::setPlaces(places);
    }

    void setRequest(const QPlaceMatchRequest &request) {
        QPlaceMatchReply::setRequest(request);
    }
};

class tst_QPlaceMatchReply : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceMatchReply();

private Q_SLOTS:
    void constructorTest();
    void typeTest();
    void requestTest();
//    void resultsTest();
};

tst_QPlaceMatchReply::tst_QPlaceMatchReply()
{
}

void tst_QPlaceMatchReply::constructorTest()
{
    QPlaceMatchReply *reply = new TestMatchReply(this);
    QVERIFY(reply->parent() == this);
    delete reply;
}

void tst_QPlaceMatchReply::typeTest()
{
    TestMatchReply *reply = new TestMatchReply(this);
    QVERIFY(reply->type() == QPlaceReply::MatchReply);
    delete reply;
}

void tst_QPlaceMatchReply::requestTest()
{
    TestMatchReply *reply = new TestMatchReply(this);
    QPlaceMatchRequest request;

    QPlace place1;
    place1.setName(QStringLiteral("place1"));

    QPlace place2;
    place2.setName(QStringLiteral("place2"));

    QList<QPlace> places;
    places << place1 << place2;

    request.setPlaces(places);

    reply->setRequest(request);
    QCOMPARE(reply->request(), request);

    reply->setRequest(QPlaceMatchRequest());
    QCOMPARE(reply->request(), QPlaceMatchRequest());
    delete reply;
}


QTEST_APPLESS_MAIN(tst_QPlaceMatchReply)

#include "tst_qplacematchreply.moc"
