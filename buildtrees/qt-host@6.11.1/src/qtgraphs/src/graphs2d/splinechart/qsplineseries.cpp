// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qsplineseries.h>
#include <private/qgraphsview_p.h>
#include <private/qsplineseries_p.h>
#include <private/qxypoint_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
               "class QSplineSeries;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DSplineSeriesCalculateControlPoints_entry, int pointCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DSplineSeriesCalculateControlPoints_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DSplineSeriesCalculateSplinePoints_entry, int controlPointCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DSplineSeriesCalculateSplinePoints_exit);

/*!
    \class QSplineSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief QSplineSeries presents data in spline graphs.

    The graph displays smooth spline segments that moves through all the points
    defined in the graph.
 */
/*!
    \qmltype SplineSeries
    \nativetype QSplineSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits XYSeries

    \brief SplineSeries presents data in spline graphs.

    The graph displays smooth spline segments that moves through all the points
    defined in the graph.

    \image graphs2d-spline.png
*/

/*!
    \property QSplineSeries::width

    The width of the spline line in pixels. The default value is 2.0.
*/
/*!
    \qmlproperty real SplineSeries::width
    The width of the line. By default, the width is 2.0.
*/

/*!
    \property QSplineSeries::capStyle

    Controls the cap style of the spline line. The value is one of Qt::PenCapStyle enum values.
    The default value is Qt::SquareCap.

    \sa Qt::PenCapStyle
*/
/*!
    \qmlproperty Qt::PenCapStyle SplineSeries::capStyle
    Controls the cap style of the line. Set to one of \l{Qt::FlatCap}{Qt.FlatCap},
    \l{Qt::SquareCap}{Qt.SquareCap} or \l{Qt::RoundCap}{Qt.RoundCap}. By
    default the cap style is Qt.SquareCap.

    \sa Qt::PenCapStyle
*/

/*!
    \qmlproperty bool SplineSeries::optimized
    \since 6.11
    Sets the control point fitting of the spline. When enabled a Catmull-Rom implementation is used.
    By default optimized is set to false.
*/

/*!
    \qmlsignal SplineSeries::widthChanged()
    This signal is emitted when the spline series width changes.
*/

/*!
    \qmlsignal SplineSeries::capStyleChanged()
    This signal is emitted when the spline series cap style changes.
*/

/*!
    \qmlsignal SplineSeries::optimizedChanged()
    \since 6.11
    This signal is emitted when the spline series' optimized property is set.
*/

QSplineSeries::QSplineSeries(QObject *parent)
    : QXYSeries(*(new QSplineSeriesPrivate()), parent)
{}

QSplineSeries::~QSplineSeries() {}

QSplineSeries::QSplineSeries(QSplineSeriesPrivate &dd, QObject *parent)
    : QXYSeries(dd, parent)
{}

void QSplineSeries::componentComplete()
{
    Q_D(QSplineSeries);

    for (auto *child : children()) {
        if (auto point = qobject_cast<QXYPoint *>(child)) {
            append(point->x(), point->y());
            qCDebug(lcSeries2D, "append points x: %f y: %f to splineSeries",
                    point->x(),
                    point->y());
        }
    }

    d->calculateSplinePoints();

    if (d->m_graphTransition)
        d->m_graphTransition->initialize();

    connect(this, &QSplineSeries::pointAdded, this, [d]([[maybe_unused]] int index) {
        d->calculateSplinePoints();
    });

    connect(this, &QSplineSeries::pointRemoved, this, [d]([[maybe_unused]] int index) {
        d->calculateSplinePoints();
    });

    connect(this, &QSplineSeries::pointsRemoved, this
            , [d]([[maybe_unused]] int index, [[maybe_unused]] int count) {
       d->calculateSplinePoints();
   });

    connect(this, &QSplineSeries::pointReplaced, this, [d]([[maybe_unused]] int index) {
        d->calculateSplinePoints();
    });

    connect(this, &QSplineSeries::pointsReplaced, this, [d]() { d->calculateSplinePoints(); });

    qCDebug(lcEvents2D, "QSplineSeries::componentComplete.");

    QAbstractSeries::componentComplete();
}

QAbstractSeries::SeriesType QSplineSeries::type() const
{
    return QAbstractSeries::SeriesType::Spline;
}

QList<QPointF> &QSplineSeries::getControlPoints()
{
    Q_D(QSplineSeries);
    return d->m_controlPoints;
}

qreal QSplineSeries::width() const
{
    Q_D(const QSplineSeries);
    return d->m_width;
}

void QSplineSeries::setWidth(qreal newWidth)
{
    Q_D(QSplineSeries);

    if (newWidth < 0) {
        qCWarning(lcProperties2D, "QSplineSeries::setWidth. Tried to use invalid width,"
                  "width has been automatically set to 0");
        newWidth = 0;
    }

    if (qFuzzyCompare(d->m_width + 1, newWidth + 1)) {
        qCDebug(lcProperties2D, "QSplineSeries::setWidth. Width is already set to: %f",
                newWidth);
        return;
    }
    d->m_width = newWidth;
    emit widthChanged();
    emit update();
}

Qt::PenCapStyle QSplineSeries::capStyle() const
{
    Q_D(const QSplineSeries);
    return d->m_capStyle;
}

void QSplineSeries::setCapStyle(Qt::PenCapStyle newCapStyle)
{
    Q_D(QSplineSeries);
    if (d->m_capStyle == newCapStyle) {
        qCDebug(lcProperties2D) << "QSplineSeries::setCapStyle. CapStyle is already set to:"
                                << newCapStyle;
        return;
    }

    d->m_capStyle = newCapStyle;
    emit capStyleChanged();
    emit update();
}

bool QSplineSeries::isOptimized() const
{
    Q_D(const QSplineSeries);
    return d->m_optimized;
}

void QSplineSeries::setOptimized(bool optimized)
{
    Q_D(QSplineSeries);

    if (d->m_optimized == optimized) {
        qCDebug(lcProperties2D) << "QSplineSeries::setOptimized. Optimize is already set to:"
                                << optimized;
        return;
    }

    d->m_optimized = optimized;
    d->calculateSplinePoints();
    emit optimizedChanged(optimized);
    emit update();
}

QSplineSeriesPrivate::QSplineSeriesPrivate()
    : QXYSeriesPrivate(QAbstractSeries::SeriesType::Spline)
    , m_width(1.0)
    , m_capStyle(Qt::PenCapStyle::SquareCap)
    , m_controlPoints()
    , m_optimized(false)
{}

void QSplineSeriesPrivate::calculateSplinePoints()
{
    if (m_points.size() == 0) {
        m_controlPoints.clear();
        qCWarning(lcSeries2D, "points list size is 0, can't calculate spline points.");
        return;
    } else if (m_points.size() == 1) {
        qCWarning(lcSeries2D, "points list size is 1, can't calculate spline points.");
        m_controlPoints = {m_points[0], m_points[0]};
        return;
    }

    if (m_optimized) {
        m_controlPoints = calculateCatmullRomPoints(m_points);
        return;
    }

    QList<QPointF> controlPoints;
    controlPoints.resize(m_points.size() * 2 - 2);

    qsizetype n = m_points.size() - 1;
    Q_TRACE_SCOPE(QGraphs2DSplineSeriesCalculateSplinePoints, n);

    if (n == 1) {
        //for n==1
        controlPoints[0].setX((2 * m_points[0].x() + m_points[1].x()) / 3);
        controlPoints[0].setY((2 * m_points[0].y() + m_points[1].y()) / 3);
        controlPoints[1].setX(2 * controlPoints[0].x() - m_points[0].x());
        controlPoints[1].setY(2 * controlPoints[0].y() - m_points[0].y());
        m_controlPoints = controlPoints;
    }

    // Calculate first Bezier control points
    // Set of equations for P0 to Pn points.
    //
    //  |   2   1   0   0   ... 0   0   0   ... 0   0   0   |   |   P1_1    |   |   P1 + 2 * P0             |
    //  |   1   4   1   0   ... 0   0   0   ... 0   0   0   |   |   P1_2    |   |   4 * P1 + 2 * P2         |
    //  |   0   1   4   1   ... 0   0   0   ... 0   0   0   |   |   P1_3    |   |   4 * P2 + 2 * P3         |
    //  |   .   .   .   .   .   .   .   .   .   .   .   .   |   |   ...     |   |   ...                     |
    //  |   0   0   0   0   ... 1   4   1   ... 0   0   0   | * |   P1_i    | = |   4 * P(i-1) + 2 * Pi     |
    //  |   .   .   .   .   .   .   .   .   .   .   .   .   |   |   ...     |   |   ...                     |
    //  |   0   0   0   0   0   0   0   0   ... 1   4   1   |   |   P1_(n-1)|   |   4 * P(n-2) + 2 * P(n-1) |
    //  |   0   0   0   0   0   0   0   0   ... 0   2   7   |   |   P1_n    |   |   8 * P(n-1) + Pn         |
    //
    QList<QPointF> list;
    list.resize(n);

    list[0].setX(m_points[0].x() + 2 * m_points[1].x());
    list[0].setY(m_points[0].y() + 2 * m_points[1].y());

    for (int i = 1; i < n - 1; ++i) {
        list[i].setX(4 * m_points[i].x() + 2 * m_points[i + 1].x());
        list[i].setY(4 * m_points[i].y() + 2 * m_points[i + 1].y());
    }

    list[n - 1].setX((8 * m_points[n - 1].x() + m_points[n].x()) / 2.0);
    list[n - 1].setY((8 * m_points[n - 1].y() + m_points[n].y()) / 2.0);

    const QList<QPointF> control = calculateControlPoints(list);

    for (int i = 0, j = 0; i < n; ++i, ++j) {
        controlPoints[j].setX(control[i].x());
        controlPoints[j].setY(control[i].y());

        j++;

        if (i < n - 1) {
            controlPoints[j].setX(2 * m_points[i + 1].x() - control[i + 1].x());
            controlPoints[j].setY(2 * m_points[i + 1].y() - control[i + 1].y());
        } else {
            controlPoints[j].setX((m_points[n].x() + control[n - 1].x()) / 2);
            controlPoints[j].setY((m_points[n].y() + control[n - 1].y()) / 2);
        }
    }

    m_controlPoints = controlPoints;
}

QList<QPointF> QSplineSeriesPrivate::calculateControlPoints(const QList<QPointF> &list)
{
    QList<QPointF> result;

    qsizetype count = list.size();

    Q_TRACE_SCOPE(QGraphs2DSplineSeriesCalculateControlPoints, count);
    result.resize(count);
    result[0].setX(list[0].x() / 2.0);
    result[0].setY(list[0].y() / 2.0);

    QList<QPointF> temp;
    temp.resize(count);
    temp[0] = {0, 0};

    qreal bx = 2.0;
    qreal by = 2.0;

    for (int i = 1; i < count; i++) {
        temp[i].setX(1 / bx);
        bx = (i < count - 1 ? 4.0 : 3.5) - temp[i].x();
        result[i].setX((list[i].x() - result[i - 1].x()) / bx);

        temp[i].setY(1 / by);
        by = (i < count - 1 ? 4.0 : 3.5) - temp[i].y();
        result[i].setY((list[i].y() - result[i - 1].y()) / by);
    }

    for (int i = 1; i < count; i++) {
        result[count - i - 1].setX(
            result[count - i - 1].x() - temp[count - i].x() * result[count - i].x());
        result[count - i - 1].setY(
            result[count - i - 1].y() - temp[count - i].y() * result[count - i].y());
    }

    return result;
}


// Based on https://arxiv.org/pdf/2011.08232
// Converting Catmull-Rom to Bezier
QList<QPointF> QSplineSeriesPrivate::calculateCatmullRomPoints(const QList<QPointF> &list)
{
    const qreal tension = 0.5f;
    QList<QPointF> result;
    result.resize(list.size() * 2 - 2);

    auto mirror1x = list[0].x() - (list[1].x() - list[0].x());
    auto mirror1y = list[0].y() - (list[1].y() - list[0].y());
    auto mirror2x = list[list.size() - 1].x()
                    - (list[list.size() - 2].x() - list[list.size() - 1].x());
    auto mirror2y = list[list.size() - 1].y()
                    - (list[list.size() - 2].y() - list[list.size() - 1].y());

    for (auto i = 0, j = 0; i < list.size() - 1; i++, j += 2) {
        auto p0 = i == 0 ? QPointF{mirror1x, mirror1y} : list[i - 1];
        auto p1 = list[i];
        auto p2 = list[i + 1];
        auto p3 = i < list.size() - 2 ? list[i + 2] : QPointF{mirror2x, mirror2y};

        auto c1 = p1 + (p2 - p0) / (6 * tension);
        auto c2 = p2 - (p3 - p1) / (6 * tension);

        result[j] = c1;
        result[j + 1] = c2;
    }

    return result;
}

QT_END_NAMESPACE

#include "moc_qsplineseries.cpp"
