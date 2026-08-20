// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "assimpimporter.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtQuick3DAssetImport/private/qssgassetimporterfactory_p.h>
#include <QtQuick3DAssetImport/private/qssgassetimporter_p.h>

QT_BEGIN_NAMESPACE

AssimpImporter::AssimpImporter()
{
    QFile optionFile(":/assimpimporter/options.json");
    if (optionFile.open(QIODevice::ReadOnly)) {
        QByteArray options = optionFile.readAll();
        auto optionsDocument = QJsonDocument::fromJson(options);
        m_options = optionsDocument.object();
    }
}

AssimpImporter::~AssimpImporter()
{
}

QString AssimpImporter::name() const
{
    return QStringLiteral("assimp");
}

QStringList AssimpImporter::inputExtensions() const
{
    QStringList extensions;
    extensions.append(QStringLiteral("fbx"));
    extensions.append(QStringLiteral("dae"));
    extensions.append(QStringLiteral("obj"));
    extensions.append(QStringLiteral("gltf"));
    extensions.append(QStringLiteral("glb"));
    extensions.append(QStringLiteral("stl"));
    extensions.append(QStringLiteral("ply"));
    return extensions;
}

QString AssimpImporter::outputExtension() const
{
    return QStringLiteral(".qml");
}

QString AssimpImporter::type() const
{
    return QStringLiteral("Scene");
}

QString AssimpImporter::typeDescription() const
{
    return QObject::tr("3D Scene");
}

QJsonObject AssimpImporter::importOptions() const
{
    return m_options;
}

QT_END_NAMESPACE
