// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "serviceinfo.h"

#include <QBluetoothUuid>
#include <QLowEnergyService>

using namespace Qt::StringLiterals;

ServiceInfo::ServiceInfo(QLowEnergyService *service):
    m_service(service)
{
    m_service->setParent(this);
}

QLowEnergyService *ServiceInfo::service() const
{
    return m_service;
}

QString ServiceInfo::getName() const
{
    if (!m_service)
        return QString();

    return m_service->serviceName();
}

QString ServiceInfo::getType() const
{
    if (!m_service)
        return QString();

    QString result;
    if (m_service->type() & QLowEnergyService::PrimaryService)
        result += u"primary"_s;
    else
        result += u"secondary"_s;

    if (m_service->type() & QLowEnergyService::IncludedService)
        result += u" included"_s;

    result.prepend('<'_L1).append('>'_L1);

    return result;
}

QString ServiceInfo::getUuid() const
{
    if (!m_service)
        return QString();

    const QBluetoothUuid uuid = m_service->serviceUuid();
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return u"0x"_s + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success)
        return u"0x"_s + QString::number(result32, 16);

    return uuid.toString().remove('{'_L1).remove('}'_L1);
}
