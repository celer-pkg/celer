// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterrenderitem_p.h"

QT_BEGIN_NAMESPACE

ScatterRenderItem::ScatterRenderItem()
    : AbstractRenderItem(),
      m_visible(false)
{
}

ScatterRenderItem::ScatterRenderItem(const ScatterRenderItem &other)
    : AbstractRenderItem(other)
{
    m_position = other.m_position;
    m_visible = other.m_visible;
}

ScatterRenderItem::~ScatterRenderItem()
{
}

QT_END_NAMESPACE
