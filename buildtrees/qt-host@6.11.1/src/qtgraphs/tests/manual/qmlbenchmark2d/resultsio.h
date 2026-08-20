// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RESULTSIO_H
#define RESULTSIO_H

#include <QObject>

class ResultsIO : public QObject
{
    Q_OBJECT

public:
    explicit ResultsIO(QObject *parent = nullptr);

    Q_INVOKABLE void saveResults(QString json);
    Q_INVOKABLE QString loadChartsResults();
    Q_INVOKABLE QString loadGraphsResults();
    Q_INVOKABLE bool useCharts();
};

#endif // RESULTSIO_H
