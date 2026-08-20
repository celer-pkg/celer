// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore>
#include <QtTest>
#include <QtNetwork>

#include <QtNetworkAuth/qabstractoauth.h>

#include <private/qabstractoauth_p.h>

class tst_AbstractOAuth : public QObject
{
    Q_OBJECT

private:
    struct AbstractOAuthPrivate : public QAbstractOAuthPrivate {
        AbstractOAuthPrivate() : QAbstractOAuthPrivate("", QUrl(), QString(), nullptr)
        {}
    };

    struct AbstractOAuth : QAbstractOAuth {
        AbstractOAuth() : QAbstractOAuth(*new AbstractOAuthPrivate(),
                                         nullptr)
        {}

        QNetworkReply *head(const QUrl &, const QVariantMap &) override { return nullptr; }
        QNetworkReply *get(const QUrl &, const QVariantMap &) override { return nullptr; }
        QNetworkReply *post(const QUrl &, const QVariantMap &) override { return nullptr; }
        QNetworkReply *put(const QUrl &, const QVariantMap &) override { return nullptr; }
        QNetworkReply *deleteResource(const QUrl &, const QVariantMap &) override
        {
            return nullptr;
        }
        void grant() override {}

        void prepareRequest(QNetworkRequest *, const QByteArray &,
                            const QByteArray & = QByteArray()) override
        {
        }
    };

private Q_SLOTS:
    void authorizationUrlSignal();
    void generateRandomString_data();
    void generateRandomString();
};

void tst_AbstractOAuth::authorizationUrlSignal()
{
    AbstractOAuth obj;
    QUrl expectedValue = QUrl("http://example.net/");
    const QUrl defaultValue = obj.authorizationUrl();
    QVERIFY(expectedValue != defaultValue);
    bool emitted = false;
    connect(&obj, &QAbstractOAuth::authorizationUrlChanged, this, [&](const QUrl &value) {
        QCOMPARE(expectedValue, value);
        emitted = true;
    });
    obj.setAuthorizationUrl(expectedValue);
    QVERIFY(emitted);
}

void tst_AbstractOAuth::generateRandomString_data()
{
    QTest::addColumn<int>("length");
    for (int i = 0; i <= 255; ++i)
        QTest::addRow("%d", i) << i;
}

// copied from https://xkcd.com/221/
int getRandomNumber()
{
    return 4;   // chosen by fair dice roll.
                // guaranteed to be random.
}

void tst_AbstractOAuth::generateRandomString()
{
    struct Unprotected : public QAbstractOAuth {
        using QAbstractOAuth::generateRandomString;
    };

    QFETCH(int, length);
    QByteArray random1 = Unprotected::generateRandomString(length);
    QCOMPARE(random1.size(), length);

    // Check that it is truly random by repeating and checking that it is
    // different. We don't try it for 1 and 2 characters because the chance of
    // random coincidence is too high: 1 in 2^(6*n), so 1 in 64 and 1 in 4096
    // respectively. For 3 characters, that decreases to 1 in 262,144.
    if (length <= 2)
        return;

    QByteArray random2 = Unprotected::generateRandomString(length);
    QCOMPARE_NE(random2, random1);

    // Generate a Base64 string using getRandomNumber() random bytes. Base64
    // encodes 6 bits per byte, so a 255-character string has 1530 bits of
    // data.
    char buf[192] = {};
    int rawlen = (length * 6 + 7) / 8;
    for (int i = 0; i < rawlen; ++i)
        buf[i] = getRandomNumber();
    QByteArray random3 = QByteArray(buf, rawlen).toBase64(QByteArray::Base64UrlEncoding);
    Q_ASSERT(random3.size() >= length);
    random3.truncate(length);
    QCOMPARE_NE(random3, random1);
}

QTEST_MAIN(tst_AbstractOAuth)
#include "tst_abstractoauth.moc"
