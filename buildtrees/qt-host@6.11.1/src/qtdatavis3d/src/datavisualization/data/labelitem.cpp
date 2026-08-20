// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "labelitem_p.h"

QT_BEGIN_NAMESPACE

LabelItem::LabelItem()
    : m_size(QSize(0, 0)),
      m_textureId(0)
{
}

LabelItem::~LabelItem()
{
    clear();
}

void LabelItem::setSize(const QSize &size)
{
    m_size = size;
}

QSize LabelItem::size() const
{
    return m_size;
}

void LabelItem::setTextureId(GLuint textureId)
{
    QOpenGLContext::currentContext()->functions()->glDeleteTextures(1, &m_textureId);
    m_textureId = textureId;
}

GLuint LabelItem::textureId() const
{
    return m_textureId;
}

void LabelItem::clear()
{
    if (m_textureId && QOpenGLContext::currentContext())
        QOpenGLContext::currentContext()->functions()->glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
    m_size = QSize(0, 0);
}

QT_END_NAMESPACE
