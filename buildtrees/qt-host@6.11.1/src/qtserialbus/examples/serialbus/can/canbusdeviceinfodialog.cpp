// Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "canbusdeviceinfodialog.h"
#include "ui_canbusdeviceinfodialog.h"

#include <QCanBusDeviceInfo>

CanBusDeviceInfoDialog::CanBusDeviceInfoDialog(const QCanBusDeviceInfo &info, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CanBusDeviceInfoDialog)
{
    m_ui->setupUi(this);
    m_ui->deviceInfoBox->setDeviceInfo(info);

    connect(m_ui->okButton, &QPushButton::pressed, this, &QDialog::close);
}

CanBusDeviceInfoDialog::~CanBusDeviceInfoDialog()
{
    delete m_ui;
}
