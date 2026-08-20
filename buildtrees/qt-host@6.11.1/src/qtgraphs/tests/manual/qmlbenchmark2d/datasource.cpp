// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "datasource.h"

#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QXYSeries>
#include <QtMath>

DataSource::DataSource(QObject *parent)
    : QObject(parent)
{}

void DataSource::reset(int dataScale)
{
    m_dataScale = dataScale;
    m_testIndex = 0;
    m_timer = QDateTime::currentMSecsSinceEpoch();

    generateData();
}

void DataSource::update(QObject *series)
{
    if (QDateTime::currentMSecsSinceEpoch() - m_timer >= 4000 && m_testIndex + 1 < m_data.size()) {
        m_timer = QDateTime::currentMSecsSinceEpoch();
        m_testIndex++;
    }

    if (series) {
        m_index++;
        if (m_index > m_data.at(m_testIndex).count() - 1)
            m_index = 0;

        QList<QPointF> points = m_data.at(m_testIndex).at(m_index);
        if (auto xySeries = qobject_cast<QXYSeries *>(series)) {
            xySeries->replace(points);
            //xySeries->update(); // Uncomment for 6.8 release test
        } else if (auto barSeries = qobject_cast<QBarSeries *>(series)) {
            if (barSeries->count() <= 0) {
                auto set = new QBarSet("Set", barSeries);
                barSeries->append(set);
            }

            QBarSet *set = barSeries->at(0);
            for (qsizetype i = 0; i < points.size(); i++) {
                if (set->count() <= i)
                    set->append(points[i].y());
                else
                    set->replace(i, points[i].y());
            }
        }
    }
}

void DataSource::generateData()
{
    int type = 0;
    int testCount = 7;
    int rowCount = 5;
    // Remove previous data
    m_data.clear();

    // Append the new data depending on the type
    for (int k(0); k < testCount; k++) {
        int colCount = 0;
        if (k > 0)
            colCount = qPow(2, k - 1) * m_dataScale;

        QList<QList<QPointF>> test;
        for (int i(0); i < rowCount; i++) {
            QList<QPointF> points;
            points.reserve(colCount);
            for (int j(0); j < colCount; j++) {
                qreal x(0);
                qreal y(0);
                switch (type) {
                case 0:
                    // data with sin + random component
                    y = qSin(M_PI / 50 * j) + 5 + QRandomGenerator::global()->generateDouble();
                    x = j;
                    break;
                case 1:
                    // linear data
                    x = j;
                    y = (qreal) i / 10;
                    break;
                default:
                    // unknown, do nothing
                    break;
                }
                points.append(QPointF(x, y));
            }
            test.append(points);
        }
        m_data.append(test);
    }
}
