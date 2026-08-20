// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTIMELINEANIMATIONNODE_P_H
#define QTIMELINEANIMATIONNODE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QtQml>
#include <QtQuickTimeline/private/qquicktimeline_p.h>
#include <QtQuickTimeline/private/qquicktimelineanimation_p.h>
#include <QtQuickTimelineBlendTrees/private/qblendtreenode_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICKTIMELINEBLENDTREES_EXPORT QTimelineAnimationNode : public QBlendTreeNode
{
    Q_OBJECT
    Q_PROPERTY(QQuickTimelineAnimation *animation READ animation WRITE setAnimation NOTIFY animationChanged FINAL)
    Q_PROPERTY(QQuickTimeline *timeline READ timeline WRITE setTimeline NOTIFY timelineChanged FINAL)
    Q_PROPERTY(qreal currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged FINAL)
    QML_NAMED_ELEMENT(TimelineAnimationNode)
public:
    explicit QTimelineAnimationNode(QObject *parent = nullptr);

    QQuickTimelineAnimation *animation() const;
    void setAnimation(QQuickTimelineAnimation *newAnimation);

    QQuickTimeline *timeline() const;
    void setTimeline(QQuickTimeline *newTimeline);

    qreal currentFrame() const;
    void setCurrentFrame(qreal newCurrentFrame);

Q_SIGNALS:
    void animationChanged();
    void timelineChanged();
    void currentFrameChanged();

private:
    void updateFrameData();
    void updateAnimationTarget();
    QQuickTimelineAnimation *m_animation = nullptr;
    QQuickTimeline *m_timeline = nullptr;
    qreal m_currentFrame = -1.0;

    QMetaObject::Connection m_animationDestroyedConnection;
    QMetaObject::Connection m_timelineDestroyedConnection;
};

QT_END_NAMESPACE

#endif // QTIMELINEANIMATIONNODE_P_H
