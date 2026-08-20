// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtLocation/QPlaceReply>

QT_USE_NAMESPACE

class TestReply : public QPlaceReply
{
public:
    TestReply(QObject *parent) : QPlaceReply(parent) {}
    void setFinished(bool finished) { QPlaceReply::setFinished(finished); }
    void setError(QPlaceReply::Error error, const QString &errorString) {
        QPlaceReply::setError(error,errorString);
    }
};

class tst_QPlaceReply : public QObject
{
    Q_OBJECT

public:
    tst_QPlaceReply();

private Q_SLOTS:
    void constructorTest();
    void typeTest();
    void finishedTest();
    void errorTest();
};

tst_QPlaceReply::tst_QPlaceReply()
{

}

void tst_QPlaceReply::constructorTest()
{
    TestReply *reply = new TestReply(this);
    QCOMPARE(reply->parent(), this);
    delete reply;
}

void tst_QPlaceReply::typeTest()
{
    TestReply *reply = new TestReply(this);
    QCOMPARE(reply->type(), QPlaceReply::Reply);

    delete reply;
}

void tst_QPlaceReply::finishedTest()
{
    TestReply *reply = new TestReply(this);
    QCOMPARE(reply->isFinished(), false);
    reply->setFinished(true);
    QCOMPARE(reply->isFinished(), true);

    delete reply;
}

void tst_QPlaceReply::errorTest()
{
    TestReply *reply = new TestReply(this);
    QCOMPARE(reply->error(), QPlaceReply::NoError);
    QCOMPARE(reply->errorString(), QString());

    reply->setError(QPlaceReply::CommunicationError, QStringLiteral("Could not connect to server"));
    QCOMPARE(reply->error(), QPlaceReply::CommunicationError);
    QCOMPARE(reply->errorString(), QStringLiteral("Could not connect to server"));

    reply->setError(QPlaceReply::NoError, QString());
    QCOMPARE(reply->error(), QPlaceReply::NoError);
    QCOMPARE(reply->errorString(), QString());

    delete reply;
}

QTEST_APPLESS_MAIN(tst_QPlaceReply)

#include "tst_qplacereply.moc"
