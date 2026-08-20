// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "declarativecolor_p.h"

QT_BEGIN_NAMESPACE

DeclarativeColor::DeclarativeColor(QObject *parent)
    : QObject(parent)
{
}

void DeclarativeColor::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(color);
    }
}

QColor DeclarativeColor::color() const
{
    return m_color;
}

QT_END_NAMESPACE
