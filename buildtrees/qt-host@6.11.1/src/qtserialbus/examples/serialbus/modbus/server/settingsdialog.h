// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtSerialBus/qtserialbusglobal.h>
#include <QDialog>
#if QT_CONFIG(modbus_serialport)
#include <QSerialPort>
#endif

QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
#if QT_CONFIG(modbus_serialport)
        int parity = QSerialPort::EvenParity;
        int baud = QSerialPort::Baud19200;
        int dataBits = QSerialPort::Data8;
        int stopBits = QSerialPort::OneStop;
#endif
    };

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings settings() const;

private:
    Settings m_settings;
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
