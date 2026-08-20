// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGASSETIMPORTER_H
#define QSSGASSETIMPORTER_H

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

#include <QtCore/QVariantMap>
#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/qjsonobject.h>

QT_BEGIN_NAMESPACE

class QQuick3DNode;

namespace QSSGSceneDesc {
struct Scene;
}

class Q_QUICK3DASSETIMPORT_EXPORT QSSGAssetImporter : public QObject
{
    Q_OBJECT
public:
    virtual QString name() const = 0;
    virtual QStringList inputExtensions() const = 0;
    virtual QString outputExtension() const = 0;
    virtual QString type() const = 0;
    virtual QJsonObject importOptions() const = 0;
    virtual QString typeDescription() const = 0;
    virtual QString import(const QString &sourceFile,
                           const QDir &savePath,
                           const QJsonObject &options,
                           QStringList *generatedFiles = nullptr) = 0;
    virtual QString import(const QUrl &url,
                           const QJsonObject &options,
                           QSSGSceneDesc::Scene &scene) = 0;
};

QT_END_NAMESPACE

#endif // QSSGASSETIMPORTER_H
