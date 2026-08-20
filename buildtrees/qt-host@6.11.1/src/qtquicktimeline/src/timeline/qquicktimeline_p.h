// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKTIMELINE_P_H
#define QQUICKTIMELINE_P_H

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

#include "qquickkeyframe_p.h"
#include "qquicktimelineanimation_p.h"
#include "qtquicktimelineglobal_p.h"

#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuickTimelinePrivate;

class Q_QUICKTIMELINE_EXPORT QQuickTimeline : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuickTimeline)

    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(qreal startFrame READ startFrame WRITE setStartFrame  NOTIFY startFrameChanged)
    Q_PROPERTY(qreal endFrame READ endFrame WRITE setEndFrame  NOTIFY endFrameChanged)
    Q_PROPERTY(qreal currentFrame READ currentFrame WRITE setCurrentFrame  NOTIFY currentFrameChanged)
    Q_PROPERTY(QQmlListProperty<QQuickKeyframeGroup> keyframeGroups READ keyframeGroups)
    Q_PROPERTY(QQmlListProperty<QQuickTimelineAnimation> animations READ animations)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    QML_NAMED_ELEMENT(Timeline)
    QML_ADDED_IN_VERSION(1, 0)

    Q_CLASSINFO("DefaultProperty", "keyframeGroups")

public:
    explicit QQuickTimeline(QObject *parent = nullptr);

    QQmlListProperty<QQuickKeyframeGroup> keyframeGroups();
    QQmlListProperty<QQuickTimelineAnimation> animations();

    bool enabled() const;
    void setEnabled(bool enabled);

    qreal startFrame() const;
    void setStartFrame(qreal);

    qreal endFrame() const;
    void setEndFrame(qreal);

    qreal currentFrame() const;
    void setCurrentFrame(qreal);

    void reevaluate();

    void init();
    void reset();

    QList<QQuickTimelineAnimation*> getAnimations() const;

protected:
    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void enabledChanged();
    void startFrameChanged();
    void endFrameChanged();
    void currentFrameChanged();
};

QT_END_NAMESPACE

#endif // QQUICKTIMELINE_P_H
