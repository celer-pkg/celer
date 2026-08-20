// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef OAUTHTESTUTILS_H
#define OAUTHTESTUTILS_H

#include "tlswebserver.h"
#include "webserver.h"

#include <QtNetworkAuth/qoauthglobal.h>

#ifndef QT_NO_SSL
#include <QtNetwork/qsslconfiguration.h>
#endif
#include <QtNetwork/qtcpsocket.h>

#include <QtCore/qcontainerfwd.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qstring.h>
#include <QtCore/qtenvironmentvariables.h>

#include <memory>

[[nodiscard]] inline auto useTemporaryKeychain()
{
#ifndef QT_NO_SSL
    // Set the same environment value as CI uses, so that it's possible
    // to run autotests locally without macOS asking for permission to use
    // a private key in keychain (with TLS sockets)
    auto value = qEnvironmentVariable("QT_SSL_USE_TEMPORARY_KEYCHAIN");
    qputenv("QT_SSL_USE_TEMPORARY_KEYCHAIN", "1");
    auto envRollback = qScopeGuard([value](){
        if (value.isEmpty())
            qunsetenv("QT_SSL_USE_TEMPORARY_KEYCHAIN");
        else
            qputenv("QT_SSL_USE_TEMPORARY_KEYCHAIN", value.toUtf8());
    });
    return envRollback;
#else
    // avoid maybe-unused warnings from callers
    return qScopeGuard([]{});
#endif // QT_NO_SSL
}

QString createSignedJWT(const QVariantMap &header = {}, const QVariantMap &payload = {});

#ifndef QT_NO_SSL
QSslConfiguration createSslConfiguration(const QString &keyFileName,
                                         const QString &certificateFileName);
#endif // QT_NO_SSL

struct ServerResponses
{
    QByteArray authBody;
    QByteArray authHttpStatus;
    QByteArray tokenBody;
    QByteArray tokenHttpStatus;
};

template<typename ServerType>
struct TestAuthorizationServer
{
    std::unique_ptr<ServerType> server;
    QList<WebServer::HttpRequest> receivedAuthorizationRequests;
    QList<WebServer::HttpRequest> receivedTokenRequests;
    ServerResponses responses;

    QUrl authorizationEndpoint()
    {
        Q_ASSERT(server);
        return server->url(QStringLiteral("authorizationEndpoint"));
    }

    QUrl tokenEndpoint()
    {
        Q_ASSERT(server);
        return server->url(QStringLiteral("tokenEndpoint"));
    }
};

// Creates a local http authorization server.
// The provided ServerResponses are used as the initial values. The testcase
// can modify individual response members during the testcase by modifying the returned
// instance's TestAuthorizationServer::responses contents.
// The template is used so that the function can return either WebServer* or TlsWebServer*
template<typename ServerType, typename... Args>
std::unique_ptr<TestAuthorizationServer<ServerType>> createAuthorizationServer(
    ServerResponses responses, Args&&... args)
{
    auto result = std::make_unique<TestAuthorizationServer<ServerType>>();
    result->responses = std::move(responses);

    auto handler = [raw = result.get()]
        (const WebServer::HttpRequest &request, QTcpSocket *socket) {
        QByteArray replyMessage;
        if (request.url.path() == QLatin1StringView("/authorizationEndpoint")) {
            // Set received request for test cases to check
            raw->receivedAuthorizationRequests.append(request);
            replyMessage =
                "HTTP/1.0 " + raw->responses.authHttpStatus + "\r\n"
                "Content-Type: application/json; charset=\"utf-8\"\r\n"
                "Content-Length: " + QByteArray::number(raw->responses.authBody.size())
                + "\r\n\r\n" + raw->responses.authBody;
        } else if (request.url.path() == QLatin1StringView("/tokenEndpoint")) {
            // Set received request for test cases to check
            raw->receivedTokenRequests.append(request);
            replyMessage =
                "HTTP/1.0 " + raw->responses.tokenHttpStatus + "\r\n"
                "Content-Type: application/json; charset=\"utf-8\"\r\n"
                "Content-Length: " + QByteArray::number(raw->responses.tokenBody.size())
                + "\r\n\r\n" + raw->responses.tokenBody;
        } else {
            qFatal() << "Unsupported URL:" << request.url;
        }
        socket->write(replyMessage);
    };
    result->server.reset(new ServerType(handler, std::forward<Args>(args)...));
    return result;
}

#endif // OAUTHTESTUTILS_H
