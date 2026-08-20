// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "assimpimporterplugin.h"
#include "assimpimporter.h"

QT_BEGIN_NAMESPACE

QSSGAssetImporter *AssimpImporterPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key);
    Q_UNUSED(paramList);
    return new AssimpImporter();
}

QT_END_NAMESPACE
