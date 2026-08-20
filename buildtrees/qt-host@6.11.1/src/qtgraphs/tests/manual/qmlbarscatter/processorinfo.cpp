// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "processorinfo.h"

ProcessorInfo::ProcessorInfo()
    : m_normalizedUsage(1.0)
{
    PdhOpenQuery(NULL, NULL, &m_cpuQuery);
    PdhAddEnglishCounter(m_cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &m_cpuTotal);
}

ProcessorInfo::~ProcessorInfo()
{
    PdhCloseQuery(m_cpuQuery);
}

double ProcessorInfo::updateTime()
{
    PDH_FMT_COUNTERVALUE cVal;

    PdhCollectQueryData(m_cpuQuery);
    PdhGetFormattedCounterValue(m_cpuTotal, PDH_FMT_DOUBLE, NULL, &cVal);

    m_normalizedUsage = (((m_maxSamples - 1) * m_normalizedUsage) + cVal.doubleValue)
                        / m_maxSamples;

    return m_normalizedUsage;
}
