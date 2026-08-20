// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QtNetworkAuth/qabstractoauth2.h>

#include "oauthtestutils.h"

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

class tst_AbstractOAuth2 : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void scopeCharacterWarnings();
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    void setInvalidScope();
    void scopeAndRequestedScope_data();
    void scopeAndRequestedScope();
#endif
    void prepareRequest();
    void nonce();
    void sslConfig();
    void invalidRefreshLeadTime();
    void tokenUrl();
    void autoRefresh();

private:
    QString testDataDir;
};

class TestFlow : public QAbstractOAuth2
{
    Q_OBJECT
public:
    TestFlow() {}
    explicit TestFlow(QObject *parent) : QAbstractOAuth2(parent) {}

public Q_SLOTS:
    void grant() override {}

protected Q_SLOTS:
    void refreshTokensImplementation() QT7_ONLY(override) {}
};

void tst_AbstractOAuth2::initTestCase()
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.networkauth* = true"));
    testDataDir = QFileInfo(QFINDTESTDATA("../shared/certs")).absolutePath();
    if (testDataDir.isEmpty())
        testDataDir = QCoreApplication::applicationDirPath();
    if (!testDataDir.endsWith(QLatin1String("/")))
        testDataDir += QLatin1String("/");
}

void tst_AbstractOAuth2::scopeCharacterWarnings()
{
    TestFlow oauth2;
    QCOMPARE_EQ(oauth2.requestedScopeTokens(), {});

    auto scopeCharWarning = QRegularExpression{"A scope token cannot contain disallowed character"};
    // All characters in acceptable range
    const QByteArray acceptable = "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "[]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    // All good
    oauth2.setRequestedScopeTokens({acceptable});
    QCOMPARE_EQ(oauth2.requestedScopeTokens(), {acceptable}); // changed

    // Outside of acceptable range
    QTest::ignoreMessage(QtWarningMsg, scopeCharWarning);
    oauth2.setRequestedScopeTokens({"\x03"});
    QCOMPARE_EQ(oauth2.requestedScopeTokens(), {acceptable}); // unchanged


    QTest::ignoreMessage(QtWarningMsg, scopeCharWarning);
    oauth2.setRequestedScopeTokens({u"foo€bar"_s.toUtf8()});
    QCOMPARE_EQ(oauth2.requestedScopeTokens(), {acceptable}); // unchanged

    // Empty token
    QTest::ignoreMessage(QtWarningMsg, "A scope token cannot be empty.");
    oauth2.setRequestedScopeTokens({"", "some"});
    QCOMPARE_EQ(oauth2.requestedScopeTokens(), {acceptable}); // unchanged

#ifndef QOAUTH2_NO_LEGACY_SCOPE
    QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED

    // warning here is different:
    scopeCharWarning = QRegularExpression{"Scope token contains disallowed character"};

    // All good
    oauth2.setScope(acceptable);
    QCOMPARE_EQ(oauth2.scope(), QLatin1StringView{acceptable});

    QTest::ignoreMessage(QtWarningMsg, scopeCharWarning);
    oauth2.setScope(u"\x03"_s);
    QCOMPARE_EQ(oauth2.scope(), "\x03"_L1); // setScope() doesn't reject

    QTest::ignoreMessage(QtWarningMsg, scopeCharWarning);
    oauth2.setScope(u"foo€bar"_s);
    QCOMPARE_EQ(oauth2.scope(), u"foo€bar"); // setScope() doesn't reject or reinterpret

    QT_WARNING_POP
#endif // QOAUTH2_NO_LEGACY_SCOPE
}

#ifndef QOAUTH2_NO_LEGACY_SCOPE
QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
void tst_AbstractOAuth2::setInvalidScope()
{
    const auto zeroth = u"zeroth"_s;
    const auto zerothU8 = zeroth.toUtf8();
    const auto first = u"fïrst"_s; // L1
    const auto firstU8 = first.toUtf8();
    const auto second = u"s€cond"_s; // non-L1
    const auto secondU8 = second.toUtf8();

    constexpr auto ignoreMsg = [](char16_t ch) {
        char buf[256];
        std::snprintf(buf, sizeof buf,
                      "Scope token contains disallowed character '%s' (0x%04x). "
                      "This may cause interoperability issues",
                      QString(QChar(ch)).toUtf8().constData(), uint{ch});
        QTest::ignoreMessage(QtWarningMsg, buf);
    };

    TestFlow oauth2;

    {
        oauth2.setScope(zeroth);
        QCOMPARE(oauth2.scope(), zeroth); // scope is not changed
        QCOMPARE(oauth2.requestedScopeTokens(), QSet{zerothU8}); // U8 == L1 here
    }

    {
        ignoreMsg(u'ï');
        oauth2.setScope(first);
        QCOMPARE(oauth2.scope(), first); // scope is not changed
        QCOMPARE(oauth2.requestedScopeTokens(), QSet{firstU8});
    }

    {
        ignoreMsg(u'€');
        oauth2.setScope(second);
        QCOMPARE(oauth2.scope(), second); // scope is not changed
        QCOMPARE(oauth2.requestedScopeTokens(), QSet{secondU8});
    }

    {
        ignoreMsg(u'ï');
        ignoreMsg(u'€');
        const QString scope = first + u' ' + second;
        oauth2.setScope(scope);
        QCOMPARE(oauth2.scope(), scope); // scope is not changed
        QCOMPARE(oauth2.requestedScopeTokens(), QSet({firstU8, secondU8}));
    }
}

void tst_AbstractOAuth2::scopeAndRequestedScope_data()
{
    const QByteArray f = "first";
    const QByteArray s = "second";
    const QByteArray fs = "first second";

    QTest::addColumn<QByteArray>("scope");
    QTest::addColumn<QByteArray>("expected_scope");
    QTest::addColumn<QSet<QByteArray>>("requested_scope");

    QTest::addRow("singlescope") << f << f << QSet{f};
    QTest::addRow("multiscope") << fs << fs << QSet{f, s};
}

void tst_AbstractOAuth2::scopeAndRequestedScope()
{
    QFETCH(QByteArray, scope);
    QFETCH(QByteArray, expected_scope);
    QFETCH(QSet<QByteArray>, requested_scope);

    TestFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    QVERIFY(oauth2.scope().isEmpty());
    QVERIFY(oauth2.requestedScopeTokens().isEmpty());

    QSignalSpy scopeSpy(&oauth2, &QAbstractOAuth2::scopeChanged);
    QSignalSpy requestedScopeTokensSpy(&oauth2, &QAbstractOAuth2::requestedScopeTokensChanged);

    // Set 'scope' and verify that both 'scope' and 'requestedScopeTokens' change
    oauth2.setScope(scope);

    QCOMPARE(scopeSpy.size(), 1);
    QCOMPARE(oauth2.scope(), expected_scope);
    QCOMPARE(scopeSpy.at(0).at(0).toString(), expected_scope);

    QCOMPARE(requestedScopeTokensSpy.size(), 1);
    QCOMPARE(oauth2.requestedScopeTokens(), requested_scope);
    QCOMPARE(requestedScopeTokensSpy.at(0).at(0).value<QSet<QByteArray>>(), requested_scope);

    // Clear data
    oauth2.setScope(u""_s);
    oauth2.setRequestedScopeTokens({});
    scopeSpy.clear();
    requestedScopeTokensSpy.clear();

    // Set 'requestedScopeTokens' and verify that both 'scope' and 'requestedScopeTokens' change
    oauth2.setRequestedScopeTokens(requested_scope);

    QCOMPARE(requestedScopeTokensSpy.size(), 1);
    QCOMPARE(oauth2.requestedScopeTokens(), requested_scope);
    QCOMPARE(requestedScopeTokensSpy.at(0).at(0).value<QSet<QByteArray>>(), requested_scope);

    QCOMPARE(scopeSpy.size(), 1);
    auto scopeTokens = oauth2.scope().toLatin1().split(' ');
    QCOMPARE_EQ(scopeTokens.size(), requested_scope.size());
    for (const auto &token : scopeTokens)
        QVERIFY2(requested_scope.contains(token), token.data());
}
QT_WARNING_POP
#endif // QOAUTH2_NO_LEGACY_SCOPE

void tst_AbstractOAuth2::prepareRequest()
{
    TestFlow oauth2;
    oauth2.setToken(QStringLiteral("access_token"));

    QNetworkRequest request(QUrl("http://localhost"));
    oauth2.prepareRequest(&request, QByteArray());
    QCOMPARE(request.rawHeader("Authorization"), QByteArray("Bearer access_token"));
}

void tst_AbstractOAuth2::nonce()
{
    TestFlow oauth2;
    const auto nonce = "a_nonce"_ba;

    // Test setting nonce mode
    QSignalSpy nonceModeSpy(&oauth2, &QAbstractOAuth2::nonceModeChanged);
    // -- Default
    QCOMPARE(oauth2.nonceMode(), QAbstractOAuth2::NonceMode::Automatic);
    // -- Change
    oauth2.setNonceMode(QAbstractOAuth2::NonceMode::Disabled);
    QCOMPARE(nonceModeSpy.size(), 1);
    QCOMPARE(nonceModeSpy.at(0).at(0).value<QAbstractOAuth2::NonceMode>(),
             QAbstractOAuth2::NonceMode::Disabled);
    QCOMPARE(oauth2.nonceMode(), QAbstractOAuth2::NonceMode::Disabled);
    // -- Attempt to change again, but to same value
    oauth2.setNonceMode(QAbstractOAuth2::NonceMode::Disabled);
    QCOMPARE(nonceModeSpy.size(), 1);
    QCOMPARE(oauth2.nonceMode(), QAbstractOAuth2::NonceMode::Disabled);

    // Test setting nonce value
    QSignalSpy nonceSpy(&oauth2, &QAbstractOAuth2::nonceChanged);
    // -- Default
    QVERIFY(oauth2.nonce().isEmpty());
    // -- Change
    oauth2.setNonce(nonce);
    QCOMPARE(nonceSpy.size(), 1);
    QCOMPARE(nonceSpy.at(0).at(0).toByteArray(), nonce);
    QCOMPARE(oauth2.nonce(), nonce);
    // -- Attempt to change again, but to same value
    oauth2.setNonce(nonce);
    QCOMPARE(nonceSpy.size(), 1);
    QCOMPARE(oauth2.nonce(), nonce);
}

void tst_AbstractOAuth2::sslConfig()
{
#ifdef QT_NO_SSL
    QSKIP("Skipping SSL test, not supported by build");
#else
    TestFlow oauth2;
    QSignalSpy sslConfigSpy(&oauth2, &QAbstractOAuth2::sslConfigurationChanged);

    QVERIFY(sslConfigSpy.isValid());
    QCOMPARE(oauth2.sslConfiguration(), QSslConfiguration());
    QCOMPARE(sslConfigSpy.size(), 0);

    auto config = createSslConfiguration(testDataDir + "certs/selfsigned-server.key",
                                         testDataDir + "certs/selfsigned-server.crt");
    oauth2.setSslConfiguration(config);

    QCOMPARE(oauth2.sslConfiguration(), config);
    QCOMPARE(sslConfigSpy.size(), 1);

    // set same config - nothing happens
    oauth2.setSslConfiguration(config);
    QCOMPARE(sslConfigSpy.size(), 1);

    // change config
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    oauth2.setSslConfiguration(config);
    QCOMPARE(oauth2.sslConfiguration(), config);
    QCOMPARE(sslConfigSpy.size(), 2);
#endif // QT_NO_SSL
}

void tst_AbstractOAuth2::invalidRefreshLeadTime()
{
    TestFlow oauth2;
    QCOMPARE(oauth2.refreshLeadTime(), 0s);
    QTest::ignoreMessage(QtWarningMsg, "Invalid refresh leadTime");
    oauth2.setRefreshLeadTime(-5s);
    QCOMPARE(oauth2.refreshLeadTime(), 0s);
}

void tst_AbstractOAuth2::tokenUrl()
{
    TestFlow oauth2;
    QCOMPARE_EQ(oauth2.tokenUrl(), QUrl());

    const QUrl someTokenUrl{"accessToken"_L1};
    const QUrl otherTokenUrl{"otherAccessToken"_L1};

    QSignalSpy tokenUrlChangedSpy(&oauth2, &QAbstractOAuth2::tokenUrlChanged);

    oauth2.setTokenUrl(someTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), someTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(tokenUrlChangedSpy.at(0).at(0).toUrl(), someTokenUrl);

    // setting the same value does not trigger any update
    tokenUrlChangedSpy.clear();
    oauth2.setTokenUrl(someTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), someTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 0);

    // set another value
    tokenUrlChangedSpy.clear();
    oauth2.setTokenUrl(otherTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), otherTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(tokenUrlChangedSpy.at(0).at(0).toUrl(), otherTokenUrl);
}

void tst_AbstractOAuth2::autoRefresh()
{
    TestFlow oauth2;
    QSignalSpy autoRefreshSpy(&oauth2, &QAbstractOAuth2::autoRefreshChanged);

    QCOMPARE(oauth2.autoRefresh(), false);

    oauth2.setAutoRefresh(true);
    QCOMPARE(autoRefreshSpy.size(), 1);
    QCOMPARE(oauth2.autoRefresh(), true);
    QCOMPARE(autoRefreshSpy.at(0).at(0).toBool(), true);

    autoRefreshSpy.clear();
    oauth2.setAutoRefresh(false);
    QCOMPARE(autoRefreshSpy.size(), 1);
    QCOMPARE(oauth2.autoRefresh(), false);
    QCOMPARE(autoRefreshSpy.at(0).at(0).toBool(), false);
}


QTEST_MAIN(tst_AbstractOAuth2)
#include "tst_abstractoauth2.moc"
