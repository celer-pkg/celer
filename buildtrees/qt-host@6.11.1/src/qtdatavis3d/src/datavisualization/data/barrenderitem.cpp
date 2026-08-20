// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "barrenderitem_p.h"

QT_BEGIN_NAMESPACE

BarRenderItem::BarRenderItem()
    : AbstractRenderItem(),
      m_value(0),
      m_height(0.0f)
{
}

BarRenderItem::BarRenderItem(const BarRenderItem &other)
    : AbstractRenderItem(other)
{
    m_value = other.m_value;
    m_position = other.m_position;
    m_height = other.m_height;
}

BarRenderItem::~BarRenderItem()
{
}

BarRenderSliceItem::BarRenderSliceItem()
    : BarRenderItem(),
      m_sliceLabelItem(0)
{
}

BarRenderSliceItem::BarRenderSliceItem(const BarRenderSliceItem &other)
    : BarRenderItem(other)
{
    m_sliceLabel = other.m_sliceLabel;
    m_sliceLabelItem = 0;
}

BarRenderSliceItem::~BarRenderSliceItem()
{
    delete m_sliceLabelItem;
}

void BarRenderSliceItem::setItem(const BarRenderItem &renderItem)
{
    m_translation = renderItem.translation();
    m_rotation = renderItem.rotation();
    m_value = renderItem.value();
    m_position = renderItem.position();
    m_height = renderItem.height();
    m_sliceLabel = QString();
    delete m_sliceLabelItem;
    m_sliceLabelItem = 0;
}

LabelItem &BarRenderSliceItem::sliceLabelItem()
{
    if (!m_sliceLabelItem)
        m_sliceLabelItem = new LabelItem;
    return *m_sliceLabelItem;
}

void BarRenderSliceItem::setSliceLabel(const QString &label)
{
    if (m_sliceLabelItem)
        m_sliceLabelItem->clear();
    m_sliceLabel = label;
}

const QString &BarRenderSliceItem::sliceLabel() const
{
    return m_sliceLabel;
}

QT_END_NAMESPACE
