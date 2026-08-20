// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtHttpServer/qhttpserver.h>

#include <QtCore/qobject.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qbuffer.h>

#include <QtNetwork/qnetworkaccessmanager.h>
#if QT_CONFIG(ssl)
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslserver.h>
#include <QtNetwork/qsslsocket.h>
#endif

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

class tst_bench_QHttpServer_transfer : public QObject
{
    Q_OBJECT
public:
    tst_bench_QHttpServer_transfer();
private slots:
    void transferPayload_data();
    void transferPayload();
private:
    QHttpServer server;
    bool testHttp2 = false;
    bool testTls = false;
    quint16 ports[2] = {0, 0};
};

tst_bench_QHttpServer_transfer::tst_bench_QHttpServer_transfer()
{
    auto *tcpServer = new QTcpServer(&server);
    if (!tcpServer->listen())
        qFatal("Failed to listen on TCP");
    ports[0] = tcpServer->serverPort();
    server.bind(tcpServer);
    qDebug() << "Test HTTP on port" << ports[0];
#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl()) {
        testHttp2 = QSslSocket::isFeatureSupported(QSsl::SupportedFeature::ServerSideAlpn);
        testTls = true;
        auto *sslServer = new QSslServer(&server);
        sslServer->setHandshakeTimeout(std::numeric_limits<int>::max());
        QSslConfiguration sslConfig = sslServer->sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        if (testHttp2)
            sslConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
        const auto certs = QSslCertificate::fromPath(u":/cert/localhost.cert"_s);
        if (certs.isEmpty())
            qFatal("Failed to load certificate");
        sslConfig.setLocalCertificate(certs.first());
        QFile key(u":/cert/localhost.key"_s);
        if (!key.open(QIODevice::ReadOnly))
            qFatal("Failed to open key file");
        sslConfig.setPrivateKey(QSslKey(&key, QSsl::KeyAlgorithm::Rsa));
        sslServer->setSslConfiguration(sslConfig);
        if (!sslServer->listen(QHostAddress::LocalHost))
            qFatal("Failed to listen on TLS");
        ports[1] = sslServer->serverPort();
        server.bind(sslServer);
        qDebug() << "Testing TLS with" << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "Testing HTTPS on port" << ports[1];
    }
#endif

    QByteArray bigdata = "1"_ba.repeated(1024 * 1024 * 100);
    using HTTP = QHttpServerRequest::Method;
    server.route("/bytearray", HTTP::Get,
                 [=](QHttpServerResponder &resp) {
                     resp.write(bigdata, "application/octet-stream");
                 });

    server.route("/qbuffer", HTTP::Get,
                 [=](QHttpServerResponder &resp) {
                     auto *buf = new QBuffer;
                     buf->setData(bigdata);
                     buf->open(QIODevice::ReadOnly);
                     resp.write(buf, "application/octet-stream");
                 });
}

enum TransferProtocol {
    HTTP,
    HTTPS,
    HTTP2,
};

void tst_bench_QHttpServer_transfer::transferPayload_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<TransferProtocol>("transferProtocol");

    QTest::addRow("bytearray") << "/bytearray" << HTTP;
    QTest::addRow("qbuffer") << "/qbuffer" << HTTP;
    if (testHttp2) {
        QTest::addRow("bytearray-http2") << "/bytearray" << HTTP2;
        QTest::addRow("qbuffer-http2") << "/qbuffer" << HTTP2;
    }
    if (testTls) {
        QTest::addRow("bytearray-tls") << "/bytearray" << HTTPS;
        QTest::addRow("qbuffer-tls") << "/qbuffer" << HTTPS;
    }
}

void tst_bench_QHttpServer_transfer::transferPayload()
{
    QFETCH(QString, path);
    QFETCH(TransferProtocol, transferProtocol);
    const bool tls = transferProtocol != HTTP;

    QNetworkAccessManager qnam;
    QNetworkRequest req;
    QUrl url;
    url.setHost("127.0.0.1");
    url.setScheme(tls ? "https" : "http");
    url.setPort(ports[tls ? 1 : 0]);
    url.setPath(path);
    req.setUrl(url);

    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, transferProtocol == HTTP2);

#if QT_CONFIG(ssl)
    if (tls) {
        QSslConfiguration sslConfig = req.sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        req.setSslConfiguration(sslConfig);
    }
#endif

    QBENCHMARK {
        QNetworkReply *reply = qnam.get(req);
        QObject::connect(reply, &QNetworkReply::sslErrors, reply,
                         qOverload<>(&QNetworkReply::ignoreSslErrors));
        qint64 total = 0;
        const auto replyReady = [&reply]() -> bool {
            return reply->isFinished() || reply->bytesAvailable();
        };
        while (!replyReady()) {
            bool res = QTest::qWaitFor(replyReady, 5s);
            QVERIFY(res);
            total += reply->readAll().size();
        }
        QCOMPARE(total, 1024 * 1024 * 100);
        delete reply;
    }

}

QTEST_MAIN(tst_bench_QHttpServer_transfer)

#include "tst_bench_qhttpserver_transfer.moc"
