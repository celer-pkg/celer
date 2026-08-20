// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderresourceloader_p.h"

QT_BEGIN_NAMESPACE

QSSGRenderResourceLoader::QSSGRenderResourceLoader()
    : QSSGRenderGraphObject(QSSGRenderGraphObject::Type::ResourceLoader, FlagT(Flags::HasGraphicsResources))
{
}

QT_END_NAMESPACE
