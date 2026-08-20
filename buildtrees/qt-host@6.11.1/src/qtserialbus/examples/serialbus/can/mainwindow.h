// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCanBusDevice>
#include <QMainWindow>

class ConnectDialog;
class ReceivedFramesModel;

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QLabel;
class QTimer;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void processReceivedFrames();
    void sendFrame(const QCanBusFrame &frame) const;
    void processErrors(QCanBusDevice::CanBusError) const;
    void connectDevice();
    void busStatus();
    void disconnectDevice();
    void processFramesWritten(qint64);
    void onAppendFramesTimeout();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initActionsConnections();

    qint64 m_numberFramesWritten = 0;
    qint64 m_numberFramesReceived = 0;
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_written = nullptr;
    QLabel *m_received = nullptr;
    ConnectDialog *m_connectDialog = nullptr;
    std::unique_ptr<QCanBusDevice> m_canDevice;
    QTimer *m_busStatusTimer = nullptr;
    QTimer *m_appendTimer = nullptr;
    ReceivedFramesModel *m_model = nullptr;
};

#endif // MAINWINDOW_H
