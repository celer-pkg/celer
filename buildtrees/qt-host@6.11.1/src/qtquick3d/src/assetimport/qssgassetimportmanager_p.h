// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGASSETIMPORTMANAGER_H
#define QSSGASSETIMPORTMANAGER_H

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
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/qjsonobject.h>

QT_BEGIN_NAMESPACE

class QSSGAssetImporter;
class QQuick3DNode;
namespace QSSGSceneDesc { struct Scene; }

struct QSSGAssetImporterPluginInfo
{
    QString name;
    QStringList inputExtensions;
    QString outputExtension;
    QString type;
    QJsonObject importOptions;
    QString typeDescription;
};

class Q_QUICK3DASSETIMPORT_EXPORT QSSGAssetImportManager : public QObject
{
    Q_OBJECT
public:
    explicit QSSGAssetImportManager(QObject *parent = nullptr);
    ~QSSGAssetImportManager();

    enum class ImportState : quint8
    {
        Success,
        IoError,
        Unsupported
    };

    using PluginOptionMaps = QHash<QString, QJsonObject>;

    // ### Temp API
    ImportState importFile(const QString &filename,
                           const QDir &outputPath,
                           QString *error = nullptr);
    ImportState importFile(const QString &filename,
                           const QDir &outputPath,
                           const QJsonObject &options = QJsonObject(),
                           QString *error = nullptr);
    ImportState importFile(const QUrl &url,
                           QSSGSceneDesc::Scene &scene,
                           QString *error = nullptr);
    ImportState importFile(const QUrl &url,
                           QSSGSceneDesc::Scene &scene,
                           const QJsonObject &options = QJsonObject(),
                           QString *error = nullptr);
    QJsonObject getOptionsForFile(const QString &filename);
    PluginOptionMaps getAllOptions() const;
    QHash<QString, QStringList> getSupportedExtensions() const;
    QList<QSSGAssetImporterPluginInfo> getImporterPluginInfos() const;

private:
    QVector<QSSGAssetImporter *> m_assetImporters;
    QMap<QString, QSSGAssetImporter *> m_extensionsMap;
};

QT_END_NAMESPACE

#endif // QSSGASSETIMPORTMANAGER_H
