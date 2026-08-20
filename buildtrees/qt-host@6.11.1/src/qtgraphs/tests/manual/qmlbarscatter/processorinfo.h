// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PROCESSORINFO_H
#define PROCESSORINFO_H

#include <QVector>
#include <Pdh.h>
#include <windows.h>

class ProcessorInfo
{
public:
    ProcessorInfo();
    ~ProcessorInfo();

    double updateTime();

private:
    const double m_maxSamples = 15.0;
    double m_normalizedUsage;

    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuTotal;
};

#endif // PROCESSORINFO_H
