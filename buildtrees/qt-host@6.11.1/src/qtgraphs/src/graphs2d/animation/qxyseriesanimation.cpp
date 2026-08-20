// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qxyseriesanimation_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype XYSeriesAnimation
    \qmlabstract
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits GraphAnimation
    \brief An animation type which signifies the animation for points.

    XYSeriesAnimation is based on GraphAnimation and encapsulates all animations available to XYSeries graphs.
    This type initializes the state for its derived animations, but, as it is absract, can not be directly used
    for animations. See the child types GraphPointAnimation and SplineControlAnimation.

    \sa GraphAnimation, GraphPointAnimation, SplineControlAnimation
*/

QXYSeriesAnimation::QXYSeriesAnimation(QObject *parent)
    : QGraphAnimation(parent)
{}

QXYSeriesAnimation::~QXYSeriesAnimation() {}

void QXYSeriesAnimation::updateCurrent(QGraphTransition::TransitionType tt, int index, QPointF point)
{
    m_currentTransitionType = tt;
    m_newPointIndex = index;
    m_newPoint = point;

    if (m_previousTransitionType == QGraphTransition::TransitionType::None)
        m_previousTransitionType = m_currentTransitionType;

    if (animating() == QGraphAnimation::AnimationState::Stopped)
        m_activePointIndex = index;
}

QT_END_NAMESPACE

#include "moc_qxyseriesanimation_p.cpp"
