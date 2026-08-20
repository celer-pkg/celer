// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtLocation/QPlaceSearchResult>
#include <QtLocation/QPlaceIcon>

QT_USE_NAMESPACE

class tst_QPlaceSearchResult : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructorTest();
    void title();
    void icon();
    void operators();
};

void tst_QPlaceSearchResult::constructorTest()
{
    QPlaceSearchResult result;

    QCOMPARE(result.type(), QPlaceSearchResult::UnknownSearchResult);
    QVERIFY(result.title().isEmpty());
    QVERIFY(result.icon().isEmpty());

    result.setTitle(QStringLiteral("title"));
    QPlaceIcon icon;
    QVariantMap parameters;
    parameters.insert(QStringLiteral("paramKey"), QStringLiteral("paramValue"));
    icon.setParameters(parameters);
    result.setIcon(icon);

    QPlaceSearchResult result2(result);
    QCOMPARE(result2.title(), QStringLiteral("title"));
    QCOMPARE(result2.icon().parameters().value(QStringLiteral("paramKey")).toString(),
             QStringLiteral("paramValue"));

    QCOMPARE(result2, result);
}

void tst_QPlaceSearchResult::title()
{
    QPlaceSearchResult result;
    QVERIFY(result.title().isEmpty());
    result.setTitle(QStringLiteral("title"));
    QCOMPARE(result.title(), QStringLiteral("title"));
    result.setTitle(QString());
    QVERIFY(result.title().isEmpty());
}

void tst_QPlaceSearchResult::icon()
{
    QPlaceSearchResult result;
    QVERIFY(result.icon().isEmpty());
    QPlaceIcon icon;
    QVariantMap iconParams;
    iconParams.insert(QStringLiteral("paramKey"), QStringLiteral("paramValue"));
    result.setIcon(icon);
    QCOMPARE(result.icon(), icon);
    result.setIcon(QPlaceIcon());
    QVERIFY(result.icon().isEmpty());
}

void tst_QPlaceSearchResult::operators()
{
    QPlaceSearchResult result1;
    QPlaceSearchResult result2;

    QVERIFY(result1 == result2);
    QVERIFY(!(result1 != result2));

    result1.setTitle(QStringLiteral("title"));
    QVERIFY(!(result1 == result2));
    QVERIFY(result1 != result2);

    result2.setTitle(QStringLiteral("title"));
    QVERIFY(result1 == result2);
    QVERIFY(!(result1 != result2));
}

QTEST_APPLESS_MAIN(tst_QPlaceSearchResult)

#include "tst_qplacesearchresult.moc"
