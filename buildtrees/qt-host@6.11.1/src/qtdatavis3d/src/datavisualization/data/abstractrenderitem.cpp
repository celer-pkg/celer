// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "abstractrenderitem_p.h"

QT_BEGIN_NAMESPACE

AbstractRenderItem::AbstractRenderItem()
{
}

AbstractRenderItem::AbstractRenderItem(const AbstractRenderItem &other)
{
    m_translation = other.m_translation;
    m_rotation = other.m_rotation;
}

AbstractRenderItem::~AbstractRenderItem()
{
}

QT_END_NAMESPACE
