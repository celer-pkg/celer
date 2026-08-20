// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef REMOTESELECTOR_H
#define REMOTESELECTOR_H

#include <QDialog>

#include <QBluetoothServiceInfo>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QBluetoothServiceDiscoveryAgent;
class QBluetoothUuid;
class QListWidgetItem;

namespace Ui {
    class RemoteSelector;
}
QT_END_NAMESPACE

class RemoteSelector : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteSelector(const QBluetoothAddress &localAdapter, QWidget *parent = nullptr);
    ~RemoteSelector();

    void startDiscovery(const QBluetoothUuid &uuid);
    void stopDiscovery();
    QBluetoothServiceInfo service() const;

private:
    Ui::RemoteSelector *ui;

    QBluetoothServiceDiscoveryAgent *m_discoveryAgent;
    QBluetoothServiceInfo m_service;
    QMap<QListWidgetItem *, QBluetoothServiceInfo> m_discoveredServices;

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();
    void updateIcon(Qt::ColorScheme scheme);
    void on_remoteDevices_itemActivated(QListWidgetItem *item);
    void on_remoteDevices_itemClicked(QListWidgetItem *item);
    void on_cancelButton_clicked();
    void on_connectButton_clicked();
};

#endif // REMOTESELECTOR_H
