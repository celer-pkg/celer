// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QCanBusDevice>
#include <QCanBusDeviceInfo>

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui {
class ConnectDialog;
}
class QSettings;

QT_END_NAMESPACE

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    typedef QPair<QCanBusDevice::ConfigurationKey, QVariant> ConfigurationItem;

    struct Settings {
        QString pluginName;
        QString deviceInterfaceName;
        QList<ConfigurationItem> configurations;
        bool useConfigurationEnabled = false;
        bool useModelRingBuffer = true;
        int modelRingBufferSize = 1000;
        bool useAutoscroll = false;
    };

    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog() override;

    Settings settings() const;

private slots:
    void pluginChanged(const QString &plugin);
    void interfaceChanged(const QString &interface);
    void ok();
    void cancel();

private:
    QString configurationValue(QCanBusDevice::ConfigurationKey key);
    void restoreSettings();
    void revertSettings();
    void saveSettings();
    void updateSettings();

    Ui::ConnectDialog *m_ui = nullptr;
    Settings m_currentSettings;
    QList<QCanBusDeviceInfo> m_interfaces;
    QSettings *m_settings = nullptr;
};

#endif // CONNECTDIALOG_H
