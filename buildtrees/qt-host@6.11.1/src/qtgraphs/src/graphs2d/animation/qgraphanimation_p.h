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

#ifndef QGRAPHANIMATION_H
#define QGRAPHANIMATION_H

#include <QtCore/QVariantAnimation>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcAnimation)

class QGraphAnimation : public QVariantAnimation
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    Q_PROPERTY(
        AnimationState animating READ animating WRITE setAnimating NOTIFY animatingChanged FINAL)

public:
    enum class AnimationState {
        Playing,
        Stopped,
    };
    Q_ENUM(AnimationState);

    enum class GraphAnimationType { GraphPoint, ControlPoint };
    Q_ENUM(GraphAnimationType);

    explicit QGraphAnimation(QObject *parent = nullptr);
    ~QGraphAnimation() override;

    virtual GraphAnimationType animationType() = 0;
    virtual void setAnimatingValue(const QVariant &start, const QVariant &end) = 0;
    virtual void animate() = 0;
    virtual void end() = 0;
    QVariant interpolated(const QVariant &start, const QVariant &end, qreal progress) const override
        = 0;

    AnimationState animating() const;
    void setAnimating(AnimationState newAnimating);

public Q_SLOTS:
    virtual void valueUpdated(const QVariant &value) = 0;

signals:
    void animatingChanged();

private:
    AnimationState m_animating = AnimationState::Stopped;
};

QT_END_NAMESPACE

#endif // QGRAPHANIMATION_H
