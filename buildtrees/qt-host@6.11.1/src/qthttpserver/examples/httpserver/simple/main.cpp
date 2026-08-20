// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QHttpServer>
#include <QHttpServerResponse>

#if QT_CONFIG(ssl)
#  include <QSslCertificate>
#  include <QSslKey>
#  include <QSslServer>
#endif

#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>
#include <QString>
#include <QTcpServer>

using namespace Qt::StringLiterals;

static inline QString host(const QHttpServerRequest &request)
{
    return QString::fromLatin1(request.value("Host"));
}

int main(int argc, char *argv[])
{
    //! [Setting up HTTP server]
    QCoreApplication app(argc, argv);

    QHttpServer httpServer;
    httpServer.route("/", []() {
        return "Hello world";
    });
    //! [Setting up HTTP server]

    httpServer.route("/query", [] (const QHttpServerRequest &request) {
        return host(request) + u"/query/"_s;
    });

    httpServer.route("/query/", [] (qint32 id, const QHttpServerRequest &request) {
        return u"%1/query/%2"_s.arg(host(request)).arg(id);
    });

    httpServer.route("/query/<arg>/log", [] (qint32 id, const QHttpServerRequest &request) {
        return u"%1/query/%2/log"_s.arg(host(request)).arg(id);
    });

    httpServer.route("/query/<arg>/log/", [] (qint32 id, float threshold,
                                              const QHttpServerRequest &request) {
        return u"%1/query/%2/log/%3"_s.arg(host(request)).arg(id).arg(threshold);
    });

    httpServer.route("/user/", [] (const qint32 id) {
        return u"User "_s + QString::number(id);
    });

    httpServer.route("/user/<arg>/detail", [] (const qint32 id) {
        return u"User %1 detail"_s.arg(id);
    });

    httpServer.route("/user/<arg>/detail/", [] (const qint32 id, const qint32 year) {
        return u"User %1 detail year - %2"_s.arg(id).arg(year);
    });

    httpServer.route("/json/", [] {
        return QJsonObject{
            {
                {"key1", "1"},
                {"key2", "2"},
                {"key3", "3"}
            }
        };
    });

    //! [Returning assets]
    httpServer.route("/assets/<arg>", [] (const QUrl &url) {
        return QHttpServerResponse::fromFile(u":/assets/"_s + url.path());
    });
    //! [Returning assets]

    //! [Using QHttpServerRequest]
    httpServer.route("/remote_address", [](const QHttpServerRequest &request) {
        return request.remoteAddress().toString();
    });
    //! [Using QHttpServerRequest]

    // Basic authentication example (RFC 7617)
    //! [Using basic authentication]
    httpServer.route("/auth", [](const QHttpServerRequest &request) {
        auto auth = request.value("authorization").simplified();

        if (auth.size() > 6 && auth.first(6).toLower() == "basic ") {
            auto token = auth.sliced(6);
            auto userPass = QByteArray::fromBase64(token);

            if (auto colon = userPass.indexOf(':'); colon > 0) {
                auto userId = userPass.first(colon);
                auto password = userPass.sliced(colon + 1);

                if (userId == "Aladdin" && password == "open sesame")
                    return QHttpServerResponse("text/plain", "Success\n");
            }
        }
        QHttpServerResponse resp("text/plain", "Authentication required\n",
                                 QHttpServerResponse::StatusCode::Unauthorized);
        auto h = resp.headers();
        h.append(QHttpHeaders::WellKnownHeader::WWWAuthenticate,
                 R"(Basic realm="Simple example", charset="UTF-8")");
        resp.setHeaders(std::move(h));
        return std::move(resp);
    });
    //! [Using basic authentication]

    //! [Using addAfterRequestHandler()]
    httpServer.addAfterRequestHandler(&httpServer, [](const QHttpServerRequest &, QHttpServerResponse &resp) {
        auto h = resp.headers();
        h.append(QHttpHeaders::WellKnownHeader::Server, "Qt HTTP Server");
        resp.setHeaders(std::move(h));
    });
    //! [Using addAfterRequestHandler()]

    //! [Binding the TCP server]
    auto tcpserver = std::make_unique<QTcpServer>();
    if (!tcpserver->listen() || !httpServer.bind(tcpserver.get())) {
        qWarning() << QCoreApplication::translate("QHttpServerExample",
                                                  "Server failed to listen on a port.");
        return -1;
    }
    quint16 port = tcpserver->serverPort();
    tcpserver.release();
    //! [Binding the TCP server]

#if QT_CONFIG(ssl)
    //! [HTTPS Configuration example]
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    const auto sslCertificateChain =
            QSslCertificate::fromPath(QStringLiteral(":/assets/certificate.crt"));
    if (sslCertificateChain.empty()) {
        qWarning() << QCoreApplication::translate("QHttpServerExample",
                                                  "Couldn't retrieve SSL certificate from file.");
        return -1;
    }
    QFile privateKeyFile(QStringLiteral(":/assets/private.key"));
    if (!privateKeyFile.open(QIODevice::ReadOnly)) {
        qWarning() << QCoreApplication::translate("QHttpServerExample",
                                                  "Couldn't open file for reading: %1")
                      .arg(privateKeyFile.errorString());
        return -1;
    }

    conf.setLocalCertificate(sslCertificateChain.front());
    conf.setPrivateKey(QSslKey(&privateKeyFile, QSsl::Rsa));
    conf.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 }); // Add HTTP/2 support

    privateKeyFile.close();

    auto sslserver = std::make_unique<QSslServer>();
    sslserver->setSslConfiguration(conf);
    if (!sslserver->listen() || !httpServer.bind(sslserver.get())) {
        qWarning() << QCoreApplication::translate("QHttpServerExample",
                                                  "Server failed to listen on a port.");
        return -1;
    }
    quint16 sslPort = sslserver->serverPort();
    sslserver.release();

    //! [HTTPS Configuration example]

    qInfo().noquote()
        << QCoreApplication::translate("QHttpServerExample",
                                       "Running on http://127.0.0.1:%1/ and "
                                       "https://127.0.0.1:%2/ (Press CTRL+C to quit)")
           .arg(port).arg(sslPort);
#else
    qInfo().noquote()
        << QCoreApplication::translate("QHttpServerExample",
                                       "Running on http://127.0.0.1:%1/"
                                       "(Press CTRL+C to quit)").arg(port);
#endif
    //! [Start handling requests]
    return app.exec();
    //! [Start handling requests]
}
