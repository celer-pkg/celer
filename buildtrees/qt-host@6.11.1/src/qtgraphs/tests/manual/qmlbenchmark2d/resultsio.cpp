// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "resultsio.h"
#include <QFile>

ResultsIO::ResultsIO(QObject *parent)
    : QObject(parent)
{}

void ResultsIO::saveResults(QString json)
{
#if USE_CHARTS
    QFile file("charts.json");
#else
    QFile file("graphs.json");
#endif
    if (file.open(QIODevice::WriteOnly))
        file.write(json.toUtf8());
}

QString ResultsIO::loadChartsResults()
{
    QFile file("charts.json");
    if (file.open(QIODevice::ReadOnly))
        return file.readAll();
    return "";
}

QString ResultsIO::loadGraphsResults()
{
    QFile file("graphs.json");
    if (file.open(QIODevice::ReadOnly))
        return file.readAll();
    return "";
}

bool ResultsIO::useCharts()
{
#if USE_CHARTS
    return true;
#else
    return false;
#endif
}
