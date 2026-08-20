// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include <QtQuick3DRuntimeRender/private/qssgrenderjoint_p.h>
#include <QtQuick/QSGTexture>

QT_BEGIN_NAMESPACE

QSSGRenderJoint::QSSGRenderJoint()
    : QSSGRenderNode(QSSGRenderGraphObject::Type::Joint)
{
}

QSSGRenderJoint::~QSSGRenderJoint() = default;


QT_END_NAMESPACE
