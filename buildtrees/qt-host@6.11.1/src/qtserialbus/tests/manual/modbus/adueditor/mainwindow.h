// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_interface.h"

#include <QMainWindow>
#include <QObject>

QT_BEGIN_NAMESPACE
class QModbusClient;
QT_END_NAMESPACE;

class DebugHandler
{
public:
    explicit DebugHandler(QtMessageHandler newMessageHandler)
        : oldMessageHandler(qInstallMessageHandler(newMessageHandler))
    {}
    ~DebugHandler() { qInstallMessageHandler(oldMessageHandler); }

private:
    QtMessageHandler oldMessageHandler;
};

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
    friend class ModbusTcpClient;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *instance();
    void appendToLog(const QString &msg) {
        logTextEdit->appendPlainText(msg);
    }

private slots:
    void on_sendButton_clicked();
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_serialPortCombo_currentIndexChanged(int index);

private:
    void disconnectAndDelete();

private:
    DebugHandler m_debugHandler;
    QModbusClient *m_device = nullptr;
};

#endif // MAINWINDOW_H
