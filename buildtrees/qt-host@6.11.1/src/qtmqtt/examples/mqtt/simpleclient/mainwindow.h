// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMqttClient>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setClientPort(int p);
    void setSecure(bool s);
    void setWebSockets(bool ws);
    void setProtocol(QMqttClient::ProtocolVersion p);
private slots:
    void on_buttonConnect_clicked();
    void on_buttonQuit_clicked();
    void updateLogStateChange();

    void brokerDisconnected();

    void on_buttonPublish_clicked();

    void on_buttonSubscribe_clicked();

private:
    Ui::MainWindow *ui = nullptr;
    QMqttClient *m_client = nullptr;
    bool m_secure = false;
    bool m_webSockets = false;
    QMqttClient::ProtocolVersion m_protocol = QMqttClient::ProtocolVersion::MQTT_3_1_1;
};

#endif // MAINWINDOW_H
