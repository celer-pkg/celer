// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCATTERSERIES_H
#define SCATTERSERIES_H

#include "cpuusageupdater.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtGraphs/QScatterSeries>
#include <QtQml/QQmlEngine>

class ScatterSeries : public QScatterSeries
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CustomScatter)

public:
    ScatterSeries(QScatterSeries *series = nullptr);

public Q_SLOTS:
    void frameUpdate();

private:
    QTimer m_timer;
    CpuUsageUpdater m_cpuUpdater;
    int m_counter = 0;
};

#endif // SCATTERSERIES_H
