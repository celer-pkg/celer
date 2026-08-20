// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtLocation/QPlaceSearchSuggestionReply>

QT_USE_NAMESPACE

class SuggestionReply : public QPlaceSearchSuggestionReply
{
    Q_OBJECT
public:
    SuggestionReply(QObject *parent) : QPlaceSearchSuggestionReply(parent){}

    void setSuggestions(const QStringList &suggestions) {
        QPlaceSearchSuggestionReply::setSuggestions(suggestions);
    }
};

class tst_QPlaceSearchSuggestionReply : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceSearchSuggestionReply();

private Q_SLOTS:
    void constructorTest();
    void typeTest();
    void suggestionsTest();
};

tst_QPlaceSearchSuggestionReply::tst_QPlaceSearchSuggestionReply()
{
}

void tst_QPlaceSearchSuggestionReply::constructorTest()
{
    SuggestionReply *reply = new SuggestionReply(this);
    QCOMPARE(reply->parent(), this);

    delete reply;
}

void tst_QPlaceSearchSuggestionReply::typeTest()
{
    SuggestionReply *reply = new SuggestionReply(this);
    QCOMPARE(reply->type(), QPlaceReply::SearchSuggestionReply);

    delete reply;
}

void tst_QPlaceSearchSuggestionReply::suggestionsTest()
{
    QStringList suggestions;
    suggestions << QStringLiteral("one") << QStringLiteral("two")
                << QStringLiteral("three");

    SuggestionReply *reply = new SuggestionReply(this);
    reply->setSuggestions(suggestions);
    QCOMPARE(reply->suggestions(), suggestions);

    delete reply;
}

QTEST_APPLESS_MAIN(tst_QPlaceSearchSuggestionReply)

#include "tst_qplacesearchsuggestionreply.moc"
