// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquickgraphscolor_p.h"

QT_BEGIN_NAMESPACE

QQuickGraphsColor::QQuickGraphsColor(QObject *parent)
    : QObject(parent)
{}

QQuickGraphsColor::~QQuickGraphsColor() {}

void QQuickGraphsColor::setColor(QColor color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(color);
    }
}

QColor QQuickGraphsColor::color() const
{
    return m_color;
}

QT_END_NAMESPACE

#include "moc_qquickgraphscolor_p.cpp"
