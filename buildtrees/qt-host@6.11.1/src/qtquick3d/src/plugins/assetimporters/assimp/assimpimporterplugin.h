// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef ASSIMPIMPORTERPLUGIN_H
#define ASSIMPIMPORTERPLUGIN_H

#include <QtQuick3DAssetImport/private/qssgassetimporterplugin_p.h>

QT_BEGIN_NAMESPACE

class AssimpImporterPlugin : public QSSGAssetImporterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QSSGAssetImporterFactoryInterface_iid FILE "assimp.json")

public:
    QSSGAssetImporter *create(const QString &key, const QStringList &paramList) override;
};

QT_END_NAMESPACE


#endif // ASSIMPIMPORTERPLUGIN_H
