// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvganimate_p.h"

QT_BEGIN_NAMESPACE

QSvgAnimateNode::QSvgAnimateNode(QSvgNode *parent)
    : QSvgNode(parent)
    , m_end(0)
    , m_fill(Fill::Freeze)
    , m_additive(Additive::Replace)
{
    m_easing = std::make_unique<QSvgLinearEasing>();
}

void QSvgAnimateNode::setRunningTime(int startMs, int durMs, int endMs, int by)
{
    Q_UNUSED(by)
    m_start = startMs;
    m_end = endMs;
    m_duration = durMs;
}

void QSvgAnimateNode::drawCommand(QPainter *p, QSvgExtraStates &states)
{
    Q_UNUSED(p)
    Q_UNUSED(states)
}

bool QSvgAnimateNode::shouldDrawNode(QPainter *p, QSvgExtraStates &states) const
{
    Q_UNUSED(p)
    Q_UNUSED(states)
    return false;
}

QT_END_NAMESPACE
