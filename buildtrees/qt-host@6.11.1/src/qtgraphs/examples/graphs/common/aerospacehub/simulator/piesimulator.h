// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIESIMULATOR_H
#define PIESIMULATOR_H

#include "datasimulator.h"

#include <QtQml/qqmlregistration.h>
#include <QPieSeries>

class PieSimulator : public DataSimulator
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PieSimulator(QObject *parent = 0);
    ~PieSimulator() override = default;

    Q_INVOKABLE void addSeries(QPieSeries *series);

Q_SIGNALS:
    void seriesChanged(QPieSeries *newSeries);

private:
    void generatePieSlices();

    QList<QPieSeries *> m_seriesList;
};

#endif // PIESIMULATOR_H
