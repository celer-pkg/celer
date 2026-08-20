// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SERVICE_H
#define SERVICE_H

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE
class QBluetoothAddress;
class QBluetoothServiceDiscoveryAgent;
class QBluetoothServiceInfo;

namespace Ui {
    class ServiceDiscovery;
}
QT_END_NAMESPACE

class ServiceDiscoveryDialog : public QDialog
{
    Q_OBJECT

public:
    ServiceDiscoveryDialog(const QString &name, const QBluetoothAddress &address,
                           QWidget *parent = nullptr);
    ~ServiceDiscoveryDialog();

public slots:
    void addService(const QBluetoothServiceInfo &info);

private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    Ui::ServiceDiscovery *ui;
};

#endif
