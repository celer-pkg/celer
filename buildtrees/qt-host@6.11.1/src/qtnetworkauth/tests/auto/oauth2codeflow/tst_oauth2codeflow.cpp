// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtNetworkAuth/qabstractoauthreplyhandler.h>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>

#include "oauthtestutils.h"

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

class tst_OAuth2CodeFlow : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void state();
#if QT_DEPRECATED_SINCE(6, 13)
    void accessTokenUrl();
#endif
    void getToken();
    void refreshToken();
    void getAndRefreshToken();
    void tokenRequestErrors();
    void authorizationErrors();
    void modifyTokenRequests();
    void pkce_data();
    void pkce();
    void nonce();
    void idToken();
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    void scope_data();
    void scope();
#endif
    void requestedScopeTokens_data();
    void requestedScopeTokens();
    void grantedScopeTokens_data();
    void grantedScopeTokens();
    void refreshLeadTime_data();
    void refreshLeadTime();
    void alreadyExpiredTokenClientSideRefresh();

#ifndef QT_NO_SSL
    void tlsAuthentication();
#endif
    void extraTokens();
    void expirationAt();

private:
    QString testDataDir;
};

namespace Responses {

    static const auto tokenSuccess = R"(
            {
                "access_token": "an-access-token",
                "refresh_token": "a-refresh-token",
                "token_type": "bearer",
                "expires_in": 3600
            })"_ba;

    static const auto OK_200 = "200 OK"_ba;

    ServerResponses defaults()
    {
        ServerResponses responses;
        // responses.authBody not used by code flow tests
        responses.authHttpStatus = OK_200;
        responses.tokenBody = tokenSuccess;
        responses.tokenHttpStatus = OK_200;
        return responses;
    }
}

struct ReplyHandler : QAbstractOAuthReplyHandler
{
    QString callback() const override
    {
        return QLatin1String("test");
    }

    QAbstractOAuth::Error aTokenRequestError = QAbstractOAuth::Error::NoError;

    void networkReplyFinished(QNetworkReply *reply) override
    {
        if (aTokenRequestError != QAbstractOAuth::Error::NoError) {
            emit tokenRequestErrorOccurred(aTokenRequestError, "a token request error");
            return;
        }
        auto data = reply->readAll();
        const QJsonDocument document = QJsonDocument::fromJson(data);
        emit tokensReceived(document.object().toVariantMap());
    }

    void emitCallbackReceived(const QVariantMap &data)
    {
        Q_EMIT callbackReceived(data);
    }

    void emitTokensReceived(const QVariantMap &data)
    {
        Q_EMIT tokensReceived(data);
    }
};

void tst_OAuth2CodeFlow::initTestCase()
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.networkauth* = true"));
    testDataDir = QFileInfo(QFINDTESTDATA("../shared/certs")).absolutePath();
    if (testDataDir.isEmpty())
        testDataDir = QCoreApplication::applicationDirPath();
    if (!testDataDir.endsWith(QLatin1String("/")))
        testDataDir += QLatin1String("/");
}

void tst_OAuth2CodeFlow::state()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    QSignalSpy statePropertySpy(&oauth2, &QAbstractOAuth2::stateChanged);

    QString stateParameter;
    oauth2.setModifyParametersFunction(
        [&] (QAbstractOAuth::Stage, QMultiMap<QString, QVariant> *parameters) {
            stateParameter = parameters->value(u"state"_s).toString();
    });

    oauth2.grant();
    QVERIFY(!stateParameter.isEmpty()); // internally generated initial state used
    QCOMPARE(stateParameter, oauth2.state());

    // Test setting the 'state' property
    const QString simpleState = u"a_state"_s;
    oauth2.setState(simpleState);
    QCOMPARE(oauth2.state(), simpleState);
    QCOMPARE(statePropertySpy.size(), 1);
    QCOMPARE(statePropertySpy.at(0).at(0), simpleState);
    oauth2.grant();
    QCOMPARE(stateParameter, simpleState);

    // Test 'state' that contains illegal characters
    QTest::ignoreMessage(QtWarningMsg, "setState() contains illegal character(s), ignoring");
    oauth2.setState(u"foo€bar"_s);
    QCOMPARE(oauth2.state(), simpleState);
    QCOMPARE(statePropertySpy.size(), 1);

    // Test 'state' that requires encoding/decoding.
    // The 'state' value contains all allowed characters as defined by
    // https://datatracker.ietf.org/doc/html/rfc6749#appendix-A.5
    // state      = 1*VSCHAR
    // Where
    // VSCHAR     = %x20-7E
    const QString stateRequiringEncoding = u"! \"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"_s;
    const QString stateAsEncoded = u"%21+%22%23%24%25%26%27%28%29%2A%2B%2C-.%2F0123456789%3A%3B%3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D~"_s;
    oauth2.setState(stateRequiringEncoding);
    QCOMPARE(oauth2.state(), stateRequiringEncoding);
    oauth2.grant();
    QCOMPARE(stateParameter, stateAsEncoded);
    // Conclude authorization stage, and check that the 'state' which we returned as encoded
    // matches the original decoded state (ie. the status changes to TemporaryCredentialsReceived)
    replyHandler.emitCallbackReceived({{"code", "acode"}, {"state", stateAsEncoded}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);
}

#if QT_DEPRECATED_SINCE(6, 13)
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
void tst_OAuth2CodeFlow::accessTokenUrl()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    QCOMPARE_EQ(oauth2.accessTokenUrl(), QUrl());

    const QUrl someTokenUrl{"accessToken"_L1};
    const QUrl otherTokenUrl{"otherAccessToken"_L1};
    QSignalSpy tokenUrlChangedSpy(&oauth2, &QAbstractOAuth2::tokenUrlChanged);
    QSignalSpy accessTokenUrlChangedSpy(&oauth2,
                                        &QOAuth2AuthorizationCodeFlow::accessTokenUrlChanged);

    oauth2.setAccessTokenUrl(someTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), someTokenUrl);
    QCOMPARE_EQ(oauth2.accessTokenUrl(), someTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(tokenUrlChangedSpy.at(0).at(0).toUrl(), someTokenUrl);
    QCOMPARE_EQ(accessTokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(accessTokenUrlChangedSpy.at(0).at(0).toUrl(), someTokenUrl);

    // setting the same value does not trigger any update
    tokenUrlChangedSpy.clear();
    accessTokenUrlChangedSpy.clear();
    oauth2.setAccessTokenUrl(someTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), someTokenUrl);
    QCOMPARE_EQ(oauth2.accessTokenUrl(), someTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 0);
    QCOMPARE_EQ(accessTokenUrlChangedSpy.size(), 0);

    // set another value
    tokenUrlChangedSpy.clear();
    accessTokenUrlChangedSpy.clear();
    oauth2.setAccessTokenUrl(otherTokenUrl);
    QCOMPARE_EQ(oauth2.tokenUrl(), otherTokenUrl);
    QCOMPARE_EQ(oauth2.accessTokenUrl(), otherTokenUrl);
    QCOMPARE_EQ(tokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(tokenUrlChangedSpy.at(0).at(0).toUrl(), otherTokenUrl);
    QCOMPARE_EQ(accessTokenUrlChangedSpy.size(), 1);
    QCOMPARE_EQ(accessTokenUrlChangedSpy.at(0).at(0).toUrl(), otherTokenUrl);
}
QT_WARNING_POP
#endif

QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
void tst_OAuth2CodeFlow::authorizationErrors()
{
    // This tests failures in authorization stage. For this test we don't need a web server
    // as we emit the final (failing) callbackReceived directly.
    // Helper to catch the expected warning messages:
    constexpr auto expectWarning = [](){
        static const QRegularExpression authStageWarning{"Authorization stage:.*"};
        QTest::ignoreMessage(QtWarningMsg, authStageWarning);
    };

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorization"_L1});
#if QT_DEPRECATED_SINCE(6, 13)
    oauth2.setAccessTokenUrl(QUrl{"accessToken"_L1});
#else
    oauth2.setTokenUrl(QUrl{"accessToken"_L1});
#endif
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);

    QVariantMap callbackParameters;
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &oauth2, [&](const QUrl& /* url */) {
        replyHandler.emitCallbackReceived(callbackParameters);
    });

    QSignalSpy requestFailedSpy(&oauth2, &QAbstractOAuth::requestFailed);
#if QT_DEPRECATED_SINCE(6, 13)
    QSignalSpy errorSpy(&oauth2, &QAbstractOAuth2::error);
#endif
    QSignalSpy serverReportedErrorOccurredSpy(&oauth2,
                                              &QAbstractOAuth2::serverReportedErrorOccurred);
    QSignalSpy statusSpy(&oauth2, &QAbstractOAuth2::statusChanged);
    auto clearSpies = [&](){
        requestFailedSpy.clear();
        serverReportedErrorOccurredSpy.clear();
#if QT_DEPRECATED_SINCE(6, 13)
        errorSpy.clear();
#endif
        statusSpy.clear();
    };

    // Test error response from the authorization server (RFC 6749 section 5.2)
    callbackParameters = {{"error"_L1, "invalid_grant"_L1},
                          {"error_description"_L1, "The error description"_L1},
                          {"error_uri"_L1, "The error URI"_L1}};
    expectWarning();
    oauth2.grant();
#if QT_DEPRECATED_SINCE(6, 13)
    QTRY_COMPARE(errorSpy.count(), 1);
#endif
    QTRY_COMPARE(serverReportedErrorOccurredSpy.count(), 1);
    QTRY_COMPARE(requestFailedSpy.count(), 1);
#if QT_DEPRECATED_SINCE(6, 13)
    QCOMPARE(errorSpy.first().at(0).toString(), "invalid_grant"_L1);
    QCOMPARE(errorSpy.first().at(1).toString(), "The error description"_L1);
    QCOMPARE(errorSpy.first().at(2).toString(), "The error URI"_L1);
#endif
    QCOMPARE(serverReportedErrorOccurredSpy.first().at(0).toString(), "invalid_grant"_L1);
    QCOMPARE(serverReportedErrorOccurredSpy.first().at(1).toString(), "The error description"_L1);
    QCOMPARE(serverReportedErrorOccurredSpy.first().at(2).toString(), "The error URI"_L1);
    QCOMPARE(requestFailedSpy.first().at(0).value<QAbstractOAuth::Error>(),
             QAbstractOAuth::Error::ServerError);
    QVERIFY(statusSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::NotAuthenticated);

    // Test not providing authorization code
    clearSpies();
    callbackParameters = {{"state"_L1, "thestate"_L1}};
    expectWarning();
    oauth2.grant();
    QTRY_COMPARE(requestFailedSpy.count(), 1);
    QCOMPARE(requestFailedSpy.first().at(0).value<QAbstractOAuth::Error>(),
             QAbstractOAuth::Error::OAuthTokenNotFoundError);

#if QT_DEPRECATED_SINCE(6, 13)
    QCOMPARE(errorSpy.count(), 0);
#endif
    QCOMPARE(serverReportedErrorOccurredSpy.count(), 0);
    QVERIFY(statusSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::NotAuthenticated);

    // Test not providing a state
    clearSpies();
    callbackParameters = {{"code"_L1, "thecode"_L1}};
    expectWarning();
    oauth2.grant();
    QTRY_COMPARE(requestFailedSpy.count(), 1);
    QCOMPARE(requestFailedSpy.first().at(0).value<QAbstractOAuth::Error>(),
             QAbstractOAuth::Error::ServerError);
#if QT_DEPRECATED_SINCE(6, 13)
    QCOMPARE(errorSpy.count(), 0);
#endif
    QCOMPARE(serverReportedErrorOccurredSpy.count(), 0);
    QVERIFY(statusSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::NotAuthenticated);

    // Test state mismatch (here we use "thestate" while the actual, expected, state is a
    // random generated string varying each run
    clearSpies();
    callbackParameters = {{"code"_L1, "thecode"_L1}, {"state"_L1, "thestate"_L1}};
    expectWarning();
    oauth2.grant();
    QTRY_COMPARE(requestFailedSpy.count(), 1);
    QCOMPARE(requestFailedSpy.first().at(0).value<QAbstractOAuth::Error>(),
             QAbstractOAuth::Error::ServerError);
#if QT_DEPRECATED_SINCE(6, 13)
    QCOMPARE(errorSpy.count(), 0);
#endif
    QCOMPARE(serverReportedErrorOccurredSpy.count(), 0);
    QVERIFY(statusSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::NotAuthenticated);
}
QT_WARNING_POP

class RequestModifier : public QObject
{
    Q_OBJECT
public:
    RequestModifier(QObject *parent = nullptr) : QObject(parent) {}

    void handleRequestModification(QNetworkRequest &request, QAbstractOAuth::Stage stage)
    {
        stageReceivedByModifier = stage;
        auto headers = request.headers();
        headers.append("test-header-name"_ba, valueToSet);
        request.setHeaders(headers);
    }
    QAbstractOAuth::Stage stageReceivedByModifier =
        QAbstractOAuth::Stage::RequestingTemporaryCredentials;
    QByteArray valueToSet;
};

#define TEST_MODIFY_REQUEST_WITH_MODIFIER(STAGE_RECEIVED, VALUE_SET, VALUE_PREFIX) \
    { \
        server->receivedTokenRequests.clear(); \
        STAGE_RECEIVED = QAbstractOAuth::Stage::RequestingTemporaryCredentials; \
        VALUE_SET = QByteArray(VALUE_PREFIX) + "_access_token"; \
        oauth2.grant(); \
        /* Conclude authorization stage so that we proceed into access token request */ \
        replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}}); \
        QTRY_COMPARE(STAGE_RECEIVED, QAbstractOAuth::Stage::RequestingAccessToken); \
        QTRY_COMPARE(server->receivedTokenRequests.size(), 1); \
        QTRY_COMPARE(server->receivedTokenRequests.at(0).headers.value("test-header-name"_ba), VALUE_SET); \
        QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted); \
        /* Refresh token request */ \
        VALUE_SET = QByteArray(VALUE_PREFIX) + "_refresh_token"; \
        server->receivedTokenRequests.clear(); \
        STAGE_RECEIVED = QAbstractOAuth::Stage::RequestingTemporaryCredentials; \
        oauth2.refreshTokens(); \
        QCOMPARE(oauth2.status(), QAbstractOAuth::Status::RefreshingToken); \
        QTRY_COMPARE(STAGE_RECEIVED, QAbstractOAuth::Stage::RefreshingAccessToken); \
        QTRY_COMPARE(server->receivedTokenRequests.size(), 1); \
        QTRY_COMPARE(server->receivedTokenRequests.at(0).headers.value("test-header-name"_ba), VALUE_SET); \
        QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted); \
        oauth2.clearNetworkRequestModifier(); \
    } \

#define TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(VALUE_SET) \
    { \
        server->receivedTokenRequests.clear(); \
        VALUE_SET = "must_not_be_set"_ba; \
        oauth2.grant(); \
        /* Conclude authorization stage so that we proceed into access token request */ \
        replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}}); \
        QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted); \
        QTRY_COMPARE(server->receivedTokenRequests.size(), 1); \
        QVERIFY(server->receivedTokenRequests.at(0).headers.value("test-header-name"_ba).isEmpty()); \
        server->receivedTokenRequests.clear(); \
        oauth2.refreshTokens(); \
        QCOMPARE(oauth2.status(), QAbstractOAuth::Status::RefreshingToken); \
        QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted); \
        QTRY_COMPARE(server->receivedTokenRequests.size(), 1); \
        QVERIFY(server->receivedTokenRequests.at(0).headers.value("test-header-name"_ba).isEmpty()); \
        oauth2.clearNetworkRequestModifier(); \
    } \

void tst_OAuth2CodeFlow::modifyTokenRequests()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    std::unique_ptr<RequestModifier> context(new RequestModifier);
    QRegularExpression nullContextWarning(u".*Context object must not be null, ignoring"_s);
    QRegularExpression wrongThreadWarning(u".*Context object must reside in the same thread"_s);
    auto valueToSet = ""_ba;

    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    oauth2.setRefreshToken(u"refresh_token"_s);
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());
    oauth2.setState("a_state"_L1);

    QAbstractOAuth::Stage stageReceivedByModifier =
        QAbstractOAuth::Stage::RequestingTemporaryCredentials;
    auto modifierLambda = [&](QNetworkRequest &request, QAbstractOAuth::Stage stage) {
        stageReceivedByModifier = stage;
        auto headers = request.headers();
        headers.append("test-header-name"_ba, valueToSet);
        request.setHeaders(headers);
    };
    std::function<void(QNetworkRequest &, QAbstractOAuth::Stage)> modifierFunc = modifierLambda;

    // Lambda with a context object
    oauth2.setNetworkRequestModifier(context.get(), modifierLambda);
    TEST_MODIFY_REQUEST_WITH_MODIFIER(stageReceivedByModifier, valueToSet, "lambda_with_context")

    // Test that the modifier will be cleared
    oauth2.clearNetworkRequestModifier();
    TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(valueToSet)

    // Lambda without a context object
    QTest::ignoreMessage(QtWarningMsg, nullContextWarning);
    oauth2.setNetworkRequestModifier(nullptr, modifierLambda);
    TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(valueToSet)

    // std::function with a context object
    oauth2.setNetworkRequestModifier(context.get(), modifierFunc);
    TEST_MODIFY_REQUEST_WITH_MODIFIER(stageReceivedByModifier, valueToSet, "func_with_context")

    // PMF with context object
    oauth2.setNetworkRequestModifier(context.get(), &RequestModifier::handleRequestModification);
    TEST_MODIFY_REQUEST_WITH_MODIFIER(context->stageReceivedByModifier,
                                      context->valueToSet, "pmf_with_context")

    // PMF without context object
    QTest::ignoreMessage(QtWarningMsg, nullContextWarning);
    oauth2.setNetworkRequestModifier(nullptr, &RequestModifier::handleRequestModification);
    TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(context->valueToSet)

    // Destroy context object => no callback (or crash)
    oauth2.setNetworkRequestModifier(context.get(), modifierLambda);
    context.reset(nullptr);
    TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(valueToSet)

    // Context object in wrong thread
    QThread thread;
    QObject objectInWrongThread;
    // Initially context object is in correct thread
    oauth2.setNetworkRequestModifier(&objectInWrongThread, modifierLambda);
    // Move to wrong thread, verify we get warnings when it's time to call the callback
    objectInWrongThread.moveToThread(&thread);
    oauth2.grant();
    QTest::ignoreMessage(QtWarningMsg, wrongThreadWarning);
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    // Now the context object is in wrong thread when attempting to set the modifier
    oauth2.clearNetworkRequestModifier();
    QTest::ignoreMessage(QtWarningMsg, wrongThreadWarning);
    oauth2.setNetworkRequestModifier(&objectInWrongThread, modifierLambda);
    TEST_MODIFY_REQUEST_WITHOUT_MODIFIER(valueToSet)

    // These must not compile
    // oauth2.setRequestModifier();
    // oauth2.setRequestModifier(&context, [](const QString& wrongType){});
    // oauth2.setRequestModifier(&context, [](QNetworkRequest &request, int wrongType){});
    // oauth2.setRequestModifier(&context, [](int wrongType, QAbstractOAuth::Stage stage){});
}

void tst_OAuth2CodeFlow::getToken()
{
    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, [&](const QUrl &url) {
        const QUrlQuery query(url.query());
        replyHandler.emitCallbackReceived(QVariantMap {
                                               { QLatin1String("code"), QLatin1String("test") },
                                               { QLatin1String("state"),
                                                 query.queryItemValue(QLatin1String("state")) }
                                           });
    });
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    oauth2.grant();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), "an-access-token"_L1);
}

void tst_OAuth2CodeFlow::refreshToken()
{
    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    QOAuth2AuthorizationCodeFlow oauth2;
#if QT_DEPRECATED_SINCE(6, 13)
    QT_IGNORE_DEPRECATIONS(oauth2.setAccessTokenUrl(server->tokenEndpoint());)
#else
    oauth2.setTokenUrl(server->tokenEndpoint());
#endif
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    oauth2.setRefreshToken(QLatin1String("refresh_token"));
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    oauth2.refreshTokens();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), QLatin1String("an-access-token"));

#if QT_DEPRECATED_SINCE(6, 13)
    // Verify that refreshAccessToken also works
    QT_IGNORE_DEPRECATIONS(oauth2.refreshAccessToken();)
    QTRY_COMPARE(grantedSpy.size(), 2);
#endif
}

void tst_OAuth2CodeFlow::getAndRefreshToken()
{
    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, [&](const QUrl &url) {
        const QUrlQuery query(url.query());
        replyHandler.emitCallbackReceived(QVariantMap {
                                              { QLatin1String("code"), QLatin1String("test") },
                                              { QLatin1String("state"),
                                                query.queryItemValue(QLatin1String("state")) }
                                          });
    });
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    oauth2.grant();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), "an-access-token"_L1);
    grantedSpy.clear();
    server->responses.tokenBody = R"(
            {
                "access_token": "refreshed-access-token"
            })";

    oauth2.refreshTokens();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), QLatin1String("refreshed-access-token"));
}

void tst_OAuth2CodeFlow::tokenRequestErrors()
{
    // This test tests the token acquisition and refreshing errors.
    // Helper to catch the expected warning messages:
    constexpr auto expectWarning = [](){
        static const QRegularExpression tokenWarning{"Token request failed:.*"};
        QTest::ignoreMessage(QtWarningMsg, tokenWarning);
    };

    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);

    QSignalSpy requestFailedSpy(&oauth2, &QAbstractOAuth::requestFailed);
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    QSignalSpy statusSpy(&oauth2, &QAbstractOAuth2::statusChanged);
    auto clearSpies = [&](){
        requestFailedSpy.clear();
        grantedSpy.clear();
        statusSpy.clear();
    };

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &oauth2, [&](const QUrl &url) {
        // Successful authorization stage, after which we can test token requests.
        // For clarity: in these tests we omit browser interaction by directly triggering
        // the emission of replyhandler::callbackReceived() signal
        const QUrlQuery query(url.query());
        replyHandler.emitCallbackReceived(QVariantMap {
            { QLatin1String("code"), QLatin1String("test") },
            { QLatin1String("state"),
             query.queryItemValue(QLatin1String("state")) }
        });
    });

    // Check the initial state
    QVERIFY(requestFailedSpy.isEmpty());
    QVERIFY(grantedSpy.isEmpty());
    QVERIFY(statusSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::NotAuthenticated);

    // Try to get an access token with an invalid response
    server->responses.tokenBody = "an invalid response";
    expectWarning();
    oauth2.grant();
    QTRY_COMPARE(requestFailedSpy.size(), 1);
    QVERIFY(grantedSpy.isEmpty());
    QCOMPARE(statusSpy.size(), 1); // Authorization was successful so we get one signal
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);

    // Try to get an access token, but replyhandler indicates an error
    clearSpies();
    replyHandler.aTokenRequestError = QAbstractOAuth::Error::NetworkError;
    expectWarning();
    oauth2.grant();
    QTRY_COMPARE(requestFailedSpy.size(), 1);
    QVERIFY(grantedSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);

    // Make a successful access & refresh token acquisition
    replyHandler.aTokenRequestError = QAbstractOAuth::Error::NoError;
    clearSpies();
    server->responses.tokenBody = Responses::tokenSuccess;
    oauth2.grant();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(statusSpy.size(), 3);
    // First status change is going from TempCred back to NotAuthenticated
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::NotAuthenticated);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::TemporaryCredentialsReceived);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::Granted);
    QVERIFY(requestFailedSpy.isEmpty());
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QCOMPARE(oauth2.token(), u"an-access-token"_s);
    QCOMPARE(oauth2.refreshToken(), u"a-refresh-token"_s);

    // Successfully refresh access token
    clearSpies();
    oauth2.refreshTokens();
    QTRY_COMPARE(statusSpy.size(), 2);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::RefreshingToken);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::Granted);
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QVERIFY(requestFailedSpy.isEmpty());

    // Failed access token refresh
    clearSpies();
    replyHandler.aTokenRequestError = QAbstractOAuth::Error::ServerError;
    expectWarning();
    oauth2.refreshTokens();
    QTRY_COMPARE(statusSpy.size(), 2);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::RefreshingToken);
    QCOMPARE(statusSpy.takeFirst().at(0).value<QAbstractOAuth::Status>(),
             QAbstractOAuth::Status::Granted); // back to granted since we have an access token
    QCOMPARE(requestFailedSpy.size(), 1);
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
}

using Method = QOAuth2AuthorizationCodeFlow::PkceMethod;

void tst_OAuth2CodeFlow::pkce_data()
{
    QTest::addColumn<Method>("method");
    QTest::addColumn<quint8>("verifierLength");

    QTest::addRow("none") << Method::None << quint8(43);
    QTest::addRow("plain_43") << Method::Plain << quint8(43);
    QTest::addRow("plain_77") << Method::Plain << quint8(77);
    QTest::addRow("S256_43") << Method::S256 << quint8(43);
    QTest::addRow("S256_88") << Method::S256 << quint8(88);
}

void tst_OAuth2CodeFlow::pkce()
{
    QFETCH(Method, method);
    QFETCH(quint8, verifierLength);

    static constexpr auto code_verifier = "code_verifier"_L1;
    static constexpr auto code_challenge = "code_challenge"_L1;
    static constexpr auto code_challenge_method = "code_challenge_method"_L1;

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl("authorization_url"));
    oauth2.setTokenUrl(QUrl("access_token_url"));
    oauth2.setState("a_state"_L1);
    QCOMPARE(oauth2.pkceMethod(), Method::S256); // the default
    oauth2.setPkceMethod(method, verifierLength);
    QCOMPARE(oauth2.pkceMethod(), method);

    QMultiMap<QString, QVariant> tokenRequestParms;
    oauth2.setModifyParametersFunction(
        [&] (QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
            if (stage == QAbstractOAuth::Stage::RequestingAccessToken)
                tokenRequestParms = *parameters;
    });

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    QSignalSpy openBrowserSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser);

    oauth2.grant(); // Initiate authorization

    // 1. Verify the authorization URL query parameters
    QTRY_VERIFY(!openBrowserSpy.isEmpty());
    auto authParms = QUrlQuery{openBrowserSpy.takeFirst().at(0).toUrl()};
    QVERIFY(!authParms.hasQueryItem(code_verifier));
    const auto codeChallenge = authParms.queryItemValue(code_challenge).toLatin1();
    if (method == Method::None) {
        QVERIFY(!authParms.hasQueryItem(code_challenge));
        QVERIFY(!authParms.hasQueryItem(code_challenge_method));
    } else if (method == Method::Plain) {
        QCOMPARE(codeChallenge.size(), verifierLength); // With plain the challenge == verifier
        QCOMPARE(authParms.queryItemValue(code_challenge_method), "plain"_L1);
    } else { // S256
        QCOMPARE(codeChallenge.size(), 43); // SHA-256 is 32 bytes, and that in base64 is ~43 bytes
        QCOMPARE(authParms.queryItemValue(code_challenge_method), "S256"_L1);
    }

    // Conclude authorization => starts access token request
    emit replyHandler.callbackReceived({{"code", "acode"}, {"state", "a_state"}});

    // 2. Verify the access token request parameters
    QTRY_VERIFY(!tokenRequestParms.isEmpty());
    QVERIFY(!tokenRequestParms.contains(code_challenge));
    QVERIFY(!tokenRequestParms.contains(code_challenge_method));
    // Verify the challenge received earlier was based on the verifier we receive here
    if (method == Method::None) {
        QVERIFY(!tokenRequestParms.contains(code_verifier));
    } else if (method == Method::Plain) {
        QVERIFY(tokenRequestParms.contains(code_verifier));
        QCOMPARE(tokenRequestParms.value(code_verifier).toByteArray(), codeChallenge);
    } else { // S256
        QVERIFY(tokenRequestParms.contains(code_verifier));
        const auto codeVerifier = tokenRequestParms.value(code_verifier).toByteArray();
        QCOMPARE(codeVerifier.size(), verifierLength);
        QCOMPARE(QCryptographicHash::hash(codeVerifier, QCryptographicHash::Algorithm::Sha256)
                 .toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals)
                 , codeChallenge);
    }
}

void tst_OAuth2CodeFlow::nonce()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    const auto nonce = "a_nonce"_ba;

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});

    QByteArray nonceInAuthorizationUrl;
    connect(&oauth2, &QAbstractOAuth::authorizeWithBrowser, this, [&](const QUrl &url){
        QUrlQuery parameters(url);
        nonceInAuthorizationUrl = parameters.queryItemValue(u"nonce"_s).toUtf8();
    });

    // Verify that nonce is set to authorization request when appropriate
    oauth2.setNonce(nonce);
    oauth2.setRequestedScopeTokens({"scope_item1"});

    // -- Nonce is always included
    oauth2.setNonceMode(QAbstractOAuth2::NonceMode::Enabled);
    oauth2.grant();
    QCOMPARE(nonceInAuthorizationUrl, nonce);

    // -- Nonce is never included
    oauth2.setNonceMode(QAbstractOAuth2::NonceMode::Disabled);
    oauth2.grant();
    QVERIFY(nonceInAuthorizationUrl.isEmpty());

    // -- Nonce is included if scope contains 'openid'
    oauth2.setNonceMode(QAbstractOAuth2::NonceMode::Automatic);
    oauth2.grant();
    QVERIFY(nonceInAuthorizationUrl.isEmpty());

    oauth2.setRequestedScopeTokens({"scope_item1", "openid"});
    oauth2.grant();
    QCOMPARE(nonceInAuthorizationUrl, nonce);

    // -- Clear nonce, one should be generated
    oauth2.setNonce("");
    QVERIFY(oauth2.nonce().isEmpty());
    oauth2.grant();
    QVERIFY(!oauth2.nonce().isEmpty());
    QCOMPARE(nonceInAuthorizationUrl, oauth2.nonce());
}

void tst_OAuth2CodeFlow::idToken()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setRequestedScopeTokens({"openid"});
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    oauth2.setState("a_state"_L1);
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    QSignalSpy idTokenSpy(&oauth2, &QAbstractOAuth2::idTokenChanged);
    QSignalSpy requestFailedSpy(&oauth2, &QAbstractOAuth::requestFailed);

    // Verify default token is empty
    QVERIFY(oauth2.idToken().isEmpty());

    // Test without openid and verify idToken doesn't change
    oauth2.setRequestedScopeTokens({"read"});
    oauth2.grant();
    // Conclude authorization stage in order to proceed to access token stage
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    // Conclude access token stage, during which the id token is (would be) provided
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QVERIFY(idTokenSpy.isEmpty());
    QVERIFY(oauth2.idToken().isEmpty());

    // Test granted openid scope when none was requested (QTBUG-145561), first test:
    // - 'openid' scope not requested by client but
    // - 'openid' scope is granted by server and
    // - 'id_token' not provided by server
    oauth2.setRequestedScopeTokens({"read"});
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"scope"_L1, "read openid"_L1}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QVERIFY(idTokenSpy.isEmpty());
    QVERIFY(oauth2.idToken().isEmpty());
    // - 'openid' scope not requested by client but
    // - 'openid' scope is granted by server and
    // - 'id_token' is provided by server
    oauth2.setRequestedScopeTokens({"read"});
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"scope"_L1, "read openid"_L1},
                                     {"id_token"_L1, "unsolicited_id_token"_L1}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QVERIFY(!idTokenSpy.isEmpty());
    QCOMPARE(oauth2.idToken(), "unsolicited_id_token"_L1);
    idTokenSpy.clear();

    // Test with openid
    // Note: using a proper JWT or setting the matching 'nonce' is not required for this tests
    // purpose as we don't currently validate the received token, but no harm in being thorough
    auto idToken = createSignedJWT({}, {{"nonce"_L1, oauth2.nonce()}});
    oauth2.setRequestedScopeTokens({"openid"});
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"id_token"_L1, idToken}});
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QCOMPARE(oauth2.idToken(), idToken);
    QCOMPARE(idTokenSpy.size(), 1);
    QCOMPARE(idTokenSpy.at(0).at(0).toByteArray(), idToken);
    idTokenSpy.clear();

    // Providing an id_token is not mandatory when refreshing tokens. Test first with the id_token
    oauth2.setRefreshToken("refresh_token"_L1);
    oauth2.refreshTokens();
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::RefreshingToken);
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"id_token"_L1, "fresh_id"}});
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QCOMPARE(idTokenSpy.size(), 1);
    QCOMPARE(oauth2.idToken(), "fresh_id"_L1);
    // Test without providing id_token (also must not clear the pre-existing id_token)
    oauth2.refreshTokens();
    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::RefreshingToken);
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QCOMPARE(idTokenSpy.size(), 1);
    QCOMPARE(oauth2.idToken(), "fresh_id"_L1);

    // Test missing id_token error
    QVERIFY(requestFailedSpy.isEmpty());
    const QRegularExpression tokenWarning{
        "Token request failed: \"'openid' scope requested and granted but ID token not received\""};
    QTest::ignoreMessage(QtWarningMsg, tokenWarning);
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QTRY_COMPARE(requestFailedSpy.size(), 1);
    QCOMPARE(requestFailedSpy.at(0).at(0).value<QAbstractOAuth::Error>(),
             QAbstractOAuth::Error::OAuthTokenNotFoundError);
    QCOMPARE(oauth2.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);
    // idToken is cleared on failure
    QCOMPARE(idTokenSpy.size(), 2);
    QVERIFY(oauth2.idToken().isEmpty());
}

#ifndef QOAUTH2_NO_LEGACY_SCOPE
QT_WARNING_PUSH QT_WARNING_DISABLE_DEPRECATED
void tst_OAuth2CodeFlow::scope_data()
{
    static const auto requestedScopeTokens = u"requested"_s;
    QTest::addColumn<QString>("scope");
    QTest::addColumn<QString>("granted_scope");
    QTest::addColumn<QString>("expected_scope");

    QTest::addRow("scope_returned")
        << requestedScopeTokens << requestedScopeTokens << requestedScopeTokens;
    QTest::addRow("differing_scope_returned")
        << requestedScopeTokens << u"granted"_s << u"granted"_s;
    QTest::addRow("empty_scope_returned")
        << requestedScopeTokens << u""_s << requestedScopeTokens;
}

void tst_OAuth2CodeFlow::scope()
{
    QFETCH(QString, scope);
    QFETCH(QString, granted_scope);
    QFETCH(QString, expected_scope);

    QOAuth2AuthorizationCodeFlow oauth2;
    QVERIFY(oauth2.scope().isEmpty());

    // Set the requested scope and verify it changes
    QSignalSpy scopeSpy(&oauth2, &QAbstractOAuth2::scopeChanged);
    oauth2.setScope(scope);
    QCOMPARE(scopeSpy.size(), 1);
    QCOMPARE(oauth2.scope(), scope);
    QCOMPARE(scopeSpy.at(0).at(0).toString(), scope);

    // Verify that empty authorization server 'scope' response doesn't overwrite the
    // requested scope, whereas a returned scope value does
    auto server = createAuthorizationServer<WebServer>(Responses::defaults());
    if (!granted_scope.isEmpty()) {
        server->responses.tokenBody =  R"(
            {
                "access_token": "an-access-token",
                "refresh_token": "a-refresh-token",
                "token_type": "bearer",
                "expires_in": 3600,
                "scope": ")" + granted_scope.toLatin1() +
            "\"}";
    }
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
#if QT_DEPRECATED_SINCE(6, 13)
    QT_IGNORE_DEPRECATIONS(oauth2.setAccessTokenUrl(server->tokenEndpoint());)
#else
    oauth2.setTokenUrl(server->tokenEndpoint());
#endif
    oauth2.setState("a_state"_L1);
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, [&](const QUrl &) {
                replyHandler.emitCallbackReceived(QVariantMap {
                    { "code"_L1, "a_code"_L1 }, { "state"_L1, "a_state"_L1 },
        });
    });
    oauth2.grant();

    QTRY_COMPARE(oauth2.status(), QAbstractOAuth::Status::Granted);
    QCOMPARE(oauth2.scope(), expected_scope);
    if (!granted_scope.isEmpty() && (granted_scope != scope)) {
        QCOMPARE(scopeSpy.size(), 2);
        QCOMPARE(scopeSpy.at(1).at(0).toString(), expected_scope);
    } else {
        QCOMPARE(scopeSpy.size(), 1);
    }
}
QT_WARNING_POP
#endif // QOAUTH2_NO_LEGACY_SCOPE

void tst_OAuth2CodeFlow::requestedScopeTokens_data()
{
    const QByteArray f = "first";
    const QByteArray s = "second";

    QTest::addColumn<QSet<QByteArray>>("requested_scope");
    QTest::addColumn<QSet<QByteArray>>("expected_requested_scope");

    QTest::addRow("singlescope") << QSet{f} << QSet{f};
    QTest::addRow("multiscope")  << QSet{f, s} << QSet{f, s};
}

void tst_OAuth2CodeFlow::requestedScopeTokens()
{
    QFETCH(QSet<QByteArray>, requested_scope);
    QFETCH(QSet<QByteArray>, expected_requested_scope);

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    QVERIFY(oauth2.requestedScopeTokens().isEmpty());

    QSignalSpy requestedScopeTokensSpy(&oauth2, &QAbstractOAuth2::requestedScopeTokensChanged);
    QString resultingRequestScope;
    QObject::connect(&oauth2, &QAbstractOAuth2::authorizeWithBrowser, this,
                     [&resultingRequestScope](const QUrl &url) {
                         QUrlQuery queryParameters(url);
                         resultingRequestScope = queryParameters.queryItemValue(u"scope"_s);
                     });

    oauth2.setRequestedScopeTokens(requested_scope);

    QCOMPARE(requestedScopeTokensSpy.size(), 1);
    QCOMPARE(oauth2.requestedScopeTokens(), expected_requested_scope);
    QCOMPARE(requestedScopeTokensSpy.at(0).at(0).value<QSet<QByteArray>>(),
             expected_requested_scope);

    oauth2.grant();
    const auto scopeTokens = resultingRequestScope.toLatin1().split(' ');
    QCOMPARE_EQ(scopeTokens.size(), requested_scope.size());
    for (const auto &token : scopeTokens)
        QVERIFY2(requested_scope.contains(token), token.data());
}

void tst_OAuth2CodeFlow::grantedScopeTokens_data()
{
    const QSet<QByteArray> requestedScopeTokens = {"first", "second"};
    const QByteArray scope = "first second";
    const QByteArray granted1 = "granted1";
    const QByteArray granted2 = "granted2";
    const QByteArray grantedJoined = granted1 + " " + granted2;
    const QSet<QByteArray> grantedList = {granted1, granted2};

    QTest::addColumn<QSet<QByteArray>>("requested_scope");
    QTest::addColumn<QByteArray>("granted_scope");
    QTest::addColumn<QSet<QByteArray>>("expected_granted_scope");

    QTest::addRow("requested_scope_returned")
        << requestedScopeTokens << scope << requestedScopeTokens;

    QTest::addRow("differing_singlescope_returned")
        << requestedScopeTokens << granted1 << QSet{granted1};

    QTest::addRow("differing_multiscope_returned")
        << requestedScopeTokens << grantedJoined << grantedList;

    QTest::addRow("empty_scope_returned")
        << requestedScopeTokens << ""_ba << requestedScopeTokens;
}

void tst_OAuth2CodeFlow::grantedScopeTokens()
{
    QFETCH(QSet<QByteArray>, requested_scope);
    QFETCH(QByteArray, granted_scope);
    QFETCH(QSet<QByteArray>, expected_granted_scope);

    QOAuth2AuthorizationCodeFlow oauth2;
    QSignalSpy grantedSpy(&oauth2, &QAbstractOAuth2::grantedScopeTokensChanged);
    oauth2.setRequestedScopeTokens(requested_scope);
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    oauth2.setState("a_state"_L1);
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);

    oauth2.grant();
    // Conclude authorization stage in order to proceed to access token stage
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});

    QVariantMap accessTokenResponseParameters;
    if (granted_scope.isEmpty())
        accessTokenResponseParameters = {{"access_token"_L1, "at"_L1}};
    else
        accessTokenResponseParameters = {{"access_token"_L1, "at"_L1}, {"scope"_L1, granted_scope}};
    // Conclude access token stage, during which the granted scope is provided
    replyHandler.emitTokensReceived(accessTokenResponseParameters);

    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.grantedScopeTokens(), expected_granted_scope);
    QCOMPARE(grantedSpy.at(0).at(0).value<QSet<QByteArray>>(), expected_granted_scope);
}

void tst_OAuth2CodeFlow::refreshLeadTime_data()
{
    QTest::addColumn<std::chrono::seconds>("refreshLeadTime");
    QTest::addColumn<int>("expiresIn");
    QTest::addColumn<std::chrono::seconds>("waitTimeForExpiration");
    QTest::addColumn<bool>("autoRefresh");
    QTest::addColumn<bool>("expectExpirationSignal");
    QTest::addColumn<QString>("refreshToken");
    QTest::addColumn<bool>("expectRefreshRequest");

    const QString refreshToken = u"refreshToken"_s;

    // wait-time: 20s - 18s = 2s, + 1s for robustness => 3s
    QTest::addRow("validSetExpiration")
            << 18s << 20 << 3s << true  << true  << refreshToken << true;

    // wait-time calculation: 15s - 10s = 5s, + 1s for robustness => 6s
    QTest::addRow("validCalculatedExpiration")
            << 0s  << 15 << 6s << true  << true  << refreshToken << true;

    // wait-time: 5s - 10s = -5s => 2s minimum + 1s for robustness => 3s
    QTest::addRow("tooShortCalculatedExpiration")
            << 0s  << 5 << 3s  << true  << true  << refreshToken << true;

    // wait-time: 5s - 10s = -5s => 2s minimum + 1s for robustness => 3s
    QTest::addRow("tooShortSetExpiration")
            << 10s  << 5 << 3s << true  << true  << refreshToken << true;

    // wait-time: 3s - 1s = 2s, +1s for robustness => 3s
    QTest::addRow("leadTimeNearExpiration")
            << 1s  << 3 << 3s  << true  << true << refreshToken << true;

    QTest::addRow("invalidExpirationTime")
            << 1s  << 0 << 3s  << true  << false << refreshToken << false;

    // wait-time: 2s - 1s = 1s, => minimum 2s + 1s for robustness => 2s
    QTest::addRow("autoRefreshDisabled")
            << 1s  << 2 << 3s  << false << true  << refreshToken << false;

    QTest::addRow("emptyRefreshToken")
            << 18s << 20 << 3s << true  << true << QString() << false;
}

void tst_OAuth2CodeFlow::refreshLeadTime()
{
    QFETCH(std::chrono::seconds, refreshLeadTime);
    QFETCH(int, expiresIn);
    QFETCH(std::chrono::seconds, waitTimeForExpiration);
    QFETCH(bool, autoRefresh);
    QFETCH(bool, expectExpirationSignal);
    QFETCH(QString, refreshToken);
    QFETCH(bool, expectRefreshRequest);

    auto server = createAuthorizationServer<WebServer>(Responses::defaults());

    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());
    oauth2.setState("s"_L1);
    oauth2.setRefreshLeadTime(refreshLeadTime);
    oauth2.setAutoRefresh(autoRefresh);

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);

    QSignalSpy expiredSpy(&oauth2, &QAbstractOAuth2::accessTokenAboutToExpire);
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    server->responses.tokenBody = R"(
        {
            "access_token": "initial-access-token",
            "token_type": "bearer",
            "expires_in": ")" + QByteArray::number(expiresIn) + R"(",
            "scope": "s",
            "refresh_token": ")" + refreshToken.toLatin1() +
        "\"}";

    oauth2.grant();
    replyHandler.emitCallbackReceived(QVariantMap {{ "code"_L1, "c"_L1 }, { "state"_L1, "s"_L1 }});
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), u"initial-access-token"_s);

    if (expectExpirationSignal) {
        server->responses.tokenBody = R"(
            {
                "access_token": "refreshed-access-token",
                "token_type": "bearer",
                "expires_in": 3600,
                "scope": "s",
                "refresh_token": "a-refresh-token"
            })";
        QTRY_COMPARE_WITH_TIMEOUT(expiredSpy.size(), 1, waitTimeForExpiration);
        if (expectRefreshRequest) {
            QTRY_COMPARE(oauth2.token(), "refreshed-access-token"_L1);
            QCOMPARE(expiredSpy.size(), 1);
        }
    }
}

void tst_OAuth2CodeFlow::alreadyExpiredTokenClientSideRefresh()
{
    // This tests a particular corner-case where user adjusts leadTime such
    // that the pre-existing token is updated immediately
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl("authorizationEndpoint"_L1));
    oauth2.setTokenUrl(QUrl("accessTokenEndpoint"_L1));
    oauth2.setState("s"_L1);

    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);

    QSignalSpy expiredSpy(&oauth2, &QAbstractOAuth2::accessTokenAboutToExpire);
    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    oauth2.grant();
    replyHandler.emitCallbackReceived(QVariantMap {{ "code"_L1, "c"_L1 }, { "state"_L1, "s"_L1 }});
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"scope"_L1, "scope"},
                                     {"refresh_token"_L1, "refreshToken"_L1},
                                     {"expires_in"_L1, 3}});
    QTRY_COMPARE(grantedSpy.size(), 1);
    // Triggers an immediate expiration because from leadTime point-of-view the
    // token is either expired or should've expired already
    expiredSpy.clear();
    oauth2.setRefreshLeadTime(10s);
    QCOMPARE(expiredSpy.size(), 1);
}

#ifndef QT_NO_SSL
void tst_OAuth2CodeFlow::tlsAuthentication()
{
    if (!QSslSocket::supportsSsl())
        QSKIP("This test will fail because the backend does not support TLS");

    auto rollback = useTemporaryKeychain();

    // erros may vary, depending on backend
    const QSet<QSslError::SslError> expectedErrors{ QSslError::SelfSignedCertificate,
                                                    QSslError::CertificateUntrusted,
                                                    QSslError::HostNameMismatch };
    auto serverConfig = createSslConfiguration(testDataDir + "certs/selfsigned-server.key",
                                               testDataDir + "certs/selfsigned-server.crt");
    auto server = createAuthorizationServer<TlsWebServer>(Responses::defaults(), serverConfig);
    server->server->setExpectedSslErrors(expectedErrors);

    auto clientConfig = createSslConfiguration(testDataDir + "certs/selfsigned-client.key",
                                               testDataDir + "certs/selfsigned-client.crt");
    QNetworkAccessManager nam;
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setNetworkAccessManager(&nam);
    oauth2.setSslConfiguration(clientConfig);
    oauth2.setAuthorizationUrl(server->authorizationEndpoint());
    oauth2.setTokenUrl(server->tokenEndpoint());
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, [&](const QUrl &url) {
        const QUrlQuery query(url.query());
        replyHandler.emitCallbackReceived(QVariantMap {
                                               { QLatin1String("code"), QLatin1String("test") },
                                               { QLatin1String("state"),
                                                 query.queryItemValue(QLatin1String("state")) }
                                           });
    });
    connect(&nam, &QNetworkAccessManager::sslErrors, this,
        [&expectedErrors](QNetworkReply *r, const QList<QSslError> &errors) {
            // On some Windows machines the test might report all
            // three expected errors.
            QCOMPARE_GE(errors.size(), 2);
            QCOMPARE_LE(errors.size(), 3);
            for (const auto &err : errors)
                QVERIFY(expectedErrors.contains(err.error()));
            r->ignoreSslErrors();
        });

    QSignalSpy grantedSpy(&oauth2, &QOAuth2AuthorizationCodeFlow::granted);
    oauth2.grant();
    QTRY_COMPARE(grantedSpy.size(), 1);
    QCOMPARE(oauth2.token(), "an-access-token"_L1);
}
#endif // !QT_NO_SSL

void tst_OAuth2CodeFlow::extraTokens()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorizationUrl"_L1});
    oauth2.setTokenUrl(QUrl{"accessTokenUrl"_L1});
    oauth2.setState("a_state"_L1);
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    QSignalSpy extraTokensSpy(&oauth2, &QAbstractOAuth::extraTokensChanged);
    QVERIFY(oauth2.extraTokens().isEmpty());

    constexpr auto name1 = "name1"_L1;
    constexpr auto value1 = "value1"_L1;
    constexpr auto name2 = "name2"_L1;
    constexpr auto value2 = "value2"_L1;

    // Conclude authorization stage without extra tokens
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    QCOMPARE(extraTokensSpy.size(), 0);

    // Conclude authorization stage with extra tokens
    extraTokensSpy.clear();
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1},
                                       {name1, value1}});
    QTRY_COMPARE(extraTokensSpy.size(), 1);
    QVariantMap extraTokens = oauth2.extraTokens();
    QCOMPARE(extraTokens, extraTokensSpy.at(0).at(0).toMap());
    QCOMPARE(extraTokens.size(), 1);
    QCOMPARE(extraTokens.value(name1).toString(), value1);

    // Conclude token stage without additional extra tokens
    extraTokensSpy.clear();
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QCOMPARE(extraTokensSpy.size(), 0);
    extraTokens = oauth2.extraTokens();
    QCOMPARE(extraTokens.size(), 1);
    QCOMPARE(extraTokens.value(name1).toString(), value1);

    // Conclude token stage with additional extra tokens
    extraTokensSpy.clear();
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {name2, value2}});
    QTRY_COMPARE(extraTokensSpy.size(), 1);
    extraTokens = oauth2.extraTokens();
    QCOMPARE(extraTokens, extraTokensSpy.at(0).at(0).toMap());
    QCOMPARE(extraTokens.size(), 2);
    QCOMPARE(extraTokens.value(name1).toString(), value1);
    QCOMPARE(extraTokens.value(name2).toString(), value2);
}

void tst_OAuth2CodeFlow::expirationAt()
{
    QOAuth2AuthorizationCodeFlow oauth2;
    oauth2.setAuthorizationUrl(QUrl{"authorizationEndpoint"_L1});
    oauth2.setTokenUrl(QUrl{"tokenEndpoint"_L1});
    oauth2.setState("a_state"_L1);
    ReplyHandler replyHandler;
    oauth2.setReplyHandler(&replyHandler);
    QSignalSpy expirationAtSpy(&oauth2, &QAbstractOAuth2::expirationAtChanged);

    const auto expiresAtIsInSecondsFromNow = [&](int fromNow) -> bool {
        // For test robustness check that the time is within +/- 2 seconds
        return qAbs(
            oauth2.expirationAt().secsTo(QDateTime::currentDateTime().addSecs(fromNow))) <= 2;
    };
    // Initial value
    QVERIFY(!oauth2.expirationAt().isValid());

    // Conclude authorization stage
    oauth2.grant();
    replyHandler.emitCallbackReceived({{"code"_L1, "acode"_L1}, {"state"_L1, "a_state"_L1}});
    QVERIFY(expirationAtSpy.isEmpty());

    // Test expiration in 50 seconds from now
    int expires_in = 50;
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", expires_in}});
    QCOMPARE(expirationAtSpy.size(), 1);
    QCOMPARE(expirationAtSpy.at(0).at(0).toDateTime(), oauth2.expirationAt());
    QVERIFY(expiresAtIsInSecondsFromNow(expires_in));
    expirationAtSpy.clear();

    // Changes to 100 seconds from now
    expires_in = 100;
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", expires_in}});
    QCOMPARE(expirationAtSpy.size(), 1);
    QCOMPARE(expirationAtSpy.at(0).at(0).toDateTime(), oauth2.expirationAt());
    QVERIFY(expiresAtIsInSecondsFromNow(expires_in));
    expirationAtSpy.clear();

    // Zero expires_in value, expiresAt should become invalid
    expires_in = 0;
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", expires_in}});
    QCOMPARE(expirationAtSpy.size(), 1);
    QCOMPARE(expirationAtSpy.at(0).at(0).toDateTime(), oauth2.expirationAt());
    QVERIFY(!oauth2.expirationAt().isValid());
    expirationAtSpy.clear();

    // Negative expires_in value, expiresAt should remain invalid
    expires_in = -10;
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", expires_in}});
    QCOMPARE(expirationAtSpy.size(), 0);
    QVERIFY(!oauth2.expirationAt().isValid());
    expirationAtSpy.clear();

    // Non-number expires_in value, expiresAt should remain invalid
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", "garbage"}});
    QCOMPARE(expirationAtSpy.size(), 0);
    QVERIFY(!oauth2.expirationAt().isValid());
    expirationAtSpy.clear();

    // Expiration goes back to valid
    expires_in = 70;
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}, {"expires_in", expires_in}});
    QCOMPARE(expirationAtSpy.size(), 1);
    QCOMPARE(expirationAtSpy.at(0).at(0).toDateTime(), oauth2.expirationAt());
    QVERIFY(expiresAtIsInSecondsFromNow(expires_in));
    expirationAtSpy.clear();

    // Expiration is not provided, expiresAt should become invalid
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QCOMPARE(expirationAtSpy.size(), 1);
    QCOMPARE(expirationAtSpy.at(0).at(0).toDateTime(), oauth2.expirationAt());
    QVERIFY(!oauth2.expirationAt().isValid());
    expirationAtSpy.clear();

    // Expiration is still not provided, expiresAt should remain unchanged
    replyHandler.emitTokensReceived({{"access_token"_L1, "at"_L1}});
    QCOMPARE(expirationAtSpy.size(), 0);
    QVERIFY(!oauth2.expirationAt().isValid());
}

QTEST_MAIN(tst_OAuth2CodeFlow)
#include "tst_oauth2codeflow.moc"
