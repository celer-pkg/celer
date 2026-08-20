// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LINESIMULATOR_H
#define LINESIMULATOR_H

#include "datasimulator.h"

#include <QSplineSeries>

class LineSimulator : public DataSimulator
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LineSimulator(QObject *parent = 0);
    ~LineSimulator() override = default;

    Q_INVOKABLE void addSeries(QSplineSeries *series);

    void generateData() override;

Q_SIGNALS:
    void seriesChanged(QSplineSeries *newSeries);

private:
    void generatePoints();

    QList<QSplineSeries *> m_seriesList;
};

#endif // LINESIMULATOR_H
