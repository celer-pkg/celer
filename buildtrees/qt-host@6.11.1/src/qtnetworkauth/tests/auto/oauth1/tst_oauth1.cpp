// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore>
#include <QtTest>
#include <QtNetwork>
#include <QHostInfo>

#include <QtNetworkAuth/qoauth1.h>
#include <QtNetworkAuth/qoauth1signature.h>

#include <private/qoauth1_p.h>

#include "webserver.h"

Q_DECLARE_METATYPE(QNetworkAccessManager::Operation)
Q_DECLARE_METATYPE(QAbstractOAuth::Error)

// TODO: Test PUT and DELETE operations.
// TODO: Write tests to test errors.
// TODO: Remove common event loop

typedef QSharedPointer<QNetworkReply> QNetworkReplyPtr;

class tst_OAuth1 : public QObject
{
    Q_OBJECT

    using StringPair = std::pair<QString, QString>;

    QEventLoop *loop = nullptr;
    enum RunSimpleRequestReturn { Timeout = 0, Success, Failure };
    int returnCode;

    using QObject::connect;
    static bool connect(const QNetworkReplyPtr &ptr,
                        const char *signal,
                        const QObject *receiver,
                        const char *slot,
                        Qt::ConnectionType ct = Qt::AutoConnection)
    {
        return connect(ptr.data(), signal, receiver, slot, ct);
    }
    bool connect(const QNetworkReplyPtr &ptr,
                 const char *signal,
                 const char *slot,
                 Qt::ConnectionType ct = Qt::AutoConnection)
    {
        return connect(ptr.data(), signal, slot, ct);
    }

public:
    int waitForFinish(QNetworkReplyPtr &reply);
    void fillParameters(QVariantMap *parameters, const QUrlQuery &query);

    template<class Type>
    struct PropertyTester
    {
        typedef Type InnerType;
        typedef void(QOAuth1::*ConstSignalType)(const Type &);
        typedef void(QOAuth1::*SignalType)(Type);
        typedef QList<std::function<void(Type *, QOAuth1 *object)>> SetterFunctions;

    private:
        // Each entry in setters should set its first parameter to an expected value
        // and act on its second, a QOAuth1 object, to trigger signal; this
        // function shall check that signal is passed the value the setter previously
        // told us to expect.
        template<class SignalType>
        static void runImpl(SignalType signal, const SetterFunctions &setters)
        {
            QOAuth1 obj;
            Type expectedValue;
            QSignalSpy spy(&obj, signal);
            connect(&obj, signal, &obj, [&](const Type &value) {
                QCOMPARE(expectedValue, value);
            });
            for (const auto &setter : setters) {
                const auto previous = expectedValue;
                setter(&expectedValue, &obj);
                QVERIFY(previous != expectedValue); // To check if the value was modified
            }
            QCOMPARE(spy.size(), setters.size()); // The signal should be emitted
        }

    public:

        static void run(ConstSignalType signal, const SetterFunctions &setters)
        {
            runImpl(signal, setters);
        }

        static void run(SignalType signal, const SetterFunctions &setters)
        {
            runImpl(signal, setters);
        }
    };

    QMultiMap<QString, QVariant> parseAuthorizationString(const QString &string)
    {
        const QString prefix = QStringLiteral("OAuth ");
        QMultiMap<QString, QVariant> ret;
        Q_ASSERT(string.startsWith(prefix));
        QRegularExpression rx("(?<key>.[^=]*)=\"(?<value>.[^\"]*)\",?");
        auto globalMatch = rx.globalMatch(string, prefix.size());
        while (globalMatch.hasNext()) {
            const auto match = globalMatch.next();
            auto key = match.captured("key");
            QString value = match.captured("value");
            value = QString::fromUtf8(QByteArray::fromPercentEncoding(value.toUtf8()));
            ret.insert(key, value);
        }
        return ret;
    }

public Q_SLOTS:
    void finished();
    void gotError();

private Q_SLOTS:
    void clientIdentifierSignal();
    void clientSharedSecretSignal();
    void tokenSignal();
    void tokenSecretSignal();
    void temporaryCredentialsUrlSignal();
    void temporaryTokenCredentialsUrlSignal();
    void tokenCredentialsUrlSignal();
    void signatureMethodSignal();

    void getToken_data();
    void getToken();

    void prepareRequestSignature_data();
    void prepareRequestSignature();

    void grant_data();
    void grant();

    void authenticatedCalls_data();
    void authenticatedCalls();

    void prepareRequestCalls_data();
    void prepareRequestCalls();

    void secondTemporaryToken();
};

const auto oauthVersion = QStringLiteral("oauth_version");
const auto oauthConsumerKey = QStringLiteral("oauth_consumer_key");
const auto oauthNonce = QStringLiteral("oauth_nonce");
const auto oauthSignatureMethod = QStringLiteral("oauth_signature_method");
const auto oauthTimestamp = QStringLiteral("oauth_timestamp");
const auto oauthToken = QStringLiteral("oauth_token");
const auto oauthSignature = QStringLiteral("oauth_signature");

bool hostReachable(const QLatin1String &host)
{
    // http://term.ie/oauth/example testing backend is no longer available.
    // For the time being return false, so that tests relying on it skip rather than
    // fail trying. QTBUG-131869
    return false;

    // check host exists
    QHostInfo hostInfo = QHostInfo::fromName(host);
    if (hostInfo.error() != QHostInfo::NoError)
        return false;

    // try to connect to host
    QTcpSocket socket;
    socket.connectToHost(host, 80);
    if (!socket.waitForConnected(1000))
        return false;

    return true;
}

int tst_OAuth1::waitForFinish(QNetworkReplyPtr &reply)
{
    int count = 0;

    connect(reply, SIGNAL(finished()), SLOT(finished()));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(gotError()));
    returnCode = Success;
    loop = new QEventLoop;
    QSignalSpy spy(reply.data(), SIGNAL(downloadProgress(qint64,qint64)));
    while (!reply->isFinished()) {
        QTimer::singleShot(5000, loop, SLOT(quit()));
        if (loop->exec() == Timeout && count == spy.size() && !reply->isFinished()) {
            returnCode = Timeout;
            break;
        }
        count = spy.size();
    }
    delete loop;
    loop = nullptr;

    return returnCode;
}

void tst_OAuth1::fillParameters(QVariantMap *parameters, const QUrlQuery &query)
{
    const auto list = query.queryItems();
    for (auto it = list.begin(), end = list.end(); it != end; ++it)
        parameters->insert(it->first, it->second);
}

void tst_OAuth1::finished()
{
    if (loop)
        loop->exit(returnCode = Success);
}

void tst_OAuth1::gotError()
{
    if (loop)
        loop->exit(returnCode = Failure);
    disconnect(QObject::sender(), SIGNAL(finished()), this, nullptr);
}

void tst_OAuth1::clientIdentifierSignal()
{
    using PropertyTester = PropertyTester<QString>;
    PropertyTester::SetterFunctions setters {
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setClientIdentifier";
            object->setClientIdentifier(*expectedValue);
        },
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setClientCredentials";
            object->setClientCredentials(std::make_pair(*expectedValue, QString()));
        }
    };
    PropertyTester::run(&QOAuth1::clientIdentifierChanged, setters);
}

void tst_OAuth1::clientSharedSecretSignal()
{
    using PropertyTester = PropertyTester<QString>;
    PropertyTester::SetterFunctions setters {
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setClientSharedSecret";
            object->setClientSharedSecret(*expectedValue);
        },
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setClientCredentials";
            object->setClientCredentials(std::make_pair(QString(), *expectedValue));
        }
    };
    PropertyTester::run(&QOAuth1::clientSharedSecretChanged, setters);
}

void tst_OAuth1::tokenSignal()
{
    using PropertyTester = PropertyTester<QString>;
    PropertyTester::SetterFunctions setters {
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setToken";
            object->setToken(*expectedValue);
        },
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setTokenCredentials";
            object->setTokenCredentials(std::make_pair(*expectedValue, QString()));
        }
    };
    PropertyTester::run(&QOAuth1::tokenChanged, setters);
}

void tst_OAuth1::tokenSecretSignal()
{
    using PropertyTester = PropertyTester<QString>;
    PropertyTester::SetterFunctions setters {
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setTokenSecret";
            object->setTokenSecret(*expectedValue);
        },
        [](QString *expectedValue, QOAuth1 *object) {
            *expectedValue = "setTokenCredentials";
            object->setTokenCredentials(std::make_pair(QString(), *expectedValue));
        }
    };
    PropertyTester::run(&QOAuth1::tokenSecretChanged, setters);
}

void tst_OAuth1::temporaryCredentialsUrlSignal()
{
    using PropertyTester = PropertyTester<QUrl>;
    PropertyTester::SetterFunctions setters {
        [](QUrl *expectedValue, QOAuth1 *object) {
            *expectedValue = QUrl("http://example.net/");
            object->setTemporaryCredentialsUrl(*expectedValue);
        }
    };
    PropertyTester::run(&QOAuth1::temporaryCredentialsUrlChanged, setters);
}

void tst_OAuth1::temporaryTokenCredentialsUrlSignal()
{
    using PropertyTester = PropertyTester<QUrl>;
    PropertyTester::SetterFunctions setters {
        [](QUrl *expectedValue, QOAuth1 *object) {
            *expectedValue = QUrl("http://example.net/");
            object->setTemporaryCredentialsUrl(*expectedValue);
        }
    };
    PropertyTester::run(&QOAuth1::temporaryCredentialsUrlChanged, setters);
}

void tst_OAuth1::tokenCredentialsUrlSignal()
{
    using PropertyTester = PropertyTester<QUrl>;
    PropertyTester::SetterFunctions setters {
        [](QUrl *expectedValue, QOAuth1 *object) {
            *expectedValue = QUrl("http://example.net/");
            object->setTokenCredentialsUrl(*expectedValue);
        }
    };
    PropertyTester::run(&QOAuth1::tokenCredentialsUrlChanged, setters);
}

void tst_OAuth1::signatureMethodSignal()
{
    using PropertyTester = PropertyTester<QOAuth1::SignatureMethod>;
    PropertyTester::SetterFunctions setters {
        [](PropertyTester::InnerType *expectedValue, QOAuth1 *object) {
            QVERIFY(object->signatureMethod() != QOAuth1::SignatureMethod::PlainText);
            *expectedValue = QOAuth1::SignatureMethod::PlainText;
            object->setSignatureMethod(*expectedValue);
        }
    };
    PropertyTester::run(&QOAuth1::signatureMethodChanged, setters);
}

void tst_OAuth1::getToken_data()
{
    QTest::addColumn<StringPair>("clientCredentials");
    QTest::addColumn<StringPair>("token");
    QTest::addColumn<StringPair>("expectedToken");
    QTest::addColumn<QOAuth1::SignatureMethod>("signatureMethod");
    QTest::addColumn<QNetworkAccessManager::Operation>("requestType");

    const StringPair emptyCredentials;
    QTest::newRow("temporary_get_plainText")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << emptyCredentials
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << QOAuth1::SignatureMethod::PlainText
            << QNetworkAccessManager::GetOperation;

    QTest::newRow("temporary_get_hmacSha1")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << emptyCredentials
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << QOAuth1::SignatureMethod::Hmac_Sha1
            << QNetworkAccessManager::GetOperation;

    QTest::newRow("temporary_post_plainText")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << emptyCredentials
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << QOAuth1::SignatureMethod::PlainText
            << QNetworkAccessManager::PostOperation;

    QTest::newRow("temporary_post_hmacSha1")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << emptyCredentials
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << QOAuth1::SignatureMethod::Hmac_Sha1
            << QNetworkAccessManager::PostOperation;

    QTest::newRow("token_get_plainText")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << std::make_pair(QStringLiteral("accesskey"), QStringLiteral("accesssecret"))
            << QOAuth1::SignatureMethod::PlainText
            << QNetworkAccessManager::GetOperation;

    QTest::newRow("token_get_hmacSha1")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << std::make_pair(QStringLiteral("accesskey"), QStringLiteral("accesssecret"))
            << QOAuth1::SignatureMethod::Hmac_Sha1
            << QNetworkAccessManager::GetOperation;

    QTest::newRow("token_post_plainText")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << std::make_pair(QStringLiteral("accesskey"), QStringLiteral("accesssecret"))
            << QOAuth1::SignatureMethod::PlainText
            << QNetworkAccessManager::PostOperation;

    QTest::newRow("token_post_hmacSha1")
            << std::make_pair(QStringLiteral("key"), QStringLiteral("secret"))
            << std::make_pair(QStringLiteral("requestkey"), QStringLiteral("requestsecret"))
            << std::make_pair(QStringLiteral("accesskey"), QStringLiteral("accesssecret"))
            << QOAuth1::SignatureMethod::Hmac_Sha1
            << QNetworkAccessManager::PostOperation;
}

void tst_OAuth1::getToken()
{
    QFETCH(StringPair, clientCredentials);
    QFETCH(StringPair, token);
    QFETCH(StringPair, expectedToken);
    QFETCH(QOAuth1::SignatureMethod, signatureMethod);
    QFETCH(QNetworkAccessManager::Operation, requestType);

    StringPair tokenReceived;
    QNetworkAccessManager networkAccessManager;
    QNetworkReplyPtr reply;
    QMultiMap<QString, QVariant> oauthHeaders;

    WebServer webServer([&](const WebServer::HttpRequest &request, QTcpSocket *socket) {
        oauthHeaders = parseAuthorizationString(request.headers["authorization"]);
        const QString format = "oauth_token=%1&oauth_token_secret=%2";
        const QByteArray text = format.arg(expectedToken.first, expectedToken.second).toUtf8();
        const QByteArray replyMessage {
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=\"utf-8\"\r\n"
            "Content-Length: " + QByteArray::number(text.size()) + "\r\n\r\n"
            + text
        };
        socket->write(replyMessage);
    });

    struct OAuth1 : QOAuth1
    {
        OAuth1(QNetworkAccessManager *manager) : QOAuth1(manager) {}
        using QOAuth1::requestTokenCredentials;
    } o1(&networkAccessManager);
    const auto url = webServer.url(QStringLiteral("token"));

    o1.setSignatureMethod(signatureMethod);
    o1.setClientCredentials(clientCredentials.first, clientCredentials.second);
    o1.setTokenCredentials(token);
    o1.setTemporaryCredentialsUrl(url);
    QVariantMap parameters {{ "c2&a3", "c2=a3" }};
    reply.reset(o1.requestTokenCredentials(requestType, url, token, parameters));
    QVERIFY(!reply.isNull());
    connect(&o1, &QOAuth1::tokenChanged, this, [&tokenReceived](const QString &token){
        tokenReceived.first = token;
    });
    connect(&o1, &QOAuth1::tokenSecretChanged, this, [&tokenReceived](const QString &tokenSecret) {
        tokenReceived.second = tokenSecret;
    });
    QVERIFY(waitForFinish(reply) == Success);
    QCOMPARE(tokenReceived, expectedToken);
    QCOMPARE(oauthHeaders.value("oauth_consumer_key"), clientCredentials.first);
    QCOMPARE(oauthHeaders.value("oauth_version"), "1.0");
    QString expectedSignature;
    {
        QMultiMap<QString, QVariant> modifiedHeaders = oauthHeaders;
        modifiedHeaders.unite(QMultiMap<QString, QVariant>(parameters));
        modifiedHeaders.remove("oauth_signature");
        QOAuth1Signature signature(url,
                                   clientCredentials.second,
                                   token.second,
                                   static_cast<QOAuth1Signature::HttpRequestMethod>(requestType),
                                   modifiedHeaders);
        switch (signatureMethod) {
        case QOAuth1::SignatureMethod::PlainText:
            expectedSignature = signature.plainText();
            break;
        case QOAuth1::SignatureMethod::Hmac_Sha1:
            expectedSignature = signature.hmacSha1().toBase64();
            break;
        case QOAuth1::SignatureMethod::Rsa_Sha1:
            expectedSignature = signature.rsaSha1();
            break;
        }
    }
    QCOMPARE(oauthHeaders.value("oauth_signature"), expectedSignature);
}

void tst_OAuth1::prepareRequestSignature_data()
{
    QTest::addColumn<QString>("consumerKey");
    QTest::addColumn<QString>("consumerSecret");
    QTest::addColumn<QString>("accessKey");
    QTest::addColumn<QString>("accessKeySecret");
    QTest::addColumn<QNetworkRequest>("request");
    QTest::addColumn<QByteArray>("operation");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QVariantMap>("extraParams");

    QTest::newRow("get_simple")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << QNetworkRequest(QUrl("http://term.ie/oauth/example/echo_api.php"))
            << QByteArray("GET")
            << QByteArray()
            << QVariantMap();

    QTest::newRow("get_params")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << QNetworkRequest(QUrl("http://term.ie/oauth/example/echo_api.php?"
                                    "first=first&second=second"))
            << QByteArray("GET")
            << QByteArray()
            << QVariantMap();

    QNetworkRequest postRequest(QUrl("http://term.ie/oauth/example/echo_api.php"));
    postRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArray("application/x-www-form-urlencoded"));
    QTest::newRow("post_params")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << postRequest
            << QByteArray("POST")
            << QByteArray("first=first&second=second")
            << QVariantMap({
                               {"first", "first"},
                               {"second", "second"}
                           });

    QTest::newRow("patch_param")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << QNetworkRequest(QUrl("http://term.ie/oauth/example/echo_api.php?"
                                    "first=first&second=second"))
            << QByteArray("PATCH")
            << QByteArray()
            << QVariantMap();
}

void tst_OAuth1::prepareRequestSignature()
{
    QFETCH(QString, consumerKey);
    QFETCH(QString, consumerSecret);
    QFETCH(QString, accessKey);
    QFETCH(QString, accessKeySecret);
    QFETCH(QNetworkRequest, request);
    QFETCH(QByteArray, operation);
    QFETCH(QByteArray, body);
    QFETCH(QVariantMap, extraParams);

    QOAuth1 o1;
    o1.setClientCredentials(consumerKey, consumerSecret);
    o1.setTokenCredentials(accessKey, accessKeySecret);

    o1.prepareRequest(&request, operation, body);

    // extract oauth parameters from the headers
    QMultiMap<QString, QVariant> authArgs;
    const auto authHeader = request.rawHeader("Authorization");
    QCOMPARE(authHeader.mid(0, 6), "OAuth ");
    const auto values = authHeader.mid(6).split(',');
    for (const auto &pair : values) {
        const auto argPair = pair.split('=');
        QCOMPARE(argPair.size(), 2);
        QCOMPARE(argPair[1].front(), '\"');
        QCOMPARE(argPair[1].back(), '\"');
        authArgs.insert(argPair[0], argPair[1].mid(1, argPair[1].size() - 2));
    }

    //compare known parameters
    QCOMPARE(authArgs.value(oauthConsumerKey).toByteArray(), consumerKey.toLatin1());
    QCOMPARE(authArgs.value(oauthToken).toByteArray(), accessKey.toLatin1());
    QCOMPARE(authArgs.value(oauthSignatureMethod).toByteArray(), QByteArray("HMAC-SHA1"));
    QCOMPARE(authArgs.value(oauthVersion).toByteArray(), QByteArray("1.0"));
    QVERIFY(authArgs.contains(oauthNonce));
    QVERIFY(authArgs.contains(oauthTimestamp));
    QVERIFY(authArgs.contains(oauthSignature));

    // verify the signature
    const auto sigString = QUrl::fromPercentEncoding(authArgs.take(oauthSignature)
                                                     .toByteArray()).toUtf8();

    authArgs.unite(QMultiMap<QString, QVariant>(extraParams));
    QOAuth1Signature signature(request.url(),
                               consumerSecret,
                               accessKeySecret,
                               QOAuth1Signature::HttpRequestMethod::Custom,
                               authArgs);
    signature.setCustomMethodString(operation);
    const auto signatureData = signature.hmacSha1();
    QCOMPARE(signatureData.toBase64(), sigString);
}

void tst_OAuth1::grant_data()
{
    QTest::addColumn<QString>("consumerKey");
    QTest::addColumn<QString>("consumerSecret");
    QTest::addColumn<QString>("requestToken");
    QTest::addColumn<QString>("requestTokenSecret");
    QTest::addColumn<QString>("accessToken");
    QTest::addColumn<QString>("accessTokenSecret");
    QTest::addColumn<QUrl>("requestTokenUrl");
    QTest::addColumn<QUrl>("accessTokenUrl");
    QTest::addColumn<QUrl>("authenticatedCallUrl");
    QTest::addColumn<QNetworkAccessManager::Operation>("requestType");

    if (hostReachable(QLatin1String("term.ie"))) {
        QTest::newRow("term.ie_get") << "key"
                                    << "secret"
                                    << "requestkey"
                                    << "requestsecret"
                                    << "accesskey"
                                    << "accesssecret"
                                    << QUrl("http://term.ie/oauth/example/request_token.php")
                                    << QUrl("http://term.ie/oauth/example/access_token.php")
                                    << QUrl("http://term.ie/oauth/example/echo_api.php")
                                    << QNetworkAccessManager::GetOperation;
        QTest::newRow("term.ie_post") << "key"
                                    << "secret"
                                    << "requestkey"
                                    << "requestsecret"
                                    << "accesskey"
                                    << "accesssecret"
                                    << QUrl("http://term.ie/oauth/example/request_token.php")
                                    << QUrl("http://term.ie/oauth/example/access_token.php")
                                    << QUrl("http://term.ie/oauth/example/echo_api.php")
                                    << QNetworkAccessManager::PostOperation;
    } else {
        QSKIP("Skipping test due to unreachable term.ie host");
    }
}

void tst_OAuth1::grant()
{
    QFETCH(QString, consumerKey);
    QFETCH(QString, consumerSecret);
    QFETCH(QString, requestToken);
    QFETCH(QString, requestTokenSecret);
    QFETCH(QString, accessToken);
    QFETCH(QString, accessTokenSecret);
    QFETCH(QUrl, requestTokenUrl);
    QFETCH(QUrl, accessTokenUrl);

    bool tokenReceived = false;
    QNetworkAccessManager networkAccessManager;

    QOAuth1 o1(&networkAccessManager);

    {
        QSignalSpy clientIdentifierSpy(&o1, &QOAuth1::clientIdentifierChanged);
        QSignalSpy clientSharedSecretSpy(&o1, &QOAuth1::clientSharedSecretChanged);
        o1.setClientCredentials(consumerKey, consumerSecret);
        QCOMPARE(clientIdentifierSpy.size(), 1);
        QCOMPARE(clientSharedSecretSpy.size(), 1);
    }
    {
        QSignalSpy spy(&o1, &QOAuth1::temporaryCredentialsUrlChanged);
        o1.setTemporaryCredentialsUrl(requestTokenUrl);
        QCOMPARE(spy.size(), 1);
    }
    {
        QSignalSpy spy(&o1, &QOAuth1::tokenCredentialsUrlChanged);
        o1.setTokenCredentialsUrl(accessTokenUrl);
        QCOMPARE(spy.size(), 1);
    }
    connect(&o1, &QAbstractOAuth::statusChanged, this, [&](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
            if (!requestToken.isEmpty())
                QCOMPARE(requestToken, o1.tokenCredentials().first);
            if (!requestTokenSecret.isEmpty())
                QCOMPARE(requestTokenSecret, o1.tokenCredentials().second);
            tokenReceived = true;
        } else if (status == QAbstractOAuth::Status::Granted) {
            if (!accessToken.isEmpty())
                QCOMPARE(accessToken, o1.tokenCredentials().first);
            if (!accessTokenSecret.isEmpty())
                QCOMPARE(accessTokenSecret, o1.tokenCredentials().second);
            tokenReceived = true;
        }
    });

    QEventLoop eventLoop;

    QSignalSpy grantSignalSpy(&o1, &QOAuth1::granted);
    QTimer::singleShot(10000, &eventLoop, &QEventLoop::quit);
    connect(&o1, &QOAuth1::granted, &eventLoop, &QEventLoop::quit);
    o1.grant();
    eventLoop.exec();
    QVERIFY(tokenReceived);
    QCOMPARE(grantSignalSpy.size(), 1);
    QCOMPARE(o1.status(), QAbstractOAuth::Status::Granted);
}

void tst_OAuth1::authenticatedCalls_data()
{
    QTest::addColumn<QString>("consumerKey");
    QTest::addColumn<QString>("consumerSecret");
    QTest::addColumn<QString>("accessKey");
    QTest::addColumn<QString>("accessKeySecret");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QVariantMap>("parameters");
    QTest::addColumn<QNetworkAccessManager::Operation>("operation");

    const QVariantMap parameters { { QStringLiteral("first"), QStringLiteral("first") },
                                   { QStringLiteral("second"), QStringLiteral("second") },
                                   { QStringLiteral("third"), QStringLiteral("third") },
                                   { QStringLiteral("c2&a3"), QStringLiteral("2=%$&@q") }
                                 };

    if (hostReachable(QLatin1String("term.ie"))) {
        QTest::newRow("term.ie_get") << "key"
                                    << "secret"
                                    << "accesskey"
                                    << "accesssecret"
                                    << QUrl("http://term.ie/oauth/example/echo_api.php")
                                    << parameters
                                    << QNetworkAccessManager::GetOperation;
        QTest::newRow("term.ie_post") << "key"
                                    << "secret"
                                    << "accesskey"
                                    << "accesssecret"
                                    << QUrl("http://term.ie/oauth/example/echo_api.php")
                                    << parameters
                                    << QNetworkAccessManager::PostOperation;
        QTest::newRow("term.ie_percent_encoded_query")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << QUrl("http://term.ie/oauth/example/echo_api.php?key=%40value+1%2B2=3")
            << parameters
            << QNetworkAccessManager::GetOperation;
    } else {
        QSKIP("Skipping test due to unreachable term.ie host");
    }
}

void tst_OAuth1::authenticatedCalls()
{
    QFETCH(QString, consumerKey);
    QFETCH(QString, consumerSecret);
    QFETCH(QString, accessKey);
    QFETCH(QString, accessKeySecret);
    QFETCH(QUrl, url);
    QFETCH(QVariantMap, parameters);
    QFETCH(QNetworkAccessManager::Operation, operation);

    QNetworkAccessManager networkAccessManager;
    QNetworkReplyPtr reply;
    QString receivedData;
    QString parametersString;
    {
        if (url.hasQuery()) {
            parametersString = url.query(QUrl::FullyDecoded);
            if (!parameters.empty())
                parametersString.append(QLatin1Char('&'));
        }
        bool first = true;
        for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it) {
            if (first)
                first = false;
            else
                parametersString += QLatin1Char('&');
            parametersString += it.key() + QLatin1Char('=') + it.value().toString();
        }
    }

    QOAuth1 o1(&networkAccessManager);
    o1.setClientCredentials(consumerKey, consumerSecret);
    o1.setTokenCredentials(accessKey, accessKeySecret);
    if (operation == QNetworkAccessManager::GetOperation)
        reply.reset(o1.get(url, parameters));
    else if (operation == QNetworkAccessManager::PostOperation)
        reply.reset(o1.post(url, parameters));
    QVERIFY(!reply.isNull());
    QVERIFY(!reply->isFinished());

    connect(&networkAccessManager, &QNetworkAccessManager::finished, this,
            [&](QNetworkReply *reply) {
        QByteArray data = reply->readAll();
        QUrlQuery query(QString::fromUtf8(data));
        receivedData = query.toString(QUrl::FullyDecoded);
    });
    QVERIFY(waitForFinish(reply) == Success);
    QCOMPARE(receivedData, parametersString);
    reply.clear();
}

void tst_OAuth1::prepareRequestCalls_data()
{
    QTest::addColumn<QString>("consumerKey");
    QTest::addColumn<QString>("consumerSecret");
    QTest::addColumn<QString>("accessKey");
    QTest::addColumn<QString>("accessKeySecret");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QVariantMap>("parameters");
    QTest::addColumn<QByteArray>("operation");

    const QVariantMap parameters { { QStringLiteral("first"), QStringLiteral("first") },
                                   { QStringLiteral("second"), QStringLiteral("second") },
                                   { QStringLiteral("third"), QStringLiteral("third") },
                                   { QStringLiteral("c2&a3"), QStringLiteral("2=%$&@q") }
                                 };

    if (hostReachable(QLatin1String("term.ie"))) {
        QTest::newRow("term.ie_get") << "key"
                                     << "secret"
                                     << "accesskey"
                                     << "accesssecret"
                                     << QUrl("http://term.ie/oauth/example/echo_api.php")
                                     << parameters
                                     << QByteArray("GET");
        QTest::newRow("term.ie_post") << "key"
                                      << "secret"
                                      << "accesskey"
                                      << "accesssecret"
                                      << QUrl("http://term.ie/oauth/example/echo_api.php")
                                      << parameters
                                      << QByteArray("POST");
        QTest::newRow("term.ie_percent_encoded_query")
            << "key"
            << "secret"
            << "accesskey"
            << "accesssecret"
            << QUrl("http://term.ie/oauth/example/echo_api.php?key=%40value+1%2B2=3")
            << parameters
            << QByteArray("GET");
    } else {
        QSKIP("Skipping test due to unreachable term.ie host");
    }
}

void tst_OAuth1::prepareRequestCalls()
{
    QFETCH(QString, consumerKey);
    QFETCH(QString, consumerSecret);
    QFETCH(QString, accessKey);
    QFETCH(QString, accessKeySecret);
    QFETCH(QUrl, url);
    QFETCH(QVariantMap, parameters);
    QFETCH(QByteArray, operation);

    QNetworkAccessManager networkAccessManager;
    QNetworkReplyPtr reply;
    QString receivedData;
    QString parametersString;
    {
        if (url.hasQuery()) {
            parametersString = url.query(QUrl::FullyDecoded);
            if (!parameters.empty())
                parametersString.append(QLatin1Char('&'));
        }
        bool first = true;
        for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it) {
            if (first)
                first = false;
            else
                parametersString += QLatin1Char('&');
            parametersString += it.key() + QLatin1Char('=') + it.value().toString();
        }
    }

    QOAuth1 o1(&networkAccessManager);
    o1.setClientCredentials(consumerKey, consumerSecret);
    o1.setTokenCredentials(accessKey, accessKeySecret);

    if (operation != "POST") {
        QUrlQuery query(url.query());
        for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it)
            query.addQueryItem(it.key(), it.value().toString());
        url.setQuery(query);
    }
    QNetworkRequest request(url);
    QByteArray body;
    if (operation == "POST") {
        QUrlQuery query(url.query());
        for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it)
            query.addQueryItem(it.key(), it.value().toString());
        body = query.toString().toUtf8();
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArray("application/x-www-form-urlencoded"));
    }

    o1.prepareRequest(&request, operation, body);
    if (body.isEmpty())
        reply.reset(o1.networkAccessManager()->sendCustomRequest(request, operation));
    else
        reply.reset(o1.networkAccessManager()->sendCustomRequest(request, operation, body));
    QVERIFY(!reply.isNull());
    QVERIFY(!reply->isFinished());

    connect(&networkAccessManager, &QNetworkAccessManager::finished, this,
            [&](QNetworkReply *reply) {
        QByteArray data = reply->readAll();
        QUrlQuery query(QString::fromUtf8(data));
        receivedData = query.toString(QUrl::FullyDecoded);
    });
    QVERIFY(waitForFinish(reply) == Success);
    QCOMPARE(receivedData, parametersString);
    reply.clear();
}

void tst_OAuth1::secondTemporaryToken()
{
    QNetworkAccessManager networkAccessManager;

    const StringPair expectedToken(std::make_pair(QStringLiteral("temporaryKey"), QStringLiteral("temporaryToken")));
    WebServer webServer([&](const WebServer::HttpRequest &request, QTcpSocket *socket) {
        Q_UNUSED(request);
        const QString format = "oauth_token=%1&oauth_token_secret=%2&oauth_callback_confirmed=true";
        const QByteArray text = format.arg(expectedToken.first, expectedToken.second).toUtf8();
        const QByteArray replyMessage {
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=\"utf-8\"\r\n"
            "Content-Length: " + QByteArray::number(text.size()) + "\r\n\r\n"
            + text
        };
        socket->write(replyMessage);
    });

    QOAuth1 o1(&networkAccessManager);

    StringPair clientCredentials = std::make_pair(QStringLiteral("user"), QStringLiteral("passwd"));
    o1.setClientCredentials(clientCredentials);
    o1.setTemporaryCredentialsUrl(webServer.url(QStringLiteral("temporary")));
    o1.setAuthorizationUrl(webServer.url(QStringLiteral("authorization")));
    o1.setTokenCredentialsUrl(webServer.url(QStringLiteral("token")));

    StringPair tokenReceived;
    connect(&o1, &QOAuth1::tokenChanged, this, [&tokenReceived](const QString &token) {
        tokenReceived.first = token;
    });
    bool replyReceived = false;
    connect(&o1, &QOAuth1::tokenSecretChanged,
            this, [&tokenReceived, &replyReceived](const QString &tokenSecret) {
        tokenReceived.second = tokenSecret;
        replyReceived = true;
    });

    o1.grant();
    QTRY_VERIFY(replyReceived);

    QVERIFY(!tokenReceived.first.isEmpty());
    QVERIFY(!tokenReceived.second.isEmpty());
    QCOMPARE(o1.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);
    QCOMPARE(tokenReceived, expectedToken);

    replyReceived = false; // reset this so we can 'synchronize' on it again
    // Do the same request again, should end up in the same state!!
    o1.grant();
    QTRY_VERIFY(replyReceived);

    QVERIFY(!tokenReceived.first.isEmpty());
    QVERIFY(!tokenReceived.second.isEmpty());
    QCOMPARE(o1.status(), QAbstractOAuth::Status::TemporaryCredentialsReceived);
    QCOMPARE(tokenReceived, expectedToken);
}

QTEST_MAIN(tst_OAuth1)
#include "tst_oauth1.moc"
