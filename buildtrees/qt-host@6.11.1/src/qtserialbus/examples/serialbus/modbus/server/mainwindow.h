// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QButtonGroup>
#include <QMainWindow>
#include <QModbusServer>

QT_BEGIN_NAMESPACE

class QLineEdit;

namespace Ui {
class MainWindow;
class SettingsDialog;
}

QT_END_NAMESPACE

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onConnectButtonClicked();
    void onStateChanged(int state);

    void coilChanged(int id);
    void discreteInputChanged(int id);
    void bitChanged(int id, QModbusDataUnit::RegisterType table, bool value);

    void setRegister(const QString &value);
    void updateWidgets(QModbusDataUnit::RegisterType table, int address, int size);

    void onCurrentConnectTypeChanged(int);

    void handleDeviceError(QModbusDevice::Error newError);

private:
    void initActions();
    void setupDeviceData();
    void setupWidgetContainers();

    Ui::MainWindow *ui = nullptr;
    QModbusServer *modbusDevice = nullptr;

    QButtonGroup coilButtons;
    QButtonGroup discreteButtons;
    QHash<QString, QLineEdit *> registers;
    SettingsDialog *m_settingsDialog = nullptr;
};

#endif // MAINWINDOW_H
