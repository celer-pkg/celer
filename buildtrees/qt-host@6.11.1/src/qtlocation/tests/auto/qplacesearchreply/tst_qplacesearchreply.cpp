// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtPositioning/QGeoCircle>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/QPlaceSearchReply>
#include <QtLocation/QPlaceResult>


QT_USE_NAMESPACE

class TestSearchReply : public QPlaceSearchReply
{
    Q_OBJECT
public:
    TestSearchReply(QObject *parent) : QPlaceSearchReply(parent) {}
    TestSearchReply() {}

    void setResults(const QList<QPlaceSearchResult> &results) {
        QPlaceSearchReply::setResults(results);
    }

    void setRequest(const QPlaceSearchRequest &request) {
        QPlaceSearchReply::setRequest(request);
    }
};

class tst_QPlaceSearchReply : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceSearchReply();

private Q_SLOTS:
    void constructorTest();
    void typeTest();
    void requestTest();
    void resultsTest();
};

tst_QPlaceSearchReply::tst_QPlaceSearchReply()
{
}

void tst_QPlaceSearchReply::constructorTest()
{
    TestSearchReply *reply = new TestSearchReply(this);
    QVERIFY(reply->parent() == this);
    delete reply;
}

void tst_QPlaceSearchReply::typeTest()
{
    TestSearchReply *reply = new TestSearchReply(this);
    QVERIFY(reply->type() == QPlaceReply::SearchReply);
    delete reply;
}

void tst_QPlaceSearchReply::requestTest()
{
    TestSearchReply *reply = new TestSearchReply(this);
    QPlaceSearchRequest request;
    request.setLimit(10);

    QGeoCircle circle;
    circle.setCenter(QGeoCoordinate(10,20));
    request.setSearchArea(circle);

    request.setSearchTerm("pizza");

    reply->setRequest(request);
    QCOMPARE(reply->request(), request);
    reply->setRequest(QPlaceSearchRequest());
    QCOMPARE(reply->request(), QPlaceSearchRequest());
    delete reply;
}

void tst_QPlaceSearchReply::resultsTest()
{
    TestSearchReply *reply = new TestSearchReply(this);
    QList<QPlaceSearchResult> results;
    QPlace winterfell;
    winterfell.setName("Winterfell");
    QPlace casterlyRock;
    casterlyRock.setName("Casterly Rock");
    QPlace stormsEnd;
    stormsEnd.setName("Storm's end");

    QPlaceResult result1;
    result1.setPlace(winterfell);
    QPlaceResult result2;
    result2.setPlace(casterlyRock);
    QPlaceResult result3;
    result3.setPlace(stormsEnd);
    results << result1 << result2 << result3;

    reply->setResults(results);
    QCOMPARE(reply->results(), results);
    reply->setResults(QList<QPlaceSearchResult>());
    QCOMPARE(reply->results(), QList<QPlaceSearchResult>());
    delete reply;
}

QTEST_APPLESS_MAIN(tst_QPlaceSearchReply)

#include "tst_qplacesearchreply.moc"
