// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtimelineanimationnode_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TimelineAnimationNode
    \inherits BlendTreeNode
    \nativetype QTimelineAnimationNode
    \inqmlmodule QtQuick.Timeline.BlendTrees
    \ingroup qtqmltypes

    \brief A blend tree source node that plays a timeline animation.

    TimelineAnimationNode is a blend tree source node that plays a timeline
    animation and outputs the animation's frame data. This node wraps a
    TimelineAnimation and its associated Timeline and provides a way to
    intercept the animation's frame data and output it to the blend tree.
*/

/*!
    \qmlproperty TimelineAnimation TimelineAnimationNode::animation

    This property holds the timeline animation to play.
*/

/*!
    \qmlproperty Timeline TimelineAnimationNode::timeline

    This property holds the timeline that the animation is played on.
*/

/*!
    \qmlproperty real TimelineAnimationNode::currentFrame

    This property holds the current frame of the animation.
*/

QTimelineAnimationNode::QTimelineAnimationNode(QObject *parent)
    : QBlendTreeNode(parent)
{

}

QQuickTimelineAnimation *QTimelineAnimationNode::animation() const
{
    return m_animation;
}

void QTimelineAnimationNode::setAnimation(QQuickTimelineAnimation *newAnimation)
{
    if (m_animation == newAnimation)
        return;

    if (m_animation)
        disconnect(m_animationDestroyedConnection);

    m_animation = newAnimation;

    if (m_animation)
        m_animationDestroyedConnection = connect(m_animation,
                                                 &QObject::destroyed,
                                                 this,
                                                 [this] {setAnimation(nullptr);});

    updateAnimationTarget();
    updateFrameData();
    Q_EMIT animationChanged();
}

QQuickTimeline *QTimelineAnimationNode::timeline() const
{
    return m_timeline;
}

void QTimelineAnimationNode::setTimeline(QQuickTimeline *newTimeline)
{
    if (m_timeline == newTimeline)
        return;

    if (m_timeline)
        disconnect(m_timelineDestroyedConnection);

    m_timeline = newTimeline;

    if (m_timeline)
        m_timelineDestroyedConnection = connect(m_timeline,
                                                 &QObject::destroyed,
                                                 this,
                                                 [this] {setTimeline(nullptr);});

    updateFrameData();
    Q_EMIT timelineChanged();
}

qreal QTimelineAnimationNode::currentFrame() const
{
    return m_currentFrame;
}

void QTimelineAnimationNode::setCurrentFrame(qreal newCurrentFrame)
{
    if (qFuzzyCompare(m_currentFrame, newCurrentFrame))
        return;
    m_currentFrame = newCurrentFrame;
    updateFrameData();
    Q_EMIT currentFrameChanged();
}

static QHash<QQmlProperty, QVariant> getFrameData(QQuickTimeline *timeline, qreal frame)
{
    QHash<QQmlProperty, QVariant> frameData;
    if (timeline) {
        QQmlListReference keyframeGroups(timeline, "keyframeGroups");
        if (keyframeGroups.isValid() && keyframeGroups.isReadable()) {
            for (int i = 0; i < keyframeGroups.count(); ++i) {
                QQuickKeyframeGroup *keyframeGroup = qobject_cast<QQuickKeyframeGroup *>(keyframeGroups.at(i));
                if (keyframeGroup && keyframeGroup->target()) {
                    QQmlProperty qmlProperty(keyframeGroup->target(), keyframeGroup->property());
                    QVariant value = keyframeGroup->evaluate(frame);
                    frameData.insert(qmlProperty, value);
                }
            }
        }
    }

    return frameData;
}

void QTimelineAnimationNode::updateFrameData()
{
    if (!m_animation || !m_timeline)
        return;

    m_frameData = getFrameData(m_timeline, m_currentFrame);
    Q_EMIT frameDataChanged();
}

void QTimelineAnimationNode::updateAnimationTarget()
{
    if (!m_animation)
        return;
    // Property should already be set to "currentFrame"
    m_animation->setTargetObject(this);
}

QT_END_NAMESPACE
