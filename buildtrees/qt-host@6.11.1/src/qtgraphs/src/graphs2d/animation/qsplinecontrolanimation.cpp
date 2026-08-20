// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QPointF>
#include "QSplineSeries"
#include "private/qsplinecontrolanimation_p.h"
#include "private/qsplineseries_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype SplineControlAnimation
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits XYSeriesAnimation
    \brief An animation type which signifies the animation for spline control points.

    SplineControlAnimation is an animation type derived from QVariantAnimation which defines how spline control points
    are animated. It can make use of QVariantAnimation functionality and properties for its animations, such as \c duration
    and \c easing. These animations are housed inside a QParallelAnimationGroup and hence will run in parallel.
    This animation will not affect the main points of the SplineSeries, but only the two control handles
    on either side of the point. Each of the control points are linearly interpolated in succession.

    This example shows how to use both a SplineControlPointAnimation and a
    GraphPointAnimation to define animations for both the main series of points and the
    control points of a SplineSeries:

    \snippet doc_src_qmlgraphs.cpp 13

    \sa GraphTransition, GraphPointAnimation
*/

QSplineControlAnimation::QSplineControlAnimation(QObject *parent)
    : QXYSeriesAnimation(parent)
{
    setDuration(800);
    setEasingCurve(QEasingCurve::OutCubic);
}

QSplineControlAnimation::~QSplineControlAnimation() {}

QGraphAnimation::GraphAnimationType QSplineControlAnimation::animationType()
{
    return QGraphAnimation::GraphAnimationType::ControlPoint;
}

void QSplineControlAnimation::setAnimatingValue(const QVariant &start, const QVariant &end)
{
    setStartValue(start);
    setEndValue(end);
}

QVariant QSplineControlAnimation::interpolated(const QVariant &start,
                                               const QVariant &end,
                                               qreal progress) const
{
    auto startList = qvariant_cast<QList<QPointF>>(start);
    auto endList = qvariant_cast<QList<QPointF>>(end);
    auto interpolateList = QList<QPointF>();

    for (int i = 0; i < qMin(startList.size(), endList.size()); ++i) {
        interpolateList.push_back(
            {qreal(startList[i].x() * (1.0 - progress) + endList[i].x() * progress),
             qreal(startList[i].y() * (1.0 - progress) + endList[i].y() * progress)});
    }

    return QVariant::fromValue(interpolateList);
}

void QSplineControlAnimation::animate()
{
    // Hierarchy should look like GraphAnimation -> ParallelAnimationGroup -> GraphTransition -> SplineSeries
    auto series = qobject_cast<QSplineSeries *>(parent()->parent()->parent());

    if (!series || series->points().size() < 1) {
        qCDebug(lcAnimation, "Series is nullptr or series point list is empty");
        return;
    }

    auto pointList = series->points();
    auto &cPoints = series->d_func()->m_controlPoints;

    if (animating() == QGraphAnimation::AnimationState::Playing)
        end();

    setAnimating(QGraphAnimation::AnimationState::Playing);

    auto oldPoints = cPoints;

    series->d_func()->calculateSplinePoints();

    while (oldPoints.size() < cPoints.size()) {
        // Each point corresponds to a 2n - 1 control point pair other than the first
        // (Except when there are only 2 points)
        // 0 ---- 0
        // 1 ---- 1
        //   ---- 2
        // 2 ---- 3
        //   ---- 4 ...
        QPointF point = pointList[oldPoints.size() / 2];
        oldPoints.append(point);
    }

    auto varStart = QVariant::fromValue(oldPoints);
    auto varEnd = QVariant::fromValue(cPoints);

    setAnimatingValue(varStart, varEnd);
}

void QSplineControlAnimation::end()
{
    auto series = qobject_cast<QSplineSeries *>(parent()->parent()->parent());

    if (!series || animating() == QGraphAnimation::AnimationState::Stopped)
        return;

    setAnimating(QGraphAnimation::AnimationState::Stopped);
    stop();

    series->d_func()->calculateSplinePoints();

    emit series->update();
}

void QSplineControlAnimation::valueUpdated(const QVariant &value)
{
    auto series = qobject_cast<QSplineSeries *>(parent()->parent()->parent());

    if (!series)
        return;

    auto &cPoints = series->d_func()->m_controlPoints;
    auto points = qvariant_cast<QList<QPointF>>(value);

    for (int i = 0; i < qMin(points.size(), cPoints.size()); ++i)
        cPoints.replace(i, points[i]);

    emit series->update();
}

QT_END_NAMESPACE

#include "moc_qsplinecontrolanimation_p.cpp"
