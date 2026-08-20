// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest>

#include <message_latency_defs.h>
#include <server_proc_runner.h>

class GrpcClientServerStreamQml : public QObject
{
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        // Initialization requiring the QQmlEngine to be constructed
        engine->rootContext()->setContextProperty("testMessageLatencyWithThreshold",
                                                  QVariant::fromValue(MessageLatencyWithThreshold));
        if (m_serverProccess.state() != QProcess::ProcessState::Running) {
            qInfo() << "Restarting server";
            m_serverProccess.restart();
            QVERIFY2(m_serverProccess.state() == QProcess::ProcessState::Running,
                     "Precondition failed - Server cannot be started.");
        }
    }

private:
    ServerProcRunner m_serverProccess{ TEST_GRPC_SERVER_PATH };
};

QUICK_TEST_MAIN_WITH_SETUP(tst_grpc_client_serverstream_qml, GrpcClientServerStreamQml)

#include "tst_grpc_client_serverstream_qml.moc"
