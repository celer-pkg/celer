// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvganimator_p.h"
#include <QtCore/qdatetime.h>
#include <QtSvg/private/qsvganimate_p.h>

QT_BEGIN_NAMESPACE

QSvgAbstractAnimator::QSvgAbstractAnimator()
    : m_time(0)
    , m_animationDuration(0)
{
}

QSvgAbstractAnimator::~QSvgAbstractAnimator()
{
    for (auto animationHash : {&m_animationsCSS, &m_animationsSMIL}) {
        for (const auto &nodeAnimations : std::as_const(*animationHash)) {
            for (QSvgAbstractAnimation *anim : nodeAnimations)
                delete anim;
        }
    }
}


void QSvgAbstractAnimator::appendAnimation(const QSvgNode *node, QSvgAbstractAnimation *anim)
{
    if (!node)
        return;

    if (anim->animationType() == QSvgAbstractAnimation::SMIL)
        m_animationsSMIL[node].append(anim);
    else
        m_animationsCSS[node].append(anim);
}

QList<QSvgAbstractAnimation *> QSvgAbstractAnimator::animationsForNode(const QSvgNode *node) const
{
    return combinedAnimationsForNode(node);
}

void QSvgAbstractAnimator::advanceAnimations()
{
    qreal elapsedTime = currentElapsed();
    for (auto animationHash : {&m_animationsCSS, &m_animationsSMIL}) {
        for (const auto &nodeAnimations : std::as_const(*animationHash)) {
            for (QSvgAbstractAnimation *anim : nodeAnimations)
                anim->evaluateAnimation(elapsedTime);
        }
    }
}

void QSvgAbstractAnimator::setAnimationDuration(qint64 dur)
{
    m_animationDuration = dur;
}

qint64 QSvgAbstractAnimator::animationDuration() const
{
    return m_animationDuration;
}

QList<QSvgAbstractAnimation *> QSvgAbstractAnimator::combinedAnimationsForNode(const QSvgNode *node) const
{
    if (!node)
        return QList<QSvgAbstractAnimation *>();

    return m_animationsSMIL.value(node) + m_animationsCSS.value(node);
}

QSvgAnimator::QSvgAnimator()
{
}

QSvgAnimator::~QSvgAnimator()
{
}

void QSvgAnimator::restartAnimation()
{
    m_time = QDateTime::currentMSecsSinceEpoch();
}

qint64 QSvgAnimator::currentElapsed()
{
    return QDateTime::currentMSecsSinceEpoch() - m_time;
}

void QSvgAnimator::setAnimatorTime(qint64 time)
{
    m_time -= time;
}

QSvgAnimationController::QSvgAnimationController()
{
}

QSvgAnimationController::~QSvgAnimationController()
{
}

void QSvgAnimationController::restartAnimation()
{
    m_time = 0;
}

qint64 QSvgAnimationController::currentElapsed()
{
    return m_time;
}

void QSvgAnimationController::setAnimatorTime(qint64 time)
{
    m_time = qMax(0, time);
}

QT_END_NAMESPACE
