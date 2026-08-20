// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QtEndian>

#include "QtWebSockets/qwebsocketcorsauthenticator.h"

QT_USE_NAMESPACE

class tst_QWebSocketCorsAuthenticator : public QObject
{
    Q_OBJECT

public:
    tst_QWebSocketCorsAuthenticator();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_initialization();
};

tst_QWebSocketCorsAuthenticator::tst_QWebSocketCorsAuthenticator()
{}

void tst_QWebSocketCorsAuthenticator::initTestCase()
{
}

void tst_QWebSocketCorsAuthenticator::cleanupTestCase()
{}

void tst_QWebSocketCorsAuthenticator::init()
{
}

void tst_QWebSocketCorsAuthenticator::cleanup()
{
}

void tst_QWebSocketCorsAuthenticator::tst_initialization()
{
    {
        QWebSocketCorsAuthenticator authenticator((QString()));

        QCOMPARE(authenticator.allowed(), true);
        QCOMPARE(authenticator.origin(), QString());
    }
    {
        QWebSocketCorsAuthenticator authenticator(QStringLiteral("com.somesite"));

        QCOMPARE(authenticator.allowed(), true);
        QCOMPARE(authenticator.origin(), QStringLiteral("com.somesite"));

        QWebSocketCorsAuthenticator other(authenticator);
        QCOMPARE(other.origin(), authenticator.origin());
        QCOMPARE(other.allowed(), authenticator.allowed());

        authenticator.setAllowed(false);
        QVERIFY(!authenticator.allowed());
        QCOMPARE(other.allowed(), true);   //make sure other is a real copy

        authenticator.setAllowed(true);
        QVERIFY(authenticator.allowed());

        authenticator.setAllowed(false);
        other = authenticator;
        QCOMPARE(other.origin(), authenticator.origin());
        QCOMPARE(other.allowed(), authenticator.allowed());
    }
}

QTEST_MAIN(tst_QWebSocketCorsAuthenticator)

#include "tst_qwebsocketcorsauthenticator.moc"

