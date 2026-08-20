// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../shared/debugutil_p.h"
#include "../shared/qqmldebugprocess_p.h"
#include <QtQuickTestUtils/private/qmlutils_p.h>

#include <private/qqmldebugconnection_p.h>
#include <private/qqmlinspectorclient_p.h>

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>
#include <QtCore/qlibraryinfo.h>
#include <QtNetwork/qhostaddress.h>

class tst_QQmlInspector : public QQmlDebugTest
{
    Q_OBJECT

public:
    tst_QQmlInspector();

private:
    ConnectResult startQmlProcess(const QString &qmlFile, bool restrictMode = true);
    QList<QQmlDebugClient *> createClients() override;
    QQmlDebugProcess *createProcess(const QString &executable) override;

    QPointer<QQmlInspectorClient> m_client;
    QPointer<QQmlInspectorResultRecipient> m_recipient;

private slots:
    void connect_data();
    void connect();
    void setAnimationSpeed();
    void showAppOnTop();
};

tst_QQmlInspector::tst_QQmlInspector()
    : QQmlDebugTest(QT_QMLTEST_DATADIR)
{
}

QQmlDebugTest::ConnectResult tst_QQmlInspector::startQmlProcess(const QString &qmlFile,
                                                                bool restrictServices)
{
    return QQmlDebugTest::connectTo(QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/qml",
                                  restrictServices ? QStringLiteral("QmlInspector") : QString(),
                                  testFile(qmlFile), true);
}

QList<QQmlDebugClient *> tst_QQmlInspector::createClients()
{
    m_client = new QQmlInspectorClient(m_connection);
    m_recipient = new QQmlInspectorResultRecipient(m_client);
    QObject::connect(m_client.data(), &QQmlInspectorClient::responseReceived,
                     m_recipient.data(), &QQmlInspectorResultRecipient::recordResponse);
    return QList<QQmlDebugClient *>({m_client});
}

QQmlDebugProcess *tst_QQmlInspector::createProcess(const QString &executable)
{
    QQmlDebugProcess *process = QQmlDebugTest::createProcess(executable);
    // Make sure the animation timing is exact
    process->addEnvironment(QLatin1String("QSG_RENDER_LOOP=basic"));
    return process;
}

void tst_QQmlInspector::connect_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<bool>("restrictMode");
    QTest::newRow("rectangle/unrestricted") << "qtquick2.qml" << false;
    QTest::newRow("rectangle/restricted")   << "qtquick2.qml" << true;
    QTest::newRow("window/unrestricted")    << "window.qml"   << false;
    QTest::newRow("window/restricted")      << "window.qml"   << true;
}

void tst_QQmlInspector::connect()
{
    QFETCH(QString, file);
    QFETCH(bool, restrictMode);
    QCOMPARE(startQmlProcess(file, restrictMode), ConnectSuccess);
    QVERIFY(m_client);
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::Enabled);

    int requestId = m_client->setInspectToolEnabled(true);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);

    requestId = m_client->setInspectToolEnabled(false);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
}

void tst_QQmlInspector::showAppOnTop()
{
    QCOMPARE(startQmlProcess("qtquick2.qml"), ConnectSuccess);
    QVERIFY(m_client);
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::Enabled);

    int requestId = m_client->setShowAppOnTop(true);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);

    requestId = m_client->setShowAppOnTop(false);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
}

void tst_QQmlInspector::setAnimationSpeed()
{
    QCOMPARE(startQmlProcess("qtquick2.qml"), ConnectSuccess);
    QVERIFY(m_client);
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::Enabled);
    checkAnimationSpeed(m_process, 10);

    int requestId = m_client->setAnimationSpeed(0.5);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
    checkAnimationSpeed(m_process, 5);

    requestId = m_client->setAnimationSpeed(2.0);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
    checkAnimationSpeed(m_process, 20);

    requestId = m_client->setAnimationSpeed(1.0);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
    checkAnimationSpeed(m_process, 10);
}

QTEST_MAIN(tst_QQmlInspector)

#include "tst_qqmlinspector.moc"
