// Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CANBUSDEVICEINFODIALOG_H
#define CANBUSDEVICEINFODIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE

class QCanBusDeviceInfo;

namespace Ui {
class CanBusDeviceInfoDialog;
}

QT_END_NAMESPACE

class CanBusDeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CanBusDeviceInfoDialog(const QCanBusDeviceInfo &info, QWidget *parent = nullptr);
    ~CanBusDeviceInfoDialog();

private:
    Ui::CanBusDeviceInfoDialog *m_ui = nullptr;
};

#endif // CANBUSDEVICEINFODIALOG_H
