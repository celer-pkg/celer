// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgassetimportmanager_p.h"

#include "qssgassetimporter_p.h"
#include "qssgassetimporterfactory_p.h"

#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

QSSGAssetImportManager::QSSGAssetImportManager(QObject *parent) : QObject(parent)
{
    // load assetimporters
    const QStringList keys = QSSGAssetImporterFactory::keys();
    for (const auto &key : keys) {
        auto importer = QSSGAssetImporterFactory::create(key, QStringList());
        if (importer) {
            m_assetImporters.append(importer);
            // Add to extension map
            for (const auto &extension : importer->inputExtensions()) {
                m_extensionsMap.insert(extension, importer);
            }
        } else {
            qWarning() << "Failed to load asset import plugin with key: " << key;
        }
    }
}

QSSGAssetImportManager::~QSSGAssetImportManager()
{
    for (auto importer : m_assetImporters) {
        delete importer;
    }
}

// Compatibility with old API
QSSGAssetImportManager::ImportState QSSGAssetImportManager::importFile(const QString &filename,
                                                                       const QDir &outputPath,
                                                                       QString *error)
{
    return importFile(filename, outputPath, QJsonObject(), error);
}

QSSGAssetImportManager::ImportState QSSGAssetImportManager::importFile(const QString &filename,
                                                                       const QDir &outputPath,
                                                                       const QJsonObject &options,
                                                                       QString *error)
{
    QFileInfo fileInfo(filename);

    // Is this a real file?
    if (!fileInfo.exists()) {
        if (error)
            *error = QStringLiteral("file does not exist");
        return ImportState::IoError;
    }

    // Do we have a importer to load the file?
    const auto extension = fileInfo.suffix().toLower();
    auto importer = m_extensionsMap.value(extension, nullptr);
    if (!importer) {
        if (error)
            *error = QStringLiteral("unsupported file extension %1").arg(extension);
        return ImportState::Unsupported;
    }

    QStringList generatedFiles;
    auto errorString = importer->import(fileInfo.absoluteFilePath(), outputPath, options, &generatedFiles);

    if (!errorString.isEmpty()) {
        if (error) {
            *error = QStringLiteral("%1").arg(errorString);
        }

        return ImportState::IoError;
    }

    // debug output
    for (const auto &file : generatedFiles)
        qDebug() << "generated file: " << file;

    return ImportState::Success;
}

QSSGAssetImportManager::ImportState QSSGAssetImportManager::importFile(const QUrl &url,
                                                                       QSSGSceneDesc::Scene &scene,
                                                                       QString *error)
{
    return importFile(url, scene, {}, error);
}

QSSGAssetImportManager::ImportState QSSGAssetImportManager::importFile(const QUrl &url,
                                                                       QSSGSceneDesc::Scene &scene,
                                                                       const QJsonObject &options,
                                                                       QString *error)
{
    auto importState = ImportState::Unsupported;
    auto it = m_assetImporters.cbegin();
    const auto end = m_assetImporters.cend();
    for (; it != end; ++it) {
        if ((*it)->name() == QLatin1String("assimp"))
            break;
    }

    if (it != end) {
        const auto &importer = *it;
        const auto ret = importer->import(url, options, scene);
        if (!ret.isEmpty()) {
            if (error)
                *error = ret;
            importState = ImportState::IoError;
        } else {
            importState = ImportState::Success;
        }
    }

    return importState;
}

QJsonObject QSSGAssetImportManager::getOptionsForFile(const QString &filename)
{
    QFileInfo fileInfo(filename);

    QJsonObject options;

    // Is this a real file?
    if (fileInfo.exists()) {
        // Do we have a importer to load the file?
        const auto extension = fileInfo.suffix().toLower();
        auto importer = m_extensionsMap.value(extension, nullptr);
        if (importer)
            options = importer->importOptions();
    }

    return options;
}

QSSGAssetImportManager::PluginOptionMaps QSSGAssetImportManager::getAllOptions() const
{
    PluginOptionMaps options;
    for (const auto importer : m_assetImporters)
        options.insert(importer->inputExtensions().join(QChar::fromLatin1(':')), importer->importOptions());
    return options;
}

QHash<QString, QStringList> QSSGAssetImportManager::getSupportedExtensions() const
{
    QHash<QString, QStringList> extensionMap;
    for (const auto importer : std::as_const(m_assetImporters))
        extensionMap.insert(importer->typeDescription(), importer->inputExtensions());
    return extensionMap;
}

QList<QSSGAssetImporterPluginInfo> QSSGAssetImportManager::getImporterPluginInfos() const
{
    QList<QSSGAssetImporterPluginInfo> output;

    for (const QSSGAssetImporter *importer : m_assetImporters) {
        QSSGAssetImporterPluginInfo plugin;
        plugin.name = importer->name();
        plugin.inputExtensions = importer->inputExtensions();
        plugin.outputExtension = importer->outputExtension();
        plugin.type = importer->type();
        plugin.importOptions = importer->importOptions();
        plugin.typeDescription = importer->typeDescription();
        output.push_back(plugin);
    }

    return output;
}

QT_END_NAMESPACE
