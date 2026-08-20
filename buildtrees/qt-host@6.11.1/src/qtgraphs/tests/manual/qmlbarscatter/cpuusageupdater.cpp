// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "cpuusageupdater.h"

CpuUsageUpdater::CpuUsageUpdater(QObject *parent) {}

double CpuUsageUpdater::update()
{
    return m_processorInfo.updateTime();
}

qreal CpuUsageUpdater::usage() const
{
    return m_usage;
}

void CpuUsageUpdater::setUsage(qreal newUsage)
{
    if (m_usage == newUsage)
        return;
    m_usage = newUsage;
    emit usageChanged();
}
