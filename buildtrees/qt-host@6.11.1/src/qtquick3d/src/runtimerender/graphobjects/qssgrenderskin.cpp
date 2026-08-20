// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderskin_p.h"

QT_BEGIN_NAMESPACE

QSSGRenderSkin::QSSGRenderSkin()
    : QSSGRenderTextureData(QSSGRenderGraphObject::Type::Skin)
{
    setFormat(QSSGRenderTextureFormat::RGBA32F);
}

QSSGRenderSkin::~QSSGRenderSkin()
{
}

QByteArray &QSSGRenderSkin::boneData()
{
    return m_textureData;
}

QT_END_NAMESPACE
