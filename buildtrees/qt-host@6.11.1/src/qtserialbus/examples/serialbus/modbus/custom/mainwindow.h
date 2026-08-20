// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "modbusclient.h"
#include "modbusserver.h"

#include <QHash>
#include <QMainWindow>

QT_BEGIN_NAMESPACE

class QLineEdit;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class RegisterModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onConnectButtonClicked();
    void onStateChanged(int state);

    void onReadReady();
    void onReadButtonClicked();
    void onWriteButtonClicked();

    void setRegister(const QString &value);
    void updateWidgets(QModbusDataUnit::RegisterType table, int address, int size);

private:
    void setupConnections();
    void setupClientContainer();
    void setupServerContainer();

private:
    Ui::MainWindow *ui = nullptr;
    RegisterModel *m_model = nullptr;

    ModbusClient m_client;
    ModbusServer m_server;
    QHash<QString, QLineEdit *> m_registers;
};

#endif // MAINWINDOW_H
