// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DSCATTER_H
#define QTDATAVIS3D_Q3DSCATTER_H

#include <QtDataVisualization/qabstract3dgraph.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qscatter3dseries.h>

QT_BEGIN_NAMESPACE

class Q3DScatterPrivate;

class Q_DATAVISUALIZATION_EXPORT Q3DScatter : public QAbstract3DGraph
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QScatter3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)

public:
    explicit Q3DScatter(const QSurfaceFormat *format = nullptr, QWindow *parent = nullptr);
    virtual ~Q3DScatter();

    void addSeries(QScatter3DSeries *series);
    void removeSeries(QScatter3DSeries *series);
    QList<QScatter3DSeries *> seriesList() const;

    void setAxisX(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    void setAxisY(QValue3DAxis *axis);
    QValue3DAxis *axisY() const;
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisZ() const;
    void addAxis(QValue3DAxis *axis);
    void releaseAxis(QValue3DAxis *axis);
    QList<QValue3DAxis *> axes() const;

    QScatter3DSeries *selectedSeries() const;

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QScatter3DSeries *series);

private:
    Q3DScatterPrivate *dptr();
    const Q3DScatterPrivate *dptrc() const;
    Q_DISABLE_COPY(Q3DScatter)
};

QT_END_NAMESPACE

#endif
