// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgassetimporterfactory_p.h"
#include "qssgassetimporterplugin_p.h"
#include "qssgassetimporter_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QSSGAssetImporterFactoryInterface_iid, QLatin1String("/assetimporters"), Qt::CaseInsensitive))

QStringList QSSGAssetImporterFactory::keys()
{
    return loader->keyMap().values();
}

QSSGAssetImporter *QSSGAssetImporterFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<QSSGAssetImporter, QSSGAssetImporterPlugin>(loader(), name, args);
}

QT_END_NAMESPACE
