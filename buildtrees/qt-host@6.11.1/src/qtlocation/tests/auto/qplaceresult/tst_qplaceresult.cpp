// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceIcon>
#include <QtTest/QtTest>

#include "../utils/qlocationtestutils_p.h"

QT_USE_NAMESPACE

class tst_QPlaceResult : public QObject
{
    Q_OBJECT

public:
    QPlaceResult initialSubObject();
    bool checkType(const QPlaceSearchResult &);
    void detach(QPlaceSearchResult *);
    void setSubClassProperty(QPlaceResult *);

private Q_SLOTS:
    void constructorTest();
    void title();
    void icon();
    void distance();
    void place();
    void sponsored();
    void conversion();
};

QPlaceResult tst_QPlaceResult::initialSubObject()
{
    QPlaceResult placeResult;
    placeResult.setTitle(QStringLiteral("title"));

    QPlaceIcon icon;
    QVariantMap parameters;
    parameters.insert(QPlaceIcon::SingleUrl,
                      QUrl(QStringLiteral("file:///opt/icons/icon.png")));
    icon.setParameters(parameters);
    placeResult.setIcon(icon);

    QPlace place;
    place.setName(QStringLiteral("place"));
    placeResult.setPlace(place);

    placeResult.setDistance(5);
    placeResult.setSponsored(true);

    return placeResult;
}

bool tst_QPlaceResult::checkType(const QPlaceSearchResult &result)
{
    return result.type() == QPlaceSearchResult::PlaceResult;
}

void tst_QPlaceResult::detach(QPlaceSearchResult * result)
{
    result->setTitle("title");
}

void tst_QPlaceResult::setSubClassProperty(QPlaceResult *result)
{
    result->setSponsored(false);
}

void tst_QPlaceResult::constructorTest()
{
    QPlaceResult result;
    QCOMPARE(result.type(), QPlaceSearchResult::PlaceResult);

    result.setTitle(QStringLiteral("title"));

    QPlaceIcon icon;
    QVariantMap parameters;
    parameters.insert(QStringLiteral("paramKey"), QStringLiteral("paramValue"));
    icon.setParameters(parameters);
    result.setIcon(icon);

    QPlace place;
    place.setName("place");
    result.setPlace(place);

    result.setDistance(500);
    result.setSponsored(true);

    //check copy constructor
    QPlaceResult result2(result);
    QCOMPARE(result2.title(), QStringLiteral("title"));
    QCOMPARE(result2.icon(), icon);
    QCOMPARE(result2.place(), place);
    QVERIFY(qFuzzyCompare(result.distance(), 500));
    QCOMPARE(result2.isSponsored(), true);

    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    //check results are the same after detachment of underlying shared data pointer
    result2.setTitle("title");
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    //check construction of SearchResult using a PlaceResult
    QPlaceSearchResult searchResult(result);
    QCOMPARE(searchResult.title(), QStringLiteral("title"));
    QCOMPARE(searchResult.icon(), icon);
    QVERIFY(QLocationTestUtils::compareEquality(searchResult, result));
    QVERIFY(searchResult.type() == QPlaceSearchResult::PlaceResult);
    result2 = searchResult;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    //check construction of a SearchResult using a SearchResult
    //that is actually a PlaceResult underneath
    QPlaceSearchResult searchResult2(searchResult);
    QCOMPARE(searchResult2.title(), QStringLiteral("title"));
    QCOMPARE(searchResult2.icon(), icon);
    QVERIFY(QLocationTestUtils::compareEquality(searchResult2, result));
    QVERIFY(QLocationTestUtils::compareEquality(searchResult, searchResult2));
    QVERIFY(searchResult2.type() == QPlaceSearchResult::PlaceResult);
    result2 = searchResult2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::title()
{
    QPlaceResult result;
    QVERIFY(result.title().isEmpty());

    result.setTitle(QStringLiteral("title"));
    QCOMPARE(result.title(), QStringLiteral("title"));

    result.setTitle(QString());
    QVERIFY(result.title().isEmpty());

    QPlaceResult result2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    result2.setTitle("title");
    QVERIFY(QLocationTestUtils::compareInequality(result, result2));

    result.setTitle("title");
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::icon()
{
    QPlaceResult result;
    QVERIFY(result.icon().isEmpty());

    QPlaceIcon icon;
    QVariantMap iconParams;
    iconParams.insert(QStringLiteral("paramKey"), QStringLiteral("paramValue"));
    icon.setParameters(iconParams);
    result.setIcon(icon);
    QCOMPARE(result.icon(), icon);

    result.setIcon(QPlaceIcon());
    QVERIFY(result.icon().isEmpty());

    QPlaceResult result2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    result2.setIcon(icon);
    QVERIFY(QLocationTestUtils::compareInequality(result, result2));

    result.setIcon(icon);
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::distance()
{
    QPlaceResult result;
    QVERIFY(qIsNaN(result.distance()));

    result.setDistance(3.14);
    QVERIFY(qFuzzyCompare(result.distance(), 3.14));

    result.setDistance(qQNaN());
    QVERIFY(qIsNaN(result.distance()));

    QPlaceResult result2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    result2.setDistance(3.14);
    QVERIFY(QLocationTestUtils::compareInequality(result, result2));

    result.setDistance(3.14);
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::place()
{
    QPlaceResult result;
    QCOMPARE(result.place(), QPlace());

    QPlace place;
    place.setName("place");
    result.setPlace (place);
    QCOMPARE(result.place(), place);

    result.setPlace(QPlace());
    QCOMPARE(result.place(), QPlace());

    QPlaceResult result2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    result2.setPlace(place);
    QVERIFY(QLocationTestUtils::compareInequality(result, result2));

    result.setPlace(place);
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::sponsored()
{
    QPlaceResult result;
    QCOMPARE(result.isSponsored(), false);

    result.setSponsored(true);
    QCOMPARE(result.isSponsored(), true);

    result.setSponsored(false);
    QCOMPARE(result.isSponsored(), false);

    QPlaceResult result2;
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));

    result2.setSponsored(true);
    QVERIFY(QLocationTestUtils::compareInequality(result, result2));

    result.setSponsored(true);
    QVERIFY(QLocationTestUtils::compareEquality(result, result2));
}

void tst_QPlaceResult::conversion()
{
    QLocationTestUtils::testConversion<tst_QPlaceResult,
                                       QPlaceSearchResult,
                                       QPlaceResult>(this);
}

QTEST_APPLESS_MAIN(tst_QPlaceResult)

#include "tst_qplaceresult.moc"
