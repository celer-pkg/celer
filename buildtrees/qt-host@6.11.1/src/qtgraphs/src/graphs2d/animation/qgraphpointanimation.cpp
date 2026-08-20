// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QPointF>
#include <QtGraphs/QLineSeries>
#include "private/qgraphpointanimation_p.h"
#include "private/qgraphtransition_p.h"
#include "private/qxyseries_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype GraphPointAnimation
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits XYSeriesAnimation
    \brief An animation type which signifies the animation for points.

    GraphPointAnimation is an animation type derived from QVariantAnimation which defines how points are animated.
    It can make use of QVariantAnimation functionality and properties for its animations, such as \c duration and \c easing.
    These animations are housed inside of a QParallelAnimationGroup and hence will run in parallel.

    This example shows how to use a GraphPointAnimation to set points to animate with
    a \c duration of 1000ms and \c easing of OutCubic:

    \snippet doc_src_qmlgraphs.cpp 12

    For XYSeries, this is considered to be the main list of points defined inside
    the series. The point is linearly interpolated from the start to the end value.

    \note GraphPointAnimation currently supports animating only the last point in
    a series when a point is appended or removed. If a point is replaced, the
    animation will be triggered regardless of the point’s index within the
    series.

    \sa GraphTransition, SplineControlAnimation
*/

QGraphPointAnimation::QGraphPointAnimation(QObject *parent)
    : QXYSeriesAnimation(parent)
{
    setDuration(800);
    setEasingCurve(QEasingCurve::OutCubic);
}

QGraphPointAnimation::~QGraphPointAnimation() {}

QGraphAnimation::GraphAnimationType QGraphPointAnimation::animationType()
{
    return QGraphAnimation::GraphAnimationType::GraphPoint;
}

void QGraphPointAnimation::setAnimatingValue(const QVariant &start, const QVariant &end)
{
    setStartValue(start);
    setEndValue(end);
}

QVariant QGraphPointAnimation::interpolated(const QVariant &start,
                                            const QVariant &end,
                                            qreal progress) const
{
    auto startPoint = qvariant_cast<QPointF>(start);
    auto endPoint = qvariant_cast<QPointF>(end);

    auto interpolatedPoint = QPointF{
        qreal(startPoint.x() * (1.0 - progress) + endPoint.x() * progress),
        qreal(startPoint.y() * (1.0 - progress) + endPoint.y() * progress),
    };

    return QVariant::fromValue(interpolatedPoint);
}

void QGraphPointAnimation::animate()
{
    // Hierarchy should look like GraphAnimation -> ParallelAnimationGroup -> GraphTransition -> QXYSeries
    auto series = qobject_cast<QXYSeries *>(parent()->parent()->parent());

    if (!series) {
        qCCritical(lcAnimation, "QGraphPointAnimation::animate. XYSeries not found.");
        return;
    }

    if (animating() == QGraphAnimation::AnimationState::Playing) {
        end();
        m_activePointIndex = m_newPointIndex;
    }

    setAnimating(QGraphAnimation::AnimationState::Playing);

    auto &pointList = series->d_func()->m_points;

    switch (m_currentTransitionType) {
    default:
    case QGraphTransition::TransitionType::PointAdded: {
        pointList.append(series->points().size() >= 1 ? pointList.last() : m_newPoint);

        auto startv = QVariant::fromValue(pointList.last());
        auto endv = QVariant::fromValue(m_newPoint);

        qCDebug(lcAnimation) << "transition type:" << m_currentTransitionType
                             << "start value:" << startv.toPointF()
                             << "end value:" << endv.toPointF();

        setAnimatingValue(startv, endv);
    } break;
    case QGraphTransition::TransitionType::PointReplaced: {
        auto startv = QVariant::fromValue(pointList[m_activePointIndex]);
        auto endv = QVariant::fromValue(m_newPoint);

        qCDebug(lcAnimation) << "transition type:" << m_currentTransitionType
                             << "start value:" << startv.toPointF()
                             << "end value:" << endv.toPointF();
        setAnimatingValue(startv, endv);
    } break;
    case QGraphTransition::TransitionType::PointRemoved: {
        if (series->points().size() < 1)
            break;

        auto startv = QVariant::fromValue(pointList[pointList.size() - 1]);
        auto endv = QVariant::fromValue(
            pointList[pointList.size() > 1 ? pointList.size() - 2 : pointList.size() - 1]);

        qCDebug(lcAnimation) << "transition type:" << m_currentTransitionType
                             << "start value:" << startv.toPointF()
                             << "end value:" << endv.toPointF();


        setAnimatingValue(startv, endv);
    } break;
    }

    m_previousTransitionType = m_currentTransitionType;
}

void QGraphPointAnimation::end()
{
    auto series = qobject_cast<QXYSeries *>(parent()->parent()->parent());

    if (!series || animating() == QGraphAnimation::AnimationState::Stopped) {
        m_previousTransitionType = m_currentTransitionType;
        return;
    }

    setAnimating(QGraphAnimation::AnimationState::Stopped);
    stop();

    auto &points = series->d_func()->m_points;

    switch (m_previousTransitionType) {
    default:
    case QGraphTransition::TransitionType::PointAdded: {
        points.replace(m_activePointIndex, qvariant_cast<QPointF>(endValue()));
        emit series->pointAdded(points.size() - 1);
        emit series->countChanged();
    } break;
    case QGraphTransition::TransitionType::PointReplaced: {
        points.replace(m_activePointIndex, qvariant_cast<QPointF>(endValue()));
        emit series->pointReplaced(m_activePointIndex);
    } break;
    case QGraphTransition::TransitionType::PointRemoved: {
        points.remove(points.size() - 1);
        emit series->countChanged();
        emit series->pointRemoved(points.size() - 1);
    } break;
    }

    m_previousTransitionType = m_currentTransitionType;
    emit series->update();
}

void QGraphPointAnimation::valueUpdated(const QVariant &value)
{
    auto series = qobject_cast<QXYSeries *>(parent()->parent()->parent());

    if (!series)
        return;

    auto val = qvariant_cast<QPointF>(value);
    auto &points = series->d_func()->m_points;

    switch (m_currentTransitionType) {
    default:
    case QGraphTransition::TransitionType::PointAdded: {
        points.replace(m_activePointIndex, val);
    } break;
    case QGraphTransition::TransitionType::PointReplaced: {
        points.replace(m_activePointIndex, val);
    } break;
    case QGraphTransition::TransitionType::PointRemoved: {
        if (points.size() > 1)
            points.replace(points.size() - 1, val);
    } break;
    }

    emit series->update();
}

QT_END_NAMESPACE

#include "moc_qgraphpointanimation_p.cpp"
