// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include <QtQuick3DRuntimeRender/private/qssgrenderskeleton_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderSkeleton::QSSGRenderSkeleton()
    : QSSGRenderNode(QSSGRenderGraphObject::Type::Skeleton)
{
    boneTexData.setFormat(QSSGRenderTextureFormat::RGBA32F);
}

QSSGRenderSkeleton::~QSSGRenderSkeleton() = default;

QT_END_NAMESPACE
