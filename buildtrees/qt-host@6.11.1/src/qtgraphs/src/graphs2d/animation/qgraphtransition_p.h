// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QGRAPHTRANSITION_H
#define QGRAPHTRANSITION_H

#include <QObject>
#include <QParallelAnimationGroup>
#include <QQmlEngine>
#include <private/qgraphanimation_p.h>

QT_BEGIN_NAMESPACE

class QGraphTransition : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QObject> animations READ animations CONSTANT)
    Q_CLASSINFO("DefaultProperty", "animations")
    QML_NAMED_ELEMENT(GraphTransition)

public:
    enum class TransitionType {
        None,
        PointAdded,
        PointReplaced,
        PointRemoved,
    };

    Q_ENUM(TransitionType);

    QGraphTransition(QObject *parent = nullptr);
    ~QGraphTransition() override;
    QQmlListProperty<QObject> animations();

    void onPointChanged(TransitionType type, int index, QPointF point);
    void initialize();
    void stop();

    bool initialized();
    bool contains(QGraphAnimation::GraphAnimationType type);

protected:
    void classBegin() override;
    void componentComplete() override;

private:
    QList<QGraphAnimation *> m_animations;
    QParallelAnimationGroup m_animationGroup;
    bool m_initialized;

    static void append(QQmlListProperty<QObject> *, QObject *);
    static void clear(QQmlListProperty<QObject> *);
};

QT_END_NAMESPACE

#endif // QGRAPHTRANSITION_H
