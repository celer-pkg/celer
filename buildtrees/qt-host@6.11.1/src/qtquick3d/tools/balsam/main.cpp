// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtGui/QGuiApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QVariant>
#include <QtCore/QHash>

#include <QtCore/QJsonObject>

#include <QtGui/QImageReader>

#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>
#include <QtQuick3DIblBaker/private/qssgiblbaker_p.h>

#include <QJsonDocument>
#include <iostream>

class OptionsManager {
public:
    OptionsManager() {

    }
    ~OptionsManager() {
        qDeleteAll(m_optionsMap);
        m_optionsMap.clear();
    }
    void generateCommandLineOptions(const QJsonObject &options)
    {
        if (options.isEmpty() || !options.contains(QStringLiteral("options")))
            return;

        QJsonObject optionsObject = options.value(QStringLiteral("options")).toObject();
        for (const QString &optionsKey : optionsObject.keys()) {
            QJsonObject option = optionsObject.value(optionsKey).toObject();
            QString optionType = option.value(QStringLiteral("type")).toString();
            QString description = option.value(QStringLiteral("description")).toString();
            if (optionType == QStringLiteral("Boolean")) {
                // boolean flags
                m_optionsMap.insert(optionsKey, new QCommandLineOption(optionsKey, description));
                const QString disableKey = QStringLiteral("disable-") + optionsKey;
                m_optionsMap.insert(disableKey, new QCommandLineOption(QStringLiteral("disable-") + optionsKey));
            } else {
                // value types
                if (optionType == QStringLiteral("Real")) {
                    QString defaultValue = QString::number(option.value("value").toDouble());
                    QCommandLineOption *valueOption = new QCommandLineOption(optionsKey, description, optionsKey, defaultValue);
                    m_optionsMap.insert(optionsKey, valueOption);
                }
            }
        }
    }

    QJsonObject processCommandLineOptions(const QCommandLineParser &cmdLineParser, const QJsonObject &options, const QJsonObject &loadedOptions) const
    {
        QJsonObject result = loadedOptions;
        if (options.isEmpty() || !options.contains(QStringLiteral("options")))
            return result;

        QJsonObject optionsObject = options.value(QStringLiteral("options")).toObject();
        QJsonObject loadedOptionsObject = loadedOptions.value(QStringLiteral("options")).toObject();
        for (const QString &optionsKey : optionsObject.keys()) {
            QJsonObject option = optionsObject.value(optionsKey).toObject();
            QString optionType = option.value(QStringLiteral("type")).toString();
            if (optionType == QStringLiteral("Boolean")) {
                const QString disableKey = QStringLiteral("disable-") + optionsKey;
                if (m_optionsMap[optionsKey] && cmdLineParser.isSet(*m_optionsMap[optionsKey]))
                    option["value"] = true;
                else if (m_optionsMap[disableKey] && cmdLineParser.isSet(*m_optionsMap[disableKey]))
                    option["value"] = false;
                else if (auto loadedValue = loadedOptionsObject.value(optionsKey); !loadedValue.isUndefined())
                    option.insert("value", loadedValue);
            } else if (optionType == QStringLiteral("Real")) {
                if (cmdLineParser.isSet(optionsKey))
                    option["value"] = cmdLineParser.value(optionsKey).toDouble();
                else if (auto loadedValue = loadedOptionsObject.value(optionsKey); !loadedValue.isUndefined())
                    option.insert("value", loadedValue);
            }
            // update values
            optionsObject[optionsKey] = option;
        }

        removeFlagConflicts(cmdLineParser, optionsObject);
        result["options"] = optionsObject;
        return result;
    }

    void registerOptions(QCommandLineParser &parser) {
        for (const auto &cmdLineOption : std::as_const(m_optionsMap))
            parser.addOption(*cmdLineOption);
    }

    void removeFlagConflicts(const QCommandLineParser &cmdLineParser, QJsonObject &options) const
    {
        // "generateNormals" and "generateSmoothNormals" are mutually exclusive. "generateNormals"
        // takes precedence.
        QJsonObject opt;
        if (cmdLineParser.isSet(*m_optionsMap[QStringLiteral("generateNormals")])) {
            opt = options.value(QStringLiteral("generateSmoothNormals")).toObject();
            if (opt[QStringLiteral("value")] == true) {
                opt[QStringLiteral("value")] = false;
                options[QStringLiteral("generateSmoothNormals")] = opt;
                std::cerr << "\"--generateSmoothNormals\" disabled due to \"--generateNormals\".\n";
            }

        } else if (cmdLineParser.isSet(*m_optionsMap[QStringLiteral("generateSmoothNormals")])) {
            opt = options.value(QStringLiteral("generateNormals")).toObject();
            if (opt[QStringLiteral("value")] == true) {
                opt[QStringLiteral("value")] = false;
                options[QStringLiteral("generateNormals")] = opt;
                std::cerr << "\"--generateNormals\" disabled due to \"--generateSmoothNormals\".\n";
            }
        }

        // Ditto for "optimizeGraph" and "preTransformVertices".
        if (cmdLineParser.isSet(*m_optionsMap[QStringLiteral("optimizeGraph")])) {
            opt = options.value(QStringLiteral("preTransformVertices")).toObject();
            if (opt[QStringLiteral("value")] == true) {
                opt[QStringLiteral("value")] = false;
                options[QStringLiteral("preTransformVertices")] = opt;
                std::cerr << "\"--preTransformVertices\" disabled due to \"--optimizeGraph\".\n";
            }
        } else if (cmdLineParser.isSet(*m_optionsMap[QStringLiteral("preTransformVertices")])) {
            opt = options.value(QStringLiteral("optimizeGraph")).toObject();
            if (opt[QStringLiteral("value")] == true) {
                opt[QStringLiteral("value")] = false;
                options[QStringLiteral("optimizeGraph")] = opt;
                std::cerr << "\"--optimizeGraph\" disabled due to \"--preTransformVertices\".\n";
            }
        }
    }

private:
    QHash<QString, QCommandLineOption *> m_optionsMap;
};

struct BuiltinConditioners
{
    QSSGAssetImportManager::ImportState run(const QString &filename,
                                            const QDir &outputPath,
                                            QString *error);

    QSSGIblBaker iblBaker;
};

QSSGAssetImportManager::ImportState BuiltinConditioners::run(const QString &filename,
                                                             const QDir &outputPath,
                                                             QString *error)
{
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists()) {
        if (error)
            *error = QStringLiteral("File does not exist");
        return QSSGAssetImportManager::ImportState::IoError;
    }

    const QString extension = fileInfo.suffix().toLower();
    QStringList generatedFiles;
    QSSGAssetImportManager::ImportState result = QSSGAssetImportManager::ImportState::Unsupported;

    if (iblBaker.inputExtensions().contains(extension)) {
        QString errorMsg = iblBaker.import(fileInfo.absoluteFilePath(), outputPath, &generatedFiles);
        if (errorMsg.isEmpty()) {
            result = QSSGAssetImportManager::ImportState::Success;
        } else {
            *error = errorMsg;
            result = QSSGAssetImportManager::ImportState::IoError;
        }
    } else {
        if (error)
            *error = QStringLiteral("unsupported file extension %1").arg(extension);
    }

    for (const auto &file : generatedFiles)
        qDebug() << "generated file:" << file;

    return result;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QImageReader::setAllocationLimit(0);

    const bool canUsePlugins = !QCoreApplication::arguments().contains(QStringLiteral("--no-plugins"));
    if (!canUsePlugins)
        qDebug("balsam: Not loading assetimporter plugins");

    QScopedPointer<QSSGAssetImportManager> assetImporter;
    OptionsManager optionsManager;
    BuiltinConditioners builtins;

    // Setup command line arguments
    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription(
            QStringLiteral("Converts graphical assets to a runtime format for use with Qt Quick 3D"));
    cmdLineParser.addHelpOption();
    cmdLineParser.addPositionalArgument(QStringLiteral("sourceFilename"), QStringLiteral("Asset file to be imported"));
    QCommandLineOption outputPathOption({ "outputPath", "o" }, QStringLiteral("Sets the location to place the generated file(s). Default is the current directory"), QStringLiteral("outputPath"), QDir::currentPath());
    cmdLineParser.addOption(outputPathOption);
    QCommandLineOption noPluginsOption(QStringLiteral("no-plugins"), QStringLiteral("Disable assetimporter plugin loading, only considers built-ins"));
    cmdLineParser.addOption(noPluginsOption);

    QCommandLineOption loadOptionsFromFileOption({"f","options-file"}, QStringLiteral("Load options from <file>"), QStringLiteral("file"));
    cmdLineParser.addOption(loadOptionsFromFileOption);

    // Get Plugin options
    if (canUsePlugins) {
        assetImporter.reset(new QSSGAssetImportManager);
        auto pluginOptions = assetImporter->getAllOptions();
        for (const auto &options : std::as_const(pluginOptions))
            optionsManager.generateCommandLineOptions(options);
        optionsManager.registerOptions(cmdLineParser);
    }

    cmdLineParser.process(app);

    QStringList assetFileNames = cmdLineParser.positionalArguments();
    QDir outputDirectory = QDir::currentPath();
    if (cmdLineParser.isSet(outputPathOption)) {
        outputDirectory = QDir(cmdLineParser.value(outputPathOption));
        if (!outputDirectory.exists()) {
            if (!outputDirectory.mkpath(QStringLiteral("."))) {
                std::cerr << "Failed to create export directory: " << qPrintable(outputDirectory.path()) << "\n";
                return 2;
            }
        }
    }

    // if there is nothing to do show help
    if (assetFileNames.isEmpty())
        cmdLineParser.showHelp(1);

    // Convert each assetFile is possible
    for (const auto &assetFileName : assetFileNames) {
        QString errorString;
        QSSGAssetImportManager::ImportState result = QSSGAssetImportManager::ImportState::Unsupported;
        if (canUsePlugins) {
            QJsonObject options = assetImporter->getOptionsForFile(assetFileName);
            QJsonObject loadedOptions;

            if (cmdLineParser.isSet(loadOptionsFromFileOption)) {
                QFile optionsFile(cmdLineParser.value(loadOptionsFromFileOption));
                if (!optionsFile.open(QIODevice::ReadOnly)) {
                    qCritical() << "Could not open options file" << optionsFile.fileName() << "for reading.";
                    return -1;
                }
                QByteArray optionData = optionsFile.readAll();
                QJsonParseError error;
                auto optionsDoc = QJsonDocument::fromJson(optionData, &error);
                if (optionsDoc.isEmpty()) {
                    qCritical() << "Could not read options file:" << error.errorString();
                    return -1;
                }
                loadedOptions = optionsDoc.object();
            }

            options = optionsManager.processCommandLineOptions(cmdLineParser, options, loadedOptions);

            // first try the plugin-based asset importer system
            result = assetImporter->importFile(assetFileName, outputDirectory, options, &errorString);
        }
        // if the file extension is unsupported, try the builtins
        if (result == QSSGAssetImportManager::ImportState::Unsupported)
            result = builtins.run(assetFileName, outputDirectory, &errorString);
        if (result != QSSGAssetImportManager::ImportState::Success) {
            std::cerr << "Failed to import file with error: " << qPrintable(errorString) << "\n";
            return 2;
        }
    }

    return 0;
}
