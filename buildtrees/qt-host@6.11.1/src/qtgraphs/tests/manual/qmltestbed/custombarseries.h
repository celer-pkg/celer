// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CUSTOMBARSERIES_H
#define CUSTOMBARSERIES_H

#include <QQmlEngine>
#include <QtGraphs/qbarseries.h>
#include <QtGraphs/qbarset.h>
#include <QList>
#include <QTimer>

class CustomBarSeries : public QBarSeries
{
   Q_OBJECT
   QML_ELEMENT
public:
    CustomBarSeries(QBarSeries *parent = nullptr);

public Q_SLOTS:
    void updateData();

private:
    QList<QBarSet *> m_sets;
    QTimer m_updateTimer;
};

#endif // CUSTOMBARSERIES_H
