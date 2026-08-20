// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BARSIMULATOR_H
#define BARSIMULATOR_H

#include "datasimulator.h"

#include <QtQml/qqmlregistration.h>
#include <QBarSeries>

class BarSimulator : public DataSimulator
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int numberOfCategory READ numberOfCategory WRITE setNumberOfCategory NOTIFY numberOfCategoryChanged)

public:
    explicit BarSimulator(QObject *parent = 0);
    ~BarSimulator() override = default;

    int numberOfCategory() const;
    void setNumberOfCategory(int numberOfRows);
    Q_INVOKABLE void addSeries(QBarSeries *newSeries);

    void generateData() override;

Q_SIGNALS:
    void numberOfCategoryChanged(int numberOfRows);
    void seriesChanged(QBarSeries *newSeries);

private:
    void generateBarSet();

    int m_numberOfCategory = 1;
    QList<QBarSeries *> m_seriesList;
};

#endif // BARSIMULATOR_H
