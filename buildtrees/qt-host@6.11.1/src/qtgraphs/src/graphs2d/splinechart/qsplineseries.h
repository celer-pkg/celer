// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSPLINESERIES_H
#define QTGRAPHS_QSPLINESERIES_H

#include <QtCore/qobject.h>
#include <QtGraphs/qxyseries.h>

QT_BEGIN_NAMESPACE

class QSplineSeriesPrivate;

class Q_GRAPHS_EXPORT QSplineSeries : public QXYSeries
{
    Q_OBJECT
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(Qt::PenCapStyle capStyle READ capStyle WRITE setCapStyle NOTIFY capStyleChanged FINAL)
    Q_PROPERTY(
        bool optimized READ isOptimized WRITE setOptimized NOTIFY optimizedChanged REVISION(6, 11))

    QML_NAMED_ELEMENT(SplineSeries)
public:
    explicit QSplineSeries(QObject *parent = nullptr);
    ~QSplineSeries() override;
    QAbstractSeries::SeriesType type() const override;


    qreal width() const;
    void setWidth(qreal newWidth);

    Qt::PenCapStyle capStyle() const;
    void setCapStyle(Qt::PenCapStyle newCapStyle);

    bool isOptimized() const;
    void setOptimized(bool optimized);

    QList<QPointF> &getControlPoints();

Q_SIGNALS:
    void widthChanged();
    void capStyleChanged();
    Q_REVISION(6, 11) void optimizedChanged(bool newOptimized);

protected:
    QSplineSeries(QSplineSeriesPrivate &dd, QObject *parent = nullptr);

    void componentComplete() override;

private:
    Q_DECLARE_PRIVATE(QSplineSeries)
    Q_DISABLE_COPY(QSplineSeries)

    friend class QSplineControlAnimation;
    friend class QGraphTransition;
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QSPLINESERIES_H
