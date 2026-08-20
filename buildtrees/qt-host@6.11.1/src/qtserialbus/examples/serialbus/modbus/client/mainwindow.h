// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModbusDataUnit>

QT_BEGIN_NAMESPACE

class QModbusClient;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class SettingsDialog;
class WriteRegisterModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initActions();
    QModbusDataUnit readRequest() const;
    QModbusDataUnit writeRequest() const;

private slots:
    void onConnectButtonClicked();
    void onModbusStateChanged(int state);

    void onReadButtonClicked();
    void onReadReady();

    void onWriteButtonClicked();
    void onReadWriteButtonClicked();

    void onConnectTypeChanged(int);
    void onWriteTableChanged(int);

private:
    Ui::MainWindow *ui = nullptr;
    QModbusClient *modbusDevice = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    WriteRegisterModel *writeModel = nullptr;
};

#endif // MAINWINDOW_H
