// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <QObject>
#include <QtGraphs>

class RandomGenerator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int activeSeriesIndex READ activeSeriesIndex WRITE setActiveSeriesIndex NOTIFY activeSeriesIndexChanged)
    Q_PROPERTY(QSurface3DSeries *surface3DSeries READ surface3DSeries WRITE setSurface3DSeries)
    Q_PROPERTY(QScatter3DSeries *scatter3DSeries READ scatter3DSeries WRITE setScatter3DSeries)
    Q_PROPERTY(QSpline3DSeries *spline3DSeries READ spline3DSeries WRITE setSpline3DSeries)
    Q_PROPERTY(QLineSeries *lineSeries  READ lineSeries WRITE setLineSeries)
    Q_PROPERTY(QSplineSeries *splineSeries READ splineSeries WRITE setSplineSeries)
    Q_PROPERTY(QScatterSeries *scatterSeries READ scatterSeries WRITE setScatterSeries)

public:
    RandomGenerator(QObject *parent = nullptr);

    int rowCount() const;
    void setRowCount(int rowCount);

    int activeSeriesIndex() const;
    void setActiveSeriesIndex(int index);

    QSurface3DSeries *surface3DSeries() const {return m_surface3D;};
    void setSurface3DSeries(QSurface3DSeries *series) {m_surface3D = series;};

    QScatter3DSeries *scatter3DSeries() const {return m_scatter3D;};
    void setScatter3DSeries(QScatter3DSeries *series) {m_scatter3D = series;};

    QSpline3DSeries *spline3DSeries() const {return m_spline3D;};
    void setSpline3DSeries(QSpline3DSeries *series) {m_spline3D = series;};

    QLineSeries *lineSeries() const {return m_line2D;};
    void setLineSeries(QLineSeries *series) {m_line2D = series;};

    QSplineSeries *splineSeries() const {return m_spline2D;};
    void setSplineSeries(QSplineSeries *series) {m_spline2D = series;};

    QScatterSeries *scatterSeries() const {return m_scatter2D;};
    void setScatterSeries(QScatterSeries *series) {m_scatter2D = series;};

    Q_INVOKABLE void nextCache();
Q_SIGNALS:
    void rowCountChanged(int rowCount);
    void activeSeriesIndexChanged(int index);

private:
    QSurface3DSeries *m_surface3D = nullptr;
    QScatter3DSeries *m_scatter3D = nullptr;
    QSpline3DSeries *m_spline3D = nullptr;
    QLineSeries *m_line2D = nullptr;
    QSplineSeries *m_spline2D = nullptr;
    QScatterSeries *m_scatter2D = nullptr;

    int m_activeSeriesIndex = 0;
    int m_rowCount;
    int m_cacheCount = 60;
    int m_currentCache = 0;
    QVector<QList<qreal>> m_data;

    void generateCaches();
    void clearCaches();

    void updateSurface3D();
    void updateScatter3D(int index);
    void update2DGraph(int index);
};
#endif // !RANDOMGENERATOR_H
