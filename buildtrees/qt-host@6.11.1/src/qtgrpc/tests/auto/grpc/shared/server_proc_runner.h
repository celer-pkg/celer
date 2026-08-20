// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef SERVER_PROC_RUNNER_H
#define SERVER_PROC_RUNNER_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QFileInfo>
#include <QDir>

#include <chrono>
#include <memory>

class ServerProcRunner : public QObject
{
    constexpr static auto waitForServerLatency = std::chrono::milliseconds(5000);
    constexpr static auto waitForServerRead = std::chrono::milliseconds(2000);

public:
    ServerProcRunner(const QString &serverPath) : serverPath(serverPath) { start(); }
    ~ServerProcRunner() override { stop(); }

    void restart()
    {
        stop();
        start();
    }

    QProcess::ProcessState state() const
    {
        return serverProc ? serverProc->state() : QProcess::ProcessState::NotRunning;
    }

private:
    void start()
    {
        if (serverPath.isEmpty()) {
            qInfo() << "testserver binary is missing.";
            return;
        }
        serverProc = std::make_unique<QProcess>();
        serverProc->setReadChannel(QProcess::StandardError);

        QObject::connect(serverProc.get(), &QProcess::readyReadStandardOutput, this,
                         [this] { qInfo().noquote() << serverProc->readAllStandardOutput(); });

        serverProc->setProgram(serverPath);
        serverProc->setWorkingDirectory(QFileInfo(serverPath).dir().absolutePath());
        serverProc->setArguments({ "--latency", QString::number(QT_GRPC_TEST_MESSAGE_LATENCY) });
        serverProc->start();
        if (!serverProc->waitForStarted(waitForServerLatency.count())) {
            qInfo() << "Failed to start the server" << serverPath
                    << QString::number(serverProc->exitCode(), 16);
            return;
        }
        // Wait for the 'Server listening' log from the server
        if (!serverProc->waitForReadyRead(waitForServerRead.count())) {
            qInfo() << "Could not wait for ready read from the server" << serverPath;
            return;
        }
        auto serverData = serverProc->readAllStandardError();
        if (!serverData.startsWith("Server listening")) {
            qInfo() << "The server was not ready within the deadline" << serverPath;
            return;
        }

        // Connect remaining error logs to the server
        QObject::connect(serverProc.get(), &QProcess::readyReadStandardError, this,
                         [this] { qWarning().noquote() << serverProc->readAllStandardError(); });
        qInfo() << "Testserver started" << serverPath;
    }
    void stop()
    {
        if (serverProc && serverProc->state() == QProcess::ProcessState::Running) {
            serverProc->kill();
            serverProc->waitForFinished(waitForServerLatency.count());
        }
        qInfo() << "Testserver stopped" << serverPath;
    }

    const QString serverPath;
    std::unique_ptr<QProcess> serverProc;
};

#endif // SERVER_PROC_RUNNER_H
