// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BARSERIESHELPER_H
#define BARSERIESHELPER_H

#include "cpuusageupdater.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtGraphs/QBarCategoryAxis>
#include <QtGraphs/QBarSeries>
#include <QtGraphs/QValueAxis>

class BarSeries : public QBarSeries
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CustomBar)

public:
    BarSeries(QObject *parent = nullptr);

public Q_SLOTS:
    void frameUpdate();

private:
    QBarSet *m_barList;
    QTimer m_timer;
    CpuUsageUpdater m_cpuUpdater;
    int m_counter;
};

#endif // BARSERIESHELPER_H
