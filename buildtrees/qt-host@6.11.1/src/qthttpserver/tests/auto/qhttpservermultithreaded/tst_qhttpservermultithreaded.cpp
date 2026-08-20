// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserver.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtTest/qtest.h>
#include <QtConcurrent/qtconcurrentrun.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qurl.h>
#include <QtCore/qstring.h>
#include <QtCore/qsemaphore.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#ifndef QTEST_THROW_ON_FAIL
# error This test requires QTEST_THROW_ON_FAIL being active.
#endif

#if QT_CONFIG(localserver)
#include <QtNetwork/qlocalsocket.h>
#include <QtNetwork/qlocalserver.h>
#endif

#if QT_CONFIG(ssl)
// TODO check if guards are necessary
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslserver.h>

#include <QtTest/private/qtesthelpers_p.h>

constexpr char g_privateKey[] = R"(-----BEGIN RSA PRIVATE KEY-----
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
-----END RSA PRIVATE KEY-----)";

constexpr char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
MIIFszCCA5ugAwIBAgIUfpP54qSLfus/pFUIBDizbnrDjE4wDQYJKoZIhvcNAQEL
BQAwaDELMAkGA1UEBhMCRlIxDzANBgNVBAgMBkZyYW5jZTERMA8GA1UEBwwIR3Jl
bm9ibGUxFjAUBgNVBAoMDVF0Q29udHJpYnV0b3IxHTAbBgNVBAMMFHFodHRwc3Nl
cnZlcnRlc3QuY29tMCAXDTIyMDIwNzE0MzE0NVoYDzIyNjgwNzA3MTQzMTQ1WjBo
MQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREwDwYDVQQHDAhHcmVub2Js
ZTEWMBQGA1UECgwNUXRDb250cmlidXRvcjEdMBsGA1UEAwwUcWh0dHBzc2VydmVy
dGVzdC5jb20wggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC92u1m1Wq7
CIbXb693c6NFWJt0rY5vvcM45aBvmsiBD4HZTNw4VrTGyJHJz9tqb0IQSqCflgdK
f3kske83d1uwDy+sWohdOSKwUC4cmqyaxhlsAiIJfy3W2VxSZCwmaNnUgqGTfuWA
YwqRzbw6H8IdtOuEllZ1DKgFs1vwp6pIeR49i7T8Ey07+WuRcmNXoFeArwGA6ol8
6qsYKwG9b6ypnP0PnVkEFVvdgKsFk/bQ1UDnFlSo4XBLeOf72E7VeUlzCIUDdXNT
u5JW3bOPOFfSIfDoY5VuNHghK2of/LShr1H2ZBrSGbNDstxX02KCFfmQzw+yBP2L
peG/aeayOk+t4tiBeheqkNGsvQ9HQJK2za5xF1+EyFjxWI2hqtnXZnYJNo3GIc8b
M1g0k9pAbjSjcsRtDcY/uKBnyCj999JX6RklLmO4AcKxS81QDiC8V9VcvHOgbqp7
AtdIJeioPIn1PuDgvzVbX2GwHFWnyAOpUCIGemD4ZMtoLr1Y6b04xcRE5M6WKfuD
GVDSlk77b8ze4UWj+FeUgf8xvk08qWk0GqAumojC0Yy6C3sPH0mgzBa8QGXMyB7J
xiY1fBIex1DmTEVLm5i5kSuLrZeuT9ZaV6/88MJF1xJSGuNVI2i0VC94YpLALnPY
U4AhmKgDSY/GwGLFJpBjx+GJoLnJ1TDgXQIDAQABo1MwUTAdBgNVHQ4EFgQUK7Un
0JA3DBUVhclrm6pIZsO60U4wHwYDVR0jBBgwFoAUK7Un0JA3DBUVhclrm6pIZsO6
0U4wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAgEAuvSFAgfgurDT
/dbXuJ0O+FuGg4NOTNRil5ji3DnMzReIHpSiKiXu76PHHRFnlosvfAFOHlpYntun
LhbUAxim/iIgWZR33uzvqXMXBORZ0zffjy2SjGCW8ZJYyTmg9c0tc0jEjv7owtlU
m6tUXMOs9U0CzvEKLt0K0dMALaLkXtscuzEWA4PHVvnvTu0Wyjj/8n+DgYzY09kC
YF0lJfcG6bddDgspmYyFpULeGGP7+qwgGh4cVBtY5I4Htr3p7hDo6UGDF6AsMQZF
1CAEgBVRbJgI2GTnptpm9k3EFKwQ81z5O+NnP3ZsuuZ3CEVaPHyQf/POLAIhmZLt
0vS9qoRiS4uMUJDXz2kJFBOFHki073eMvHiKtlpYOlJXMQ4MkHCydjeeuhHcgUCq
ZDWuQMmq/8tMwf4YtvxYtXzAMVW9dM8BgWu2G8/JwPMGUGhLfKkHmc8dlQzGDe/W
K/uVHlJZNF4Y0eXVlq9DUhpvKOjGc8A208wQlsTUgPxljgJ2+4F3D+t0luc3h65m
25iw8eRGuYDoCQLG7u7MI0g8A0H+0h9Xrt8PQql86vmQhmTUhKfedVGOo2t2Bcfn
ignL7f4e1m2jh0oWTLhuP1hnVFN4KAKpVIJXhbEkH59cLCN6ARXiEHCM9rmK5Rgk
NQZlAZc2w1Ha9lqisaWWpt42QVhQM64=
-----END CERTIFICATE-----)";
#endif // QT_CONFIG(ssl)

#include <atomic>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

enum ServerType
{
    HTTP1_0,
    HTTP1_1,
#if QT_CONFIG(ssl)
    SSL,
#endif
#if QT_CONFIG(localserver)
    LOCAL
#endif
};

static int port = 0;
#if QT_CONFIG(ssl)
static int sslPort = 0;
#endif
static const QString local = u"tst_qhttpservermultithreaded"_s + QString::number(QCoreApplication::applicationPid());
// Simple HTTP 1.1 client
class LocalHttpClient
{
public:
    LocalHttpClient(ServerType type);
    ~LocalHttpClient();
    QString get(const QString &url, const QHttpHeaders &headers = {});
    QString getSlowRead(const QString &url, qsizetype chunkSize, qsizetype mSleep,
                        const QHttpHeaders &headers);
    void pipelinedSendGet(const QString &url);
    QString piplinedFetchResults();
    QString postSlow(const QString &url, const QHttpHeaders &headers, qsizetype mSleep);

private:
    void sendGet(const QString &url, const QHttpHeaders &headers = {});
    QString fetchResults();
    void fetchStatusLine();
    void fetchCrLf();
    QIODevice *socket = nullptr;
    QByteArray version;
    ServerType serverType;
};

LocalHttpClient::LocalHttpClient(ServerType type)
    : version(type == HTTP1_0 ? "HTTP/1.0" : "HTTP/1.1"), serverType(type)
{
    if (type == HTTP1_0 || type == HTTP1_1) {
        QTcpSocket *tcpSocket = new QTcpSocket();
        tcpSocket->connectToHost("localhost", port);
        tcpSocket->waitForConnected();
        QVERIFY2(tcpSocket->waitForConnected(),
                 "Failed waiting for client TCP socket to be connected");
        socket = tcpSocket;
#if QT_CONFIG(ssl)
    } else if (type == SSL) {
        QVERIFY2(QSslSocket::supportsSsl(),
                 "Client SSL socket requested even though it is not supported");
        QSslSocket *sslSocket = new QSslSocket();
        const QList<QSslError> expectedSslErrors = {
            QSslError(QSslError::SelfSignedCertificate, QSslCertificate(g_certificate)),
            // Non-OpenSSL backends are not able to report a specific error code
            // for self-signed certificates.
            QSslError(QSslError::CertificateUntrusted, QSslCertificate(g_certificate)),
            QSslError(QSslError::HostNameMismatch, QSslCertificate(g_certificate)),
        };
        sslSocket->ignoreSslErrors(expectedSslErrors);
        sslSocket->connectToHostEncrypted("localhost", sslPort);
        QVERIFY2(sslSocket->waitForEncrypted(),
                 "Failed waiting for client SSL socket to be encrypted");
        socket = sslSocket;
#endif
#if QT_CONFIG(localserver)
    } else if (type == LOCAL) {
        QLocalSocket *localSocket = new QLocalSocket();
#ifdef Q_OS_LINUX
        localSocket->setSocketOptions(QLocalSocket::AbstractNamespaceOption);
#endif
        localSocket->connectToServer(local);
        QVERIFY2(localSocket->waitForConnected(),
                 "Failed waiting for client local socket to be connected");
        socket = localSocket;
#endif
    } else {
        QFAIL("Unknown server type");
    }
}

LocalHttpClient::~LocalHttpClient()
{
    delete socket;
}

void LocalHttpClient::pipelinedSendGet(const QString &url)
{
    sendGet(url);
}

QString LocalHttpClient::piplinedFetchResults()
{
    return fetchResults();
}

QString LocalHttpClient::get(const QString &url, const QHttpHeaders &headers)
{
    sendGet(url, headers);
    return fetchResults();
}

QString LocalHttpClient::getSlowRead(const QString &url, qsizetype chunkSize, qsizetype mSleep,
                                     const QHttpHeaders &headers)
{
    sendGet(url, headers);

    qint64 contentLength = -1;
    constexpr qint64 headerBufferSize = 4 * 1024;
    char headerBuffer[headerBufferSize];
    qint64 read = 0;
    forever {
        while (!socket->canReadLine()) {
            socket->waitForReadyRead(10);
        }
        read = socket->readLine(headerBuffer, headerBufferSize);
        QVERIFY2(read >= 0, "IO error reading headers");
        if (read <= 2)
            break; // End of headers
        QByteArrayView line(headerBuffer, read);
        auto colon = line.indexOf(':');
        if (colon != -1 && colon + 1 < line.size()) {
            auto headerTitle = line.first(colon);
            if (headerTitle.compare("Content-Length", Qt::CaseInsensitive) == 0)
                contentLength = line.sliced(colon + 1).trimmed().toLongLong();
        }
    };

    QVERIFY2(contentLength != -1, "Content-Length field missing");
    if (contentLength == 0)
        return u""_s; // No content

    read = 0;
    QByteArray buffer(contentLength, Qt::Uninitialized);
    forever {
        qint64 result = socket->read(&buffer[read], qMin(contentLength - read, chunkSize));
        QVERIFY2(result >= 0, "IO error reading content");
        read += result;
        if (read == contentLength)
            break;
        if (mSleep > 0)
            QThread::msleep(mSleep);
        socket->waitForReadyRead(10);
    };

    return QString::fromUtf8(buffer);
}

QString LocalHttpClient::postSlow(const QString &url, const QHttpHeaders &headers, qsizetype mSleep)
{
    Q_ASSERT(socket);
    qint64 result = socket->write(u"POST %1 %2\r\n"_s.arg(url, version).toUtf8());
    QVERIFY2(result != -1, "Error writing POST method");

    for (qsizetype i = 0; i < headers.size(); ++i) {
        QByteArray output;
        output.append(headers.nameAt(i));
        output.append(": ");
        output.append(headers.valueAt(i));
        output.append("\r\n");
        result = socket->write(output);
        QVERIFY2(result >= 0, "Error writing header line");

        QTimer timer;
        timer.setSingleShot(true);
        timer.start(mSleep);
        socket->waitForBytesWritten(mSleep);
        int remaining = timer.remainingTime();
        if (remaining > 0)
            QThread::msleep(remaining);
    }
    result = socket->write("\r\n");
    QVERIFY2(result >= 0, "Error ending headers");

    return fetchResults();
}

void LocalHttpClient::sendGet(const QString &url, const QHttpHeaders &headers)
{
    Q_ASSERT(socket);
    socket->write(u"GET %1 %2\r\n"_s.arg(url, version).toUtf8());
    for (qsizetype i = 0; i < headers.size(); ++i) {
        QByteArray output;
        output.append(headers.nameAt(i));
        output.append(": ");
        output.append(headers.valueAt(i));
        output.append("\r\n");
        socket->write(output);
    }
    socket->write("\r\n");
}

void LocalHttpClient::fetchStatusLine()
{
    Q_ASSERT(socket);
    constexpr qint64 bufferSize = 4 * 1024;
    char buffer[bufferSize];
    qint64 read = 0;
    while (!socket->canReadLine())
        socket->waitForReadyRead(10);
    read = socket->readLine(buffer, bufferSize);
    QVERIFY2(read > 2, "Status line too short");

    QByteArrayView line(buffer, read);
    auto space = line.indexOf(' ');
    QVERIFY2(space != -1, "Error parsing status line");

    auto httpVersion = line.first(space);
    QCOMPARE(httpVersion, version);
}

QString LocalHttpClient::fetchResults()
{
    Q_ASSERT(socket);
    fetchStatusLine();
    qint64 contentLength = -1;
    QByteArray transferEncoding;
    constexpr qint64 bufferSize = 4 * 1024;
    char buffer[bufferSize];
    qint64 read = 0;
    forever {
        while (!socket->canReadLine()) {
            socket->waitForReadyRead(10);
        }
        read = socket->readLine(buffer, bufferSize);
        QVERIFY2(read >= 0, "IO error reading header");
        if (read <= 2)
            break; // End of headers
        QByteArrayView line(buffer, read);
        auto colon = line.indexOf(':');
        if (colon != -1 && colon + 1 < line.size()) {
            auto headerTitle = line.first(colon);
            if (headerTitle.compare("Content-Length", Qt::CaseInsensitive) == 0)
                contentLength = line.sliced(colon + 1).trimmed().toLongLong();
            else if (headerTitle.compare("Transfer-Encoding", Qt::CaseInsensitive) == 0)
                transferEncoding = line.sliced(colon + 1).trimmed().toByteArray();
        }
    };

    QVERIFY2(contentLength < bufferSize, "Buffer too small");
    if (contentLength == 0)
        return u""_s; // No content

    if (contentLength > 0) {
        read = 0;
        while (read < contentLength) {
            socket->waitForReadyRead(10);
            qint64 result = socket->read(&buffer[read], contentLength - read);
            QVERIFY2(result >= 0, "IO Error reading content");
            read += result;
        };
    } else if (transferEncoding.compare("chunked", Qt::CaseInsensitive) == 0) {
        QVERIFY2(serverType != HTTP1_0, "Chunked encoding not supported for HTTP/1.0");
        read = 0;
        forever {
            while (!socket->canReadLine())
                socket->waitForReadyRead(10);
            QByteArray line = socket->readLine();
            qint64 result = line.trimmed().toInt(nullptr, 16);
            if (result == 0) {
                fetchCrLf();
                contentLength = read;
                break;
            }
            qint64 toRead = qMin(result, bufferSize - read);
            while (toRead > 0) {
                result = socket->read(&buffer[read], toRead);
                QVERIFY2(result >= 0, "Read error decoding chunked encoding");
                read += result;
                toRead -= result;
                if (toRead)
                    socket->waitForReadyRead(10);
            }
            fetchCrLf();
        };
    } else {
        // Read until closed
        read = 0;
        while (read < bufferSize) {
            socket->waitForReadyRead(10);
            qint64 result = socket->read(&buffer[read], bufferSize - read);
            if (result == -1) {
                contentLength = read;
                break;
            }
            read += result;
        };
    }
    return QString::fromUtf8(buffer, contentLength);
}

void LocalHttpClient::fetchCrLf()
{
    qint64 read = 0;
    char buffer[2];
    while (read < 2) {
        qint64 toRead = 2 - read;
        qint64 result = socket->read(&buffer[read], toRead);
        QVERIFY2(result >= 0, "Read error decoding chunked encoding");
        read += result;
        if (read < 2)
            socket->waitForReadyRead(10);
    }
    QVERIFY2(buffer[0] == '\r' && buffer[1] == '\n', "Read error decoding chunked encoding");
}

class SequentialIODevice : public QIODevice
{
    Q_OBJECT

public:
    SequentialIODevice(const QByteArray &data, int times, std::chrono::milliseconds readInterval)
        : message(data), times(times)
    {
        setOpenMode(QIODeviceBase::ReadOnly);
        timer = new QTimer(this);
        timer->callOnTimeout(this, &SequentialIODevice::onTimeout);
        timer->setSingleShot(false);
        timer->setInterval(readInterval);
        timer->start();
    }

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return buffer.size() - readPos; }
    qint64 bytesToWrite() const override { return 0; }

    bool seek(qint64) override
    {
        return false; // No random accesss on sequential devices
    }

    qint64 readData(char *data, qint64 maxSize) override
    {
        qint64 length = qMin(maxSize, buffer.size() - readPos);
        if (length == 0)
            return finishedReading ? -1 : 0;
        QVERIFY(length + readPos <= buffer.size());
        memcpy(data, buffer.constData() + readPos, length);
        readPos += length;

        if (readPos == buffer.size()) {
            readPos = 0;
            buffer.clear();
        }
        return length;
    }

    qint64 writeData(const char *, qint64) override { return -1; }

    void onTimeout()
    {
        if (times > 0) {
            buffer.append(message);
            emit readyRead();
        }

        if (--times <= 0) {
            timer->stop();
            finishedReading = true;
            emit readChannelFinished();
        }
    }

private:
    QTimer *timer;
    const QByteArray message;
    QByteArray buffer;
    qsizetype readPos = 0;
    int times;
    bool finishedReading = false;
};

class tst_QHttpServerMultithreaded final : public QObject
{
    Q_OBJECT

private:
    QString toUpper(const QString &input) const;
    QString toLower(const QString &input) const;
    QString toString(qsizetype input) const;
    void clearCallCount();
    qsizetype getCallCount() const;

private slots:
    void initTestCase_data();
    void initTestCase();
    void init();
    void singleCall_data();
    void singleCall();
    void multipleCallsOnEachConnection();
    void moreConnectionsThanServerThreads();
    void readSlow();
    void postSlow();
    void useSemaphores();
    void waitingInParallel();
    void waitPipelined();
    void waitPipelinedQnam();
    void manyWaitingToRespond();
    void oneSlowManyFast();
    void multipartBack();
    void sequentialDevice();

private:
    static constexpr qsizetype NumberOfThreads = 6;
    QSemaphore readySem, routeSem;
    QThreadPool threadPool;
    QHttpServer httpserver;
    mutable qsizetype callCounter = 0;
    mutable QMutex mutex;
};

void tst_QHttpServerMultithreaded::clearCallCount()
{
    QMutexLocker locker(&mutex);
    callCounter = 0;
}

qsizetype tst_QHttpServerMultithreaded::getCallCount() const
{
    QMutexLocker locker(&mutex);
    return callCounter;
}

QString tst_QHttpServerMultithreaded::toUpper(const QString &input) const
{
    QMutexLocker locker(&mutex);
    ++callCounter;
    return input.toUpper();
}

QString tst_QHttpServerMultithreaded::toLower(const QString &input) const
{
    QMutexLocker locker(&mutex);
    ++callCounter;
    return input.toLower();
}

QString tst_QHttpServerMultithreaded::toString(qsizetype input) const
{
    QMutexLocker locker(&mutex);
    ++callCounter;
    return QString::number(input);
}

void tst_QHttpServerMultithreaded::initTestCase_data()
{
    QTest::addColumn<ServerType>("serverType");
    QTest::addRow("HTTP/1.0") << ServerType::HTTP1_0;
    QTest::addRow("HTTP/1.1") << ServerType::HTTP1_1;
#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl() && !QTestPrivate::isSecureTransportBlockingTest())
        QTest::addRow("SSL") << ServerType::SSL;
    else if (QSslSocket::supportsSsl())
        qInfo("Not testing TLS, keychain access will block the test");
#endif
#if QT_CONFIG(localserver)
    QTest::addRow("LOCAL") << ServerType::LOCAL;
#endif
}

void tst_QHttpServerMultithreaded::initTestCase()
{
    threadPool.setMaxThreadCount(NumberOfThreads);
    httpserver.route("/convert-uppercase/<arg>", [&](const QString &input) {
        return QtConcurrent::run(&threadPool, [this, input] () {
            return QHttpServerResponse(toUpper(input));
        });
    });

    httpserver.route("/convert-lowercase/<arg>", [&](const QString &input) {
        return QtConcurrent::run(&threadPool, [this, input] () {
            return QHttpServerResponse(toLower(input));
        });
    });

    httpserver.route("/wait/<arg>", [&](qsizetype wait) {
        return QtConcurrent::run(&threadPool, [this, wait] () {
            QThread::msleep(wait);
            return QHttpServerResponse(toString(wait));
        });
    });

    httpserver.route("/semroute/<arg>", [this](int i){
        return QtConcurrent::run(&threadPool, [this, i] () {
            readySem.release();
            routeSem.acquire();
            return QHttpServerResponse(QString::number(i));
        });
    });

    httpserver.route("/headers/", QHttpServerRequest::Method::Post,
                     [this](const QHttpServerRequest &request) {
                         auto headers = request.headers().toListOfPairs();
                         return QtConcurrent::run(&threadPool, [headers]() {
                             QString result;
                             for (auto header : headers)
                                 result += QString::fromUtf8(header.first) + "\n";
                             return QHttpServerResponse(result);
                         });
                     });

    httpserver.route("/multipart/<arg>/<arg>",
                     [this](QString message, int times, QHttpServerResponder &responder) {
                         ++callCounter;
                         if (times > 0) {
                             QByteArray ba = message.toUtf8();
                             responder.writeBeginChunked("text/plain"_ba);
                             for (int i = 1; i < times; ++i)
                                 responder.writeChunk(ba);
                             responder.writeEndChunked(ba);
                         } else {
                             responder.write();
                         }
                     });

    httpserver.route(
            "/sequential/<arg>/<arg>",
            [this](QString message, int times, QHttpServerResponder &&responder) {
                auto r = std::make_shared<QHttpServerResponder>(std::move(responder));
                return QtConcurrent::run(
                        &threadPool,
                        [this, message = std::move(message), times, r = std::move(r)]() {
                            ++callCounter;
                            auto device = new SequentialIODevice(message.toUtf8(), times, 50ms);
                            r->write(device, QHttpHeaders());
                        });
            });

    auto tcpserver = std::make_unique<QTcpServer>();
    QVERIFY2(tcpserver->listen(), "HTTP server listen failed");
    port = tcpserver->serverPort();
    QVERIFY2(httpserver.bind(tcpserver.get()), "HTTP server bind failed");
    tcpserver.release();

#if QT_CONFIG(localserver)
    auto localserver = std::make_unique<QLocalServer>();
#ifdef Q_OS_LINUX
    localserver->setSocketOptions(QLocalServer::AbstractNamespaceOption);
#endif
    localserver->removeServer(local);
    QVERIFY2(localserver->listen(local), "Local server listen failed");
    QVERIFY2(httpserver.bind(localserver.get()), "Local server bind failed");
    localserver.release();
#endif
#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl()) {
        auto sslserver = std::make_unique<QSslServer>();
        QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
        serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
        serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
        sslserver->setSslConfiguration(serverConfig);
        QVERIFY2(sslserver->listen(), "HTTPS server listen failed");
        sslPort = sslserver->serverPort();
        QVERIFY2(httpserver.bind(sslserver.get()), "HTTPS server bind failed");
        sslserver.release();
    }
#endif
}

void tst_QHttpServerMultithreaded::init()
{
    clearCallCount();
}

void tst_QHttpServerMultithreaded::singleCall_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("result");

    QTest::addRow("Downcase title case")
        << "/convert-lowercase/Hey"
        << "hey";

    QTest::addRow("Upcase title case")
        << "/convert-uppercase/Hey"
        << "HEY";

    QTest::addRow("Downcase lower case")
        << "/convert-lowercase/hey"
        << "hey";

    QTest::addRow("Upcase lower case")
        << "/convert-uppercase/hey"
        << "HEY";

    QTest::addRow("Downcase upper case")
        << "/convert-lowercase/HEY"
        << "hey";

    QTest::addRow("Upcase upper case")
        << "/convert-uppercase/HEY"
        << "HEY";
}

void tst_QHttpServerMultithreaded::singleCall()
{
    QFETCH_GLOBAL(ServerType, serverType);
    QFETCH(QString, url);
    QFETCH(QString, result);

    QFuture<QString> future = QtConcurrent::run([&]() {
        LocalHttpClient client(serverType);
        return client.get(url);
    });

    // Wait for incoming requests to be handled in main thread before they are
    // processed in separate threads
    QTRY_VERIFY(future.isFinished());

    QString returned = future.result();
    QCOMPARE(returned, result);
    QCOMPARE(getCallCount(), 1);
}

void tst_QHttpServerMultithreaded::multipleCallsOnEachConnection()
{
    QFETCH_GLOBAL(ServerType, serverType);
    QList<QString> inputs(100);
    for (qsizetype i = 0; i < inputs.size(); ++i)
        inputs[i] = u"Hey%1"_s.arg(i);

    QThreadPool clientThreadPool;
    clientThreadPool.setMaxThreadCount(2);

    QFuture<QList<QString>> futureLower = QtConcurrent::run(&clientThreadPool, [&]() {
        LocalHttpClient client(serverType);
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
        QList<QString> results;
        for (auto &input : inputs)
            results.push_back(client.get(u"/convert-lowercase/%1"_s.arg(input), headers));
        return results;
    });

    QFuture<QList<QString>> futureUpper = QtConcurrent::run(&clientThreadPool, [&]() {
        LocalHttpClient client(serverType);
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
        QList<QString> results;
        for (auto &input : inputs)
            results.push_back(client.get(u"/convert-uppercase/%1"_s.arg(input), headers));
        return results;
    });

    QTRY_VERIFY_WITH_TIMEOUT(futureLower.isFinished() && futureUpper.isFinished(), 10s);

    QList<QString> convertedToLower = futureLower.result();
    QList<QString> convertedToUpper = futureUpper.result();
    QCOMPARE(convertedToLower.size(), inputs.size());
    QCOMPARE(convertedToUpper.size(), inputs.size());
    QCOMPARE(getCallCount(), inputs.size() * 2);
    for (qsizetype i = 0; i < inputs.size(); ++i) {
        QCOMPARE(convertedToLower[i], inputs[i].toLower());
        QCOMPARE(convertedToUpper[i], inputs[i].toUpper());
    }
}

void tst_QHttpServerMultithreaded::moreConnectionsThanServerThreads()
{
    QFETCH_GLOBAL(ServerType, serverType);
    QList<QString> inputs(25);
    for (qsizetype i = 0; i < inputs.size(); ++i)
        inputs[i] = u"Hey%1"_s.arg(i);

    constexpr qsizetype NumberOfTasks = NumberOfThreads * 2;
    QThreadPool clientThreadPool;
    clientThreadPool.setMaxThreadCount(NumberOfTasks);
    QList<QFuture<QList<std::pair<QString, QString>>>> futures(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        futures[i] = QtConcurrent::run(&clientThreadPool, [&]() {
            LocalHttpClient client(serverType);
            QList<std::pair<QString, QString>> results;
            QHttpHeaders headers;
            headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
            for (auto &input : inputs) {
                QString lower = client.get(u"/convert-lowercase/%1"_s.arg(input), headers);
                QString upper = client.get(u"/convert-uppercase/%1"_s.arg(input), headers);
                results.push_back(std::make_pair(lower, upper));
            }
            return results;
        });
    };

    QTRY_VERIFY_WITH_TIMEOUT(std::all_of(futures.begin(), futures.end(),
                                         [](auto &future) { return future.isFinished(); }),
                             10s);

    QCOMPARE(getCallCount(), inputs.size() * NumberOfTasks * 2);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QList<std::pair<QString, QString>> converted = futures[i].result();
        for (qsizetype j = 0; j < inputs.size(); ++j) {
            QCOMPARE(converted[j].first, inputs[j].toLower());
            QCOMPARE(converted[j].second, inputs[j].toUpper());
        }
    }
}

void tst_QHttpServerMultithreaded::readSlow()
{
    QFETCH_GLOBAL(ServerType, serverType);
    QString input(u"ReadThisSlowly"_s);
    constexpr qsizetype NumberOfTasks = NumberOfThreads;

    QList<QFuture<QString>> futures(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        futures[i] = QtConcurrent::run([&](QPromise<QString> &promise) {
            LocalHttpClient client(serverType);
            QHttpHeaders headers;
            headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
            promise.addResult(
                    client.getSlowRead(u"/convert-lowercase/%1"_s.arg(input), 10, 3000, headers));
            promise.addResult(
                    client.getSlowRead(u"/convert-uppercase/%1"_s.arg(input), 10, 1000, headers));
        });
    }

    QTRY_VERIFY_WITH_TIMEOUT(std::all_of(futures.begin(), futures.end(),
                                         [](auto &future) { return future.isFinished(); }),
                             20s);

    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QList<QString> returned = futures[i].results();
        QCOMPARE(returned.size(), 2);
        QCOMPARE(returned[0], input.toLower());
        QCOMPARE(returned[1], input.toUpper());
    }
    QCOMPARE(getCallCount(), 2 * NumberOfTasks);
}

void tst_QHttpServerMultithreaded::postSlow()
{
    QFETCH_GLOBAL(ServerType, serverType);
    constexpr qsizetype NumberOfTasks = NumberOfThreads;
    QHttpHeaders headers;
    QString headerName1 = "custom-header1";
    QString headerValue1 = "1";
    QString headerName2 = "custom-header2";
    QString headerValue2 = "2";
    headers.append(headerName1, headerValue1);
    headers.append(headerName2, headerValue2);

    QList<QFuture<QString>> futures(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        futures[i] = QtConcurrent::run([&](QPromise<QString> &promise) {
            LocalHttpClient client(serverType);
            promise.addResult(client.postSlow(u"/headers/"_s, headers, 3000));
        });
    }

    QTRY_VERIFY_WITH_TIMEOUT(std::all_of(futures.begin(), futures.end(),
                                         [](auto &future) { return future.isFinished(); }),
                             30s);

    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QList<QString> returned = futures[i].results();
        QCOMPARE(returned.size(), 1);
        auto headerNames = returned[0].split('\n', Qt::SkipEmptyParts);
        QCOMPARE(headerNames.size(), 2);
        QCOMPARE(headerNames[0], headerName1);
        QCOMPARE(headerNames[1], headerName2);
    }
}

void tst_QHttpServerMultithreaded::useSemaphores()
{
    if (NumberOfThreads < 1)
        QSKIP("This test is only run with at least one server thread");

    QFETCH_GLOBAL(ServerType, serverType);
    QCOMPARE(readySem.available(), 0);
    QCOMPARE(routeSem.available(), 0);

    constexpr qsizetype NumberProcessed = NumberOfThreads;
    QThreadPool clientThreadPool;
    clientThreadPool.setMaxThreadCount(NumberProcessed);
    QList<QFuture<QString>> futures(NumberProcessed);
    std::atomic<qsizetype> startedThreads = 0;
    for (qsizetype i = 0; i < NumberProcessed; ++i) {
        futures[i] = QtConcurrent::run(&clientThreadPool, [&, i]() {
            LocalHttpClient client(serverType);
            ++startedThreads;
            return client.get(u"/semroute/%1"_s.arg(i));
        });
    }

    QTest::qWait(2000);
    QTRY_COMPARE(startedThreads, NumberOfThreads);
    readySem.acquire(NumberProcessed);
    routeSem.release(NumberProcessed);

    QTRY_VERIFY(std::all_of(futures.begin(), futures.end(),
                            [](auto &future) { return future.isFinished(); }));

    for (qsizetype i = 0; i < NumberProcessed; ++i)
        QCOMPARE(futures[i].result(), QString::number(i));

    QCOMPARE(readySem.available(), 0);
    QCOMPARE(routeSem.available(), 0);
}

void tst_QHttpServerMultithreaded::waitingInParallel()
{
    QFETCH_GLOBAL(ServerType, serverType);
    QThreadPool clientThreadPool;
    clientThreadPool.setMaxThreadCount(2);

    QFuture<QString> future1 = QtConcurrent::run(&clientThreadPool, [&]() {
        LocalHttpClient client(serverType);
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
        QString result = client.get(u"/wait/2001"_s, headers);
        result += client.get(u"/wait/9"_s, headers);
        return result;
    });

    QFuture<QString> future2 = QtConcurrent::run(&clientThreadPool, [&]() {
        LocalHttpClient client(serverType);
        QHttpHeaders headers;
        headers.append(QHttpHeaders::WellKnownHeader::Connection, "keep-alive");
        QString result = client.get(u"/wait/2000"_s, headers);
        result += client.get(u"/wait/10"_s, headers);
        return result;
    });

    QTRY_VERIFY(future1.isFinished() && future2.isFinished());

    QCOMPARE(getCallCount(), 4);
}

void tst_QHttpServerMultithreaded::waitPipelined()
{
    QFETCH_GLOBAL(ServerType, serverType);
    if (serverType == HTTP1_0)
        QSKIP("Pipelining is not supported for HTTP/1.0");
    QList<qsizetype> waitTimes = { 3000, 1000, 1 };
    constexpr qsizetype NumberOfTasks = NumberOfThreads;

    QList<QFuture<QString>> futures(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        futures[i] = QtConcurrent::run([&](QPromise<QString> &promise) {
            LocalHttpClient client(serverType);
            for (auto times : waitTimes) {
                client.pipelinedSendGet(u"/wait/%1"_s.arg(times));
            }
            for (qsizetype j = 0; j < waitTimes.size(); ++j)
                promise.addResult(client.piplinedFetchResults());
        });
    }

    QTRY_VERIFY_WITH_TIMEOUT(std::all_of(futures.begin(), futures.end(),
                                         [](auto &future) { return future.isFinished(); }),
                             20s);

    QCOMPARE(getCallCount(), waitTimes.size() * NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QList<QString> returned = futures[i].results();
        QCOMPARE(returned.size(), waitTimes.size());
        for (qsizetype j = 0; j < waitTimes.size() ; ++j)
            QCOMPARE(returned[j], QString::number(waitTimes[j]));
    }
}

void tst_QHttpServerMultithreaded::waitPipelinedQnam()
{
    QFETCH_GLOBAL(ServerType, serverType);
    if (serverType != HTTP1_1)
        QSKIP("This test only supports HTTP/1.1 over TCP");

    QList<qsizetype> waitTimes = { 3000, 1000, 1 };
    constexpr qsizetype NumberOfTasks = NumberOfThreads;
    QNetworkAccessManager manager;
    QList<QList<QNetworkReply *>> repliesForTasks;
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        repliesForTasks.push_back(QList<QNetworkReply *>());
        for (auto times : waitTimes) {
            QUrl url;
            url.setScheme(QStringLiteral("http"));
            url.setHost("localhost");
            url.setPort(port);
            url.setPath(u"/wait/%1"_s.arg(times));
            QNetworkRequest request(url);
            request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, QVariant(true));
            auto *reply = manager.get(request);
            if (reply)
                repliesForTasks[i].push_back(reply);
        }
    }

    auto allRepliesFinished = [](QList<QNetworkReply *> &replies) {
        return std::all_of(replies.begin(), replies.end(),
                           [](QNetworkReply *reply) { return reply->isFinished(); });
    };

    QTRY_VERIFY_WITH_TIMEOUT(std::all_of(repliesForTasks.begin(), repliesForTasks.end(),
                                         [&](auto &task) { return allRepliesFinished(task); }),
                             10s);

    QCOMPARE(getCallCount(), waitTimes.size() * NumberOfTasks);
    QCOMPARE(repliesForTasks.size(), NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QList<QNetworkReply *> &replies = repliesForTasks[i];
        QCOMPARE(replies.size(), waitTimes.size());
        for (qsizetype j = 0; j < waitTimes.size(); ++j) {
            QNetworkReply *reply = replies[j];
            QCOMPARE(QString::fromUtf8(reply->readAll()), QString::number(waitTimes[j]));
            reply->close();
            reply->deleteLater();
        }
    }
}

void tst_QHttpServerMultithreaded::manyWaitingToRespond()
{
    constexpr qsizetype NumberOfTasks = NumberOfThreads * 3;
    if (NumberOfThreads < 1)
        QSKIP("This test is only run with at least one server thread");

    QFETCH_GLOBAL(ServerType, serverType);

    QList<QFuture<QString>> futures(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        futures[i] = QtConcurrent::run([&]() {
            LocalHttpClient client(serverType);
            return client.get(u"/wait/50"_s);
        });
    }

    QTRY_VERIFY(std::all_of(futures.begin(), futures.end(),
                            [](auto &future) { return future.isFinished(); }));

    QCOMPARE(getCallCount(), NumberOfTasks);
}

void tst_QHttpServerMultithreaded::oneSlowManyFast()
{
    constexpr qsizetype NumberOfFastTasks = NumberOfThreads - 1;
    if (NumberOfThreads <= 1)
        QSKIP("This test is only run with more than one server thread");

    QFETCH_GLOBAL(ServerType, serverType);

    QThreadPool clientThreadPool;
    clientThreadPool.setMaxThreadCount(NumberOfFastTasks + 1);

    QFuture<QString> slowFuture = QtConcurrent::run(&clientThreadPool, [&]() {
        LocalHttpClient client(serverType);
        return client.get(u"/wait/2000"_s);
    });

    QList<QFuture<QString>> fastFutures(NumberOfFastTasks);
    for (qsizetype i = 0; i < NumberOfFastTasks; ++i) {
        fastFutures[i] = QtConcurrent::run(&clientThreadPool, [&]() {
            LocalHttpClient client(serverType);
            return client.get(u"/wait/200"_s);
        });
    }

    QTRY_VERIFY(slowFuture.isFinished());
    QVERIFY(std::all_of(fastFutures.begin(), fastFutures.end(),
                        [](auto &future) { return future.isFinished(); }));
    QCOMPARE(getCallCount(), NumberOfFastTasks + 1);
}

void tst_QHttpServerMultithreaded::multipartBack()
{
    QFETCH_GLOBAL(ServerType, serverType);

    QFuture<QString> future = QtConcurrent::run([&]() {
        LocalHttpClient client(serverType);
        return client.get("/multipart/Hey/3");
    });

    QTRY_VERIFY(future.isFinished());

    QString returned = future.result();
    QCOMPARE(returned, u"HeyHeyHey"_s);
    QCOMPARE(getCallCount(), 1);
}

void tst_QHttpServerMultithreaded::sequentialDevice()
{
    QFETCH_GLOBAL(ServerType, serverType);

    QFuture<QString> future = QtConcurrent::run([&]() {
        LocalHttpClient client(serverType);
        return client.get("/sequential/Hey/3");
    });

    QTRY_VERIFY(future.isFinished());

    QString returned = future.result();
    QCOMPARE(returned, u"HeyHeyHey"_s);
    QCOMPARE(getCallCount(), 1);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerMultithreaded)

#include "tst_qhttpservermultithreaded.moc"
