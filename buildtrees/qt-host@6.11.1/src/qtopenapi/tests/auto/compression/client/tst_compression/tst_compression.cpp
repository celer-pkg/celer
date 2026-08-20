// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"
#include "qoaihttprequest.h"

#include "compressiontestmyapisuffix.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

class tst_Compression : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void localCompressionRoundtrip_data();
    void localCompressionRoundtrip();
    void generatorVersionCheck();
    void toggleCompressionParameters_data();
    void toggleCompressionParameters();
    void cleanupTestCase();

private:
    QProcess m_serverProcess;
};

void tst_Compression::initTestCase()
{
    if (m_serverProcess.state() != QProcess::ProcessState::Running) {
        m_serverProcess.start(SERVER_PATH);
        if (!m_serverProcess.waitForStarted()) {
            qFatal() << "Couldn't start the server: " << m_serverProcess.errorString();
            exit(EXIT_FAILURE);
        }
        // give the process some time to properly start up the server
        QThread::currentThread()->msleep(1000);
    }
}

void tst_Compression::localCompressionRoundtrip_data()
{
    using namespace QtOpenApiCommon::QOAIHttpRequestWorker;
    QTest::addColumn<CompressionType>("compressionType");

    QTest::newRow("gzip") << CompressionType::Gzip;
    QTest::newRow("deflate") << CompressionType::Deflate;
}

void tst_Compression::localCompressionRoundtrip()
{
    // Generate the data to compress
    static constexpr int BlockSize = 1024;
    static constexpr size_t MaxUChar = std::numeric_limits<uchar>::max();

    QByteArray originalData;
    originalData.reserve(MaxUChar * BlockSize);
    for (uchar c = 0; c < MaxUChar; ++c)
        originalData.append(BlockSize, c);

    using namespace QtOpenApiCommon::QOAIHttpRequestWorker;
    QFETCH(const CompressionType, compressionType);

    const QByteArray compressed = compressData(originalData, 9, compressionType);
    QCOMPARE_LE(compressed.size(), originalData.size());

    const QByteArray decompressed = decompressData(compressed, compressionType);

    QCOMPARE(decompressed, originalData);
}

void tst_Compression::generatorVersionCheck()
{
    const QString generatorVer = u"%1.%2.%3"_s.arg(OPENAPI_GENERATOR_VERSION_MAJOR)
                                              .arg(OPENAPI_GENERATOR_VERSION_MINOR)
                                              .arg(OPENAPI_GENERATOR_VERSION_PATCH);
    QCOMPARE_EQ(generatorVer, OPENAPI_GENERATOR_VERSION_STR);
}

class CustomNetworkAccessManager : public QNetworkAccessManager
{
public:
    CustomNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {}
    ~CustomNetworkAccessManager() override
    {}

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &originalReq,
                                 QIODevice *outgoingData = nullptr) override;

public:
    bool m_forceCompressedResponse = false;
    QByteArray m_acceptEncodingHeader;
};

QNetworkReply *CustomNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                         const QNetworkRequest &originalReq,
                                                         QIODevice *outgoingData)
{
    m_acceptEncodingHeader = originalReq.rawHeader("Accept-Encoding"_L1);
    if (m_forceCompressedResponse) {
        QNetworkRequest copy = originalReq;
        copy.setRawHeader("Accept-Encoding", "gzip, deflate");
        return QNetworkAccessManager::createRequest(op, copy, outgoingData);
    }
    return QNetworkAccessManager::createRequest(op, originalReq, outgoingData);
}

void tst_Compression::toggleCompressionParameters_data()
{
    QTest::addColumn<bool>("requestCompression");
    QTest::addColumn<bool>("responseCompression");
    QTest::addColumn<bool>("forceCompressedResponse");

    QTest::newRow("all_uncompressed") << false << false << false;
    QTest::newRow("compressed_request") << true << false << false;
    QTest::newRow("compressed_response") << false << true << false;
    QTest::newRow("all_compressed") << true << true << false;
    QTest::newRow("all_uncompressed_force_compressed_response") << false << false << true;
}

void tst_Compression::toggleCompressionParameters()
{
    QFETCH(const bool, requestCompression);
    QFETCH(const bool, responseCompression);
    QFETCH(const bool, forceCompressedResponse);

    CustomNetworkAccessManager manager;
    manager.m_forceCompressedResponse = forceCompressedResponse;
    QRestAccessManager restManager(&manager);

    CompressionNamespace::CompressionTestMyApiSuffix api;
    api.setRestAccessManager(&restManager);

    // compression is disabled by default
    QVERIFY(!api.requestCompressionEnabled());
    QVERIFY(!api.responseCompressionEnabled());

    api.setRequestCompressionEnabled(requestCompression);
    api.setResponseCompressionEnabled(responseCompression);

    const QString filePath = QDir::currentPath() + QDir::separator() + u"test.pdf"_s;
    static QByteArray expectedContent; // do not read the file every time
    if (expectedContent.isEmpty()) {
        QFile f(filePath);
        QVERIFY(f.open(QIODevice::ReadOnly));
        expectedContent = f.readAll();
        f.close();
    }

    QtOpenApiCommon::QOAIHttpFileElement file(filePath);

    bool done = false;
    QByteArray receivedData;
    int responseCode = -1;
    api.roundtrip(file, this,
                  [&](const QRestReply &reply,
                      const QtOpenApiCommon::QOAIHttpFileElement &summary) {
        done = reply.isSuccess();
        responseCode = reply.httpStatus();
        receivedData = summary.loadFromLocalFile();
    });
    if (responseCompression)
        QCOMPARE_EQ(manager.m_acceptEncodingHeader, "gzip, deflate");
    else
        QCOMPARE_EQ(manager.m_acceptEncodingHeader, "identity");

    QTRY_COMPARE_EQ(done, true);
    QCOMPARE_EQ(responseCode, 200);
    QCOMPARE_EQ(receivedData, expectedContent);
}

void tst_Compression::cleanupTestCase()
{
    if (m_serverProcess.state() == QProcess::ProcessState::Running) {
        m_serverProcess.kill();
        m_serverProcess.waitForFinished();
    }
}

QTEST_MAIN(tst_Compression)
#include "tst_compression.moc"
