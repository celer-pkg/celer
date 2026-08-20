// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CPUUSAGEUPDATER_H
#define CPUUSAGEUPDATER_H

#include <QtCore/QObject>
#include <QtQml/QQmlEngine>

#include "processorinfo.h"

class CpuUsageUpdater : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal usage READ usage WRITE setUsage NOTIFY usageChanged FINAL)

public:
    CpuUsageUpdater(QObject *parent = nullptr);

    double update();

    qreal usage() const;
    void setUsage(qreal newUsage);

signals:
    void usageChanged();

private:
    qreal m_usage;

    ProcessorInfo m_processorInfo;
};

#endif // CPUUSAGEUPDATER_H
