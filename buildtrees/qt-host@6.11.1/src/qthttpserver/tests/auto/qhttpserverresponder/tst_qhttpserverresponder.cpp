// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qabstracthttpserver.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qfile.h>
#include <QtCore/qtemporaryfile.h>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qtcpserver.h>

#include <functional>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

static const QByteArray headerServerString = "Server"_ba;
static const QByteArray headerServerValue = "Test server"_ba;

class tst_QHttpServerResponder : public QObject
{
    Q_OBJECT

    std::unique_ptr<QNetworkAccessManager> networkAccessManager;

private slots:
    void init() { networkAccessManager.reset(new QNetworkAccessManager); }
    void cleanup() { networkAccessManager.reset(); }

    void defaultStatusCodeNoParameters();
    void defaultStatusCodeByteArray();
    void defaultStatusCodeJson();
    void writeStatusCode_data();
    void writeStatusCode();
    void writeStatusCodeExtraHeader();
    void writeJson();
    void writeJsonExtraHeader();
    void writeFile_data();
    void writeFile();
    void writeFileExtraHeader();
    void writeByteArrayExtraHeader();
};

#define qWaitForFinished(REPLY) QVERIFY(QSignalSpy(REPLY, &QNetworkReply::finished).wait())

struct HttpServer : QAbstractHttpServer {
    std::function<void(QHttpServerResponder responder)> handleRequestFunction;
    QUrl url;

    HttpServer(decltype(handleRequestFunction) function) : handleRequestFunction(function)
    {
        auto tcpserver = std::make_unique<QTcpServer>();
        tcpserver->listen();
        quint16 port = tcpserver->serverPort();
        bind(tcpserver.get());
        tcpserver.release();
        url.setUrl(QStringLiteral("http://localhost:%1").arg(port));
    }

    bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &) override;
    void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
    {
        Q_ASSERT(false);
    }
};

bool HttpServer::handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder)
{
    handleRequestFunction(std::move(responder));
    return true;
}

void tst_QHttpServerResponder::defaultStatusCodeNoParameters()
{
    HttpServer server([](QHttpServerResponder responder) { responder.write(); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::defaultStatusCodeByteArray()
{
    HttpServer server([](QHttpServerResponder responder) {
        responder.write(QByteArray(), "application/x-empty"_ba);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::defaultStatusCodeJson()
{
    const auto json = QJsonDocument::fromJson("{}"_ba);
    HttpServer server([json](QHttpServerResponder responder) { responder.write(json); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
}

void tst_QHttpServerResponder::writeStatusCode_data()
{
    using StatusCode = QHttpServerResponder::StatusCode;

    QTest::addColumn<QHttpServerResponder::StatusCode>("statusCode");
    QTest::addColumn<QNetworkReply::NetworkError>("networkError");

    QTest::addRow("OK") << StatusCode::Ok << QNetworkReply::NoError;
    QTest::addRow("Content Access Denied") << StatusCode::Forbidden
                                           << QNetworkReply::ContentAccessDenied;
    QTest::addRow("Connection Refused") << StatusCode::NotFound
                                        << QNetworkReply::ContentNotFoundError;
}

void tst_QHttpServerResponder::writeStatusCode()
{
    QFETCH(QHttpServerResponder::StatusCode, statusCode);
    QFETCH(QNetworkReply::NetworkError, networkError);
    HttpServer server([statusCode](QHttpServerResponder responder) {
        responder.write(statusCode);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->bytesAvailable(), 0);
    QCOMPARE(reply->error(), networkError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(),
             "application/x-empty"_ba);
}

void tst_QHttpServerResponder::writeStatusCodeExtraHeader()
{
    HttpServer server([=](QHttpServerResponder responder) {
        QHttpHeaders headers;
        headers.append(headerServerString, headerServerValue);
        responder.write(headers);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->bytesAvailable(), 0);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader).toByteArray(), headerServerValue);
}

void tst_QHttpServerResponder::writeJson()
{
    const auto json = QJsonDocument::fromJson(R"JSON({ "key" : "value" })JSON"_ba);
    HttpServer server([json](QHttpServerResponder responder) { responder.write(json); });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(),
             "application/json"_ba);
    QCOMPARE(QJsonDocument::fromJson(reply->readAll()), json);
}

void tst_QHttpServerResponder::writeJsonExtraHeader()
{
    const auto json = QJsonDocument::fromJson(R"JSON({ "key" : "value" })JSON"_ba);
    HttpServer server([json](QHttpServerResponder responder) {
        QHttpHeaders headers;
        headers.append(headerServerString, headerServerValue);
        responder.write(json, headers);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    qWaitForFinished(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(),
             "application/json"_ba);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader).toByteArray(), headerServerValue);
    QCOMPARE(QJsonDocument::fromJson(reply->readAll()), json);
}

void tst_QHttpServerResponder::writeFile_data()
{
    QTest::addColumn<QIODevice *>("iodevice");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("data");

    QTest::addRow("index.html")
        << qobject_cast<QIODevice *>(new QFile(QFINDTESTDATA("data/index.html"), this))
        << 200
        << "text/html"
        << "<html></html>";

    QTest::addRow("index1.html - not found")
        << qobject_cast<QIODevice *>(new QFile("./index1.html", this))
        << 500
        << "application/x-empty"
        << QString();

    QTest::addRow("temporary file")
        << qobject_cast<QIODevice *>(new QTemporaryFile(this))
        << 200
        << "text/html"
        << QString();
}

void tst_QHttpServerResponder::writeFile()
{
    QFETCH(QIODevice *, iodevice);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, data);

    QSignalSpy spyDestroyIoDevice(iodevice, &QObject::destroyed);

    HttpServer server([&iodevice](QHttpServerResponder responder) {
        responder.write(iodevice, "text/html"_ba);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll().trimmed(), data);

    QCOMPARE(spyDestroyIoDevice.size(), 1);
}

void tst_QHttpServerResponder::writeFileExtraHeader()
{
    auto file = new QFile(QFINDTESTDATA("data/index.html"), this);
    QSignalSpy spyDestroyIoDevice(file, &QObject::destroyed);

    HttpServer server([=](QHttpServerResponder responder) {
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::ContentType, "text/html"_ba);
        headers.append(headerServerString, headerServerValue);

        responder.write(file, headers);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(),
                           "text/html"_ba);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader).toByteArray(), headerServerValue);
    QCOMPARE(reply->readAll().trimmed(), "<html></html>");

    QCOMPARE(spyDestroyIoDevice.size(), 1);
}

void tst_QHttpServerResponder::writeByteArrayExtraHeader()
{
    const QByteArray data("test data");
    const QByteArray contentType("text/plain");

    HttpServer server([=](QHttpServerResponder responder) {
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::ContentType, contentType);
        headers.append(headerServerString, headerServerValue);

        responder.write(data, headers);
    });
    auto reply = networkAccessManager->get(QNetworkRequest(server.url));
    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader).toByteArray(), contentType);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader).toByteArray(), headerServerValue);
    QCOMPARE(reply->readAll(), data);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerResponder)

#include "tst_qhttpserverresponder.moc"
