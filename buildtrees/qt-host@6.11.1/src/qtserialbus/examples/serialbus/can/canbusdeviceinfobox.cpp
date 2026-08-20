// Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "canbusdeviceinfobox.h"
#include "ui_canbusdeviceinfobox.h"

#include <QCanBusDeviceInfo>

CanBusDeviceInfoBox::CanBusDeviceInfoBox(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::CanBusDeviceInfoBox)
{
    m_ui->setupUi(this);

    auto setReadOnlyAndCompact = [](QCheckBox *box) {
        box->setAttribute(Qt::WA_TransparentForMouseEvents);
        box->setFocusPolicy(Qt::NoFocus);
        box->setStyleSheet("margin-top:0; margin-bottom:0;");
    };
    setReadOnlyAndCompact(m_ui->isVirtual);
    setReadOnlyAndCompact(m_ui->isFlexibleDataRateCapable);
}

CanBusDeviceInfoBox::~CanBusDeviceInfoBox()
{
    delete m_ui;
}

void CanBusDeviceInfoBox::clear()
{
    m_ui->pluginLabel->clear();
    m_ui->nameLabel->clear();
    m_ui->descriptionLabel->clear();
    m_ui->serialNumberLabel->clear();
    m_ui->aliasLabel->clear();
    m_ui->channelLabel->clear();
    m_ui->isVirtual->setChecked(false);
    m_ui->isFlexibleDataRateCapable->setChecked(false);
}

void CanBusDeviceInfoBox::setDeviceInfo(const QCanBusDeviceInfo &info)
{
    m_ui->pluginLabel->setText(tr("Plugin: %1").arg(info.plugin()));
    m_ui->nameLabel->setText(tr("Name: %1").arg(info.name()));
    m_ui->descriptionLabel->setText(info.description());
    QString serialNumber = info.serialNumber();
    if (serialNumber.isEmpty())
        serialNumber = tr("n/a");
    m_ui->serialNumberLabel->setText(tr("Serial: %1").arg(serialNumber));
    QString alias = info.alias();
    if (alias.isEmpty())
        alias = tr("n/a");
    m_ui->aliasLabel->setText(tr("Alias: %1").arg(alias));
    m_ui->channelLabel->setText(tr("Channel: %1").arg(info.channel()));
    m_ui->isVirtual->setChecked(info.isVirtual());
    m_ui->isFlexibleDataRateCapable->setChecked(info.hasFlexibleDataRate());
}
