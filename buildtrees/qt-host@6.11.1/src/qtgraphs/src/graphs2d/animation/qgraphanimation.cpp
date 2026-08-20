// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qgraphanimation_p.h"
#include <private/qgraphanimation_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype GraphAnimation
    \qmlabstract
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \brief The GraphAnimation type is the base type for all animations for 2D Qt Graphs series.

    GraphAnimation is based on VariantAnimation and provides an interface for interpolation for child animations.
    This type is an abstract type and doesn't have any default implementation for interpolation. Its derived types
    should be used for the respective animations, such as GraphPointAnimation, SplineControlAnimation.
*/

/*!
    \qmlproperty GraphAnimation::AnimationState GraphAnimation::animating

    Holds the animation state. One of \l {GraphAnimation::AnimationState}.
 */

/*!
    \qmlproperty enumeration GraphAnimation::AnimationState

    Animation states.

    \value Playing
        Animation is playing.
    \value Stopped
        Animation is stopped.
*/

/*!
    \qmlproperty enumeration GraphAnimation::GraphAnimationType

    Animation type.

    \value GraphPoint
        A GraphPointAnimation animation.
    \value ControlPoint
        A ControlPointAnimation animation.
*/

Q_LOGGING_CATEGORY(lcAnimation, "qt.graphs2d.animation")

QGraphAnimation::QGraphAnimation(QObject *parent)
    : QVariantAnimation(parent)
{
    connect(this, &QVariantAnimation::valueChanged, this, &QGraphAnimation::valueUpdated);
    connect(this, &QVariantAnimation::finished, this, &QGraphAnimation::end);
}

QGraphAnimation::~QGraphAnimation()
{
    stop();
}

QGraphAnimation::AnimationState QGraphAnimation::animating() const
{
    return m_animating;
}

void QGraphAnimation::setAnimating(AnimationState newAnimating)
{
    if (m_animating == newAnimating) {
        qCDebug(lcAnimation) << "QGraphAnimation::setAnimating. AnimationState is already set to: " << newAnimating;
        return;
    }
    m_animating = newAnimating;

    qCDebug(lcAnimation) << "animation state:" << m_animating;
    emit animatingChanged();
}

QT_END_NAMESPACE

#include "moc_qgraphanimation_p.cpp"
