// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef ASSIMPIMPORTER_H
#define ASSIMPIMPORTER_H

#include <QtQuick3DAssetImport/private/qssgassetimporter_p.h>
#include <QtQuick3DAssetUtils/private/qssgqmlutilities_p.h>

#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtCore/QStringList>

enum class QSSGRenderComponentType;

QT_BEGIN_NAMESPACE

class AssimpImporter : public QSSGAssetImporter
{
public:
    AssimpImporter();
    ~AssimpImporter() override;

    QString name() const override;
    QStringList inputExtensions() const override;
    QString outputExtension() const override;
    QString type() const override;
    QString typeDescription() const override;
    QJsonObject importOptions() const override;
    QString import(const QString &sourceFile, const QDir &savePath, const QJsonObject &options,
                   QStringList *generatedFiles) override;
    QString import(const QUrl &sourceFile, const QJsonObject &options, QSSGSceneDesc::Scene &scene) override;

private:
    QJsonObject m_options;
};

QT_END_NAMESPACE

#endif // ASSIMPIMPORTER_H
