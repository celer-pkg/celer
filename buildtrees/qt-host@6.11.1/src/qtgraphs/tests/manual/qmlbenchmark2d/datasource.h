// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QList>
#include <QObject>
#include <QPointF>

QT_FORWARD_DECLARE_CLASS(QQuickView)

class DataSource : public QObject
{
    Q_OBJECT

public:
    explicit DataSource(QObject *parent = nullptr);

public slots:
    void generateData();
    void reset(int dataScale);
    void update(QObject *series);

private:
    QList<QList<QList<QPointF>>> m_data;
    int m_index = -1;
    int m_testIndex = 0;
    qint64 m_timer = 0;
    int m_dataScale = 1000;
};

#endif
