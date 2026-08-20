// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HEATERVIEW_H
#define HEATERVIEW_H

#include <QtRemoteObjects/QRemoteObjectNode>
#include <QtWidgets/QWidget>

#include <memory>

class BLEIoDevice;
class HeaterReplica;

QT_BEGIN_NAMESPACE
namespace Ui {
class HeaterView;
}
QT_END_NAMESPACE

class HeaterView : public QWidget
{
    Q_OBJECT

public:
    explicit HeaterView(BLEIoDevice *ioDevice, QWidget *parent = nullptr);
    ~HeaterView() override;

signals:
    void closeMe();
    void showMessage(const QString &message);

private:
    Ui::HeaterView *ui;
    QRemoteObjectNode m_node;
    std::unique_ptr<HeaterReplica> m_heater;
};

#endif
