// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGASSETIMPORTERPLUGIN_P_H
#define QSSGASSETIMPORTERPLUGIN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DAssetImport/private/qtquick3dassetimportglobal_p.h>

#include <QtCore/QObject>

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

class QSSGAssetImporter;

#define QSSGAssetImporterFactoryInterface_iid "org.qt-project.QtQuick3D.AssetImporter.6.5"

class Q_QUICK3DASSETIMPORT_EXPORT QSSGAssetImporterPlugin : public QObject
{
    Q_OBJECT
public:
    virtual QSSGAssetImporter *create(const QString &key, const QStringList &paramList) = 0;
};

QT_END_NAMESPACE

#endif // QSSGASSETIMPORTERPLUGIN_P_H
