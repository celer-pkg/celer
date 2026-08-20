// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquicktimeline_p.h"

#include <QtCore/qmath.h>
#include <QtGui/qpainter.h>

QT_BEGIN_NAMESPACE

class QQuickTimelinePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickTimeline)
public:
    QQuickTimelinePrivate() : enabled(false), componentComplete(false)
    {
    }

    qreal startFrame = 0;
    qreal endFrame = 0;
    qreal currentFrame = 0;

    bool enabled:1;
    bool componentComplete:1;

protected:
    void init();
    void disable();

    static void append_keyframe(QQmlListProperty<QQuickKeyframeGroup> *list, QQuickKeyframeGroup *a);
    static qsizetype keyframe_count(QQmlListProperty<QQuickKeyframeGroup> *list);
    static QQuickKeyframeGroup* keyframe_at(QQmlListProperty<QQuickKeyframeGroup> *list, qsizetype pos);
    static void clear_keyframes(QQmlListProperty<QQuickKeyframeGroup> *list);

    static void append_animation(QQmlListProperty<QQuickTimelineAnimation> *list, QQuickTimelineAnimation *a);
    static qsizetype animation_count(QQmlListProperty<QQuickTimelineAnimation> *list);
    static QQuickTimelineAnimation* animation_at(QQmlListProperty<QQuickTimelineAnimation> *list, qsizetype pos);
    static void clear_animations(QQmlListProperty<QQuickTimelineAnimation> *list);

    QList<QQuickKeyframeGroup *> keyframeGroups;
    QList<QQuickTimelineAnimation *> animations;
};

void QQuickTimelinePrivate::init()
{
    for (auto keyFrames : keyframeGroups) {
        keyFrames->init();
        keyFrames->setProperty(currentFrame);
    }
}

void QQuickTimelinePrivate::disable()
{
    for (auto keyFrames : keyframeGroups)
        keyFrames->resetDefaultValue();
}

void QQuickTimelinePrivate::append_keyframe(QQmlListProperty<QQuickKeyframeGroup> *list, QQuickKeyframeGroup *a)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    q->d_func()->keyframeGroups.append(a);
}

qsizetype QQuickTimelinePrivate::keyframe_count(QQmlListProperty<QQuickKeyframeGroup> *list)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    return q->d_func()->keyframeGroups.size();
}

QQuickKeyframeGroup* QQuickTimelinePrivate::keyframe_at(QQmlListProperty<QQuickKeyframeGroup> *list, qsizetype pos)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    return q->d_func()->keyframeGroups.at(pos);
}

void QQuickTimelinePrivate::clear_keyframes(QQmlListProperty<QQuickKeyframeGroup> *list)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    while (q->d_func()->keyframeGroups.size()) {
        QQuickKeyframeGroup *firstKeyframe = q->d_func()->keyframeGroups.at(0);
        q->d_func()->keyframeGroups.removeAll(firstKeyframe);
    }
}

void QQuickTimelinePrivate::append_animation(QQmlListProperty<QQuickTimelineAnimation> *list, QQuickTimelineAnimation *a)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    a->setTargetObject(q);
    q->d_func()->animations.append(a);
}

qsizetype QQuickTimelinePrivate::animation_count(QQmlListProperty<QQuickTimelineAnimation> *list)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    return q->d_func()->animations.size();
}

QQuickTimelineAnimation* QQuickTimelinePrivate::animation_at(QQmlListProperty<QQuickTimelineAnimation> *list, qsizetype pos)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    return q->d_func()->animations.at(pos);
}

void QQuickTimelinePrivate::clear_animations(QQmlListProperty<QQuickTimelineAnimation> *list)
{
    auto q = static_cast<QQuickTimeline *>(list->object);
    while (q->d_func()->animations.size()) {
        QQuickTimelineAnimation *firstAnimation = q->d_func()->animations.at(0);
        q->d_func()->animations.removeAll(firstAnimation);
    }
}

/*!
    \qmltype Timeline
    \inherits QtObject
    \nativetype QQuickTimeline
    \inqmlmodule QtQuick.Timeline
    \ingroup qtqmltypes

    \brief A timeline.

    Specifies a timeline with a range of keyframes that contain values for the
    properties of an object. The timeline allows specifying the values of items
    depending on keyframes and their easing curves.

    A timeline can be either used for animations or to control the behavior of
    items.

    For example, it is possible to create a progress bar where the current frame
    reflects the progress.
*/

/*!
    \qmlproperty double Timeline::startFrame

    The start of the timeline.
*/

/*!
    \qmlproperty double Timeline::endFrame

    The end of the timeline.
*/

/*!
    \qmlproperty double Timeline::currentFrame

    The current keyframe on the timeline. The current keyframe can be animated
    or a binding can be attached to it. Using bindings allows controlling
    the behavior of components.
*/

/*!
    \qmlproperty list Timeline::keyframes
    \readonly

    This property contains the keyframe groups attached to the timeline.
    Each keyframe group contains a list of keyframes for a specific item
    and property.
*/

/*!
    \qmlproperty list Timeline::animations
    \readonly

    A list of animations attached to the timeline.
*/

/*!
    \qmlproperty bool Timeline::enabled

    Whether the timeline is enabled.

    When the timeline is disabled, all items will have their regular values.
    When the timeline is enabled, the values of items are determined by the
    current frame and the keyframes.

    Only one timeline should be active at a particular time.
*/

QQuickTimeline::QQuickTimeline(QObject *parent) : QObject(*(new QQuickTimelinePrivate), parent)
{
}

QQmlListProperty<QQuickKeyframeGroup> QQuickTimeline::keyframeGroups()
{
    Q_D(QQuickTimeline);

    return { this, &d->keyframeGroups, QQuickTimelinePrivate::append_keyframe,
                QQuickTimelinePrivate::keyframe_count,
                QQuickTimelinePrivate::keyframe_at,
                QQuickTimelinePrivate::clear_keyframes };
}

QQmlListProperty<QQuickTimelineAnimation> QQuickTimeline::animations()
{
    Q_D(QQuickTimeline);

    return { this, &d->animations, QQuickTimelinePrivate::append_animation,
                QQuickTimelinePrivate::animation_count,
                QQuickTimelinePrivate::animation_at,
                QQuickTimelinePrivate::clear_animations };
}

bool QQuickTimeline::enabled() const
{
    Q_D(const QQuickTimeline);
    return d->enabled;
}

void QQuickTimeline::setEnabled(bool b)
{
    Q_D(QQuickTimeline);
    if (d->enabled == b)
        return;
    d->enabled = b;

    if (d->componentComplete) {
        if (b)
            init();
        else
            reset();
    }

    emit enabledChanged();
}

qreal QQuickTimeline::startFrame() const
{
    Q_D(const QQuickTimeline);
    return d->startFrame;
}

void QQuickTimeline::setStartFrame(qreal frame)
{
    Q_D(QQuickTimeline);
    if (d->startFrame == frame)
        return;
    d->startFrame = frame;
    emit startFrameChanged();
}

qreal QQuickTimeline::endFrame() const
{
    Q_D(const QQuickTimeline);
    return d->endFrame;
}

void QQuickTimeline::setEndFrame(qreal frame)
{
    Q_D(QQuickTimeline);
    if (d->endFrame == frame)
        return;
    d->endFrame = frame;
    emit endFrameChanged();
}

qreal QQuickTimeline::currentFrame() const
{
    Q_D(const QQuickTimeline);
    return d->currentFrame;
}

void QQuickTimeline::setCurrentFrame(qreal frame)
{
    Q_D(QQuickTimeline);
    if (d->currentFrame == frame)
        return;
    d->currentFrame = frame;

    reevaluate();

    emit currentFrameChanged();
}

void QQuickTimeline::reevaluate()
{
    Q_D(QQuickTimeline);

    if (d->componentComplete && d->enabled)
        for (auto keyFrames : d->keyframeGroups)
            keyFrames->setProperty(d->currentFrame);
}

void QQuickTimeline::init()
{
    Q_D(QQuickTimeline);

    if (d->componentComplete)
        d->init();
}

void QQuickTimeline::reset()
{
    Q_D(QQuickTimeline);

    if (d->componentComplete)
        d->disable();
}

QList<QQuickTimelineAnimation *> QQuickTimeline::getAnimations() const
{
     Q_D(const QQuickTimeline);

    return d->animations;
}

void QQuickTimeline::classBegin()
{
    Q_D(QQuickTimeline);
    d->componentComplete = false;
}

void QQuickTimeline::componentComplete()
{
    Q_D(QQuickTimeline);
    d->componentComplete = true;

    if (d->enabled)
        init();
}

QT_END_NAMESPACE
