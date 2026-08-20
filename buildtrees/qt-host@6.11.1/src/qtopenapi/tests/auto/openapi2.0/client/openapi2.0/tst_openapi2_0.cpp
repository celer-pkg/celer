// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/fakeapi.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

class LoggingNetworkAccessManager : public QNetworkAccessManager
{
public:
    LoggingNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {}
    ~LoggingNetworkAccessManager() override
    {}

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &originalReq,
                                 QIODevice *outgoingData = nullptr) override;

public:
    QString m_operationPath;
};

QNetworkReply *LoggingNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                          const QNetworkRequest &originalReq,
                                                          QIODevice *outgoingData)
{
    const QUrl fullUrl = originalReq.url();
    // we only need the path and query parameters
    m_operationPath = fullUrl.path(QUrl::FullyEncoded);
    if (fullUrl.hasQuery())
        m_operationPath += u'?' + fullUrl.query(QUrl::FullyEncoded);

    return QNetworkAccessManager::createRequest(op, originalReq, outgoingData);
}

namespace QtOpenAPI {

static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

QString getStatusString(const QString &summary)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        return obj.value("status").toString();
    }
    return QString();
}

class OperationParametersBackport : public FakeApi {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();

        m_manager = new LoggingNetworkAccessManager(this);
        m_restManager = new QRestAccessManager(m_manager, this);
        setRestAccessManager(m_restManager);
    }
    void testCollectionFormats();
    void cleanupTestCase();

private:
    LoggingNetworkAccessManager *m_manager = nullptr;
    QRestAccessManager *m_restManager = nullptr;
};

void OperationParametersBackport::testCollectionFormats()
{
    QList<QString> pipe, ioutil, http, url, context;

    for (auto i = 0; i < 3; i++)
        pipe.append("pipe" + QString::number(i));

    for (auto i = 0; i < 3; i++)
        ioutil.append("ioutil" + QString::number(i));

    for (auto i = 0; i < 3; i++)
        http.append("http" + QString::number(i));

    for (auto i = 0; i < 3; i++)
        url.append("url" + QString::number(i));

    for (auto i = 0; i < 3; i++)
        context.append("context" + QString::number(i));

    // NOTE: the CORE generator always returns 'collectionFormat=csv' for
    // 'collectionFormat: tsv', because there is no direct mapping
    // with the latest OpenApi versions.
    // The Qt6 client generator uses style=form for 'collectionFormat: tsv' case.
    bool done = false;
    testQueryParameterCollectionFormat(pipe, ioutil, http, url, context, this, [&](const QRestReply &reply, const QString &summary) {
        if ((done = reply.isSuccess())) {
            QString expected("/v2/fake/test-query-parameters?pipe=pipe0%7Cpipe1%7Cpipe2&ioutil=ioutil0,ioutil1,ioutil2&http=http0%20http1%20http2&url=url0,url1,url2&multiContext=context0&multiContext=context1&multiContext=context2");
            QCOMPARE(m_manager->m_operationPath, expected);
            QCOMPARE(getStatusString(summary), expected);
        } else {
            qWarning() << "testQueryParameterCollectionFormat Error: " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ(done, true);

    // NOTE: the CORE generator always returns 'collectionFormat=csv' for all PATH parameters,
    // no matter what we specify in the yaml file.
    // So the Qt6 client generator uses style=simple for all of them.
    done = false;
    testPathParameterCollectionFormat(pipe, ioutil, http, url, context, this, [&](const QRestReply &reply, const QString &summary) {
        if ((done = reply.isSuccess())) {
            QString expected("/v2/fake/test-path-parameters/pipe0,pipe1,pipe2/ioutil0,ioutil1,ioutil2/http0,http1,http2/url0,url1,url2/context0,context1,context2");
            QCOMPARE(m_manager->m_operationPath, expected);
            QCOMPARE(getStatusString(summary), expected);
        } else {
            qWarning() << "testPathParameterCollectionFormat Error: " << reply.errorString();
        }

    });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParametersBackport::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::OperationParametersBackport)
#include "tst_openapi2_0.moc"
