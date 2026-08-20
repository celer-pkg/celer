// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendertexturedata_p.h"

QT_BEGIN_NAMESPACE

QSSGRenderTextureData::QSSGRenderTextureData()
    : QSSGRenderTextureData(QSSGRenderGraphObject::Type::TextureData)
{

}

QSSGRenderTextureData::QSSGRenderTextureData(QSSGRenderGraphObject::Type type)
    : QSSGRenderGraphObject(type, FlagT(Flags::HasGraphicsResources))
{

}

QSSGRenderTextureData::~QSSGRenderTextureData()
{

}

const QByteArray &QSSGRenderTextureData::textureData() const
{
    return m_textureData;
}

void QSSGRenderTextureData::setTextureData(const QByteArray &data)
{
    m_textureData = data;
    // Bump the version number
    ++m_textureDataVersion;
}

void QSSGRenderTextureData::setSize(const QSize &size)
{
    if (m_size == size)
        return;
    m_size = size;
}

void QSSGRenderTextureData::setDepth(int depth)
{
    if (m_depth == depth)
        return;
    m_depth = depth;
}

void QSSGRenderTextureData::setFormat(QSSGRenderTextureFormat format)
{
    if (m_format == format)
        return;

    m_format = format;
}

void QSSGRenderTextureData::setHasTransparency(bool hasTransparency)
{
    if (m_hasTransparency == hasTransparency)
        return;

    m_hasTransparency = hasTransparency;
}

QT_END_NAMESPACE
