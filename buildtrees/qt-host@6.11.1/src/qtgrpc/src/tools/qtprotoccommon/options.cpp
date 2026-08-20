// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "options.h"
#include "utils.h"

#include <regex>
#include <string_view>

using namespace ::qtprotoccommon;
static const char QmlPluginOption[] = "QML";
static const char CommentsGenerationOption[] = "COPY_COMMENTS";
static const char FolderGenerationOption[] = "GENERATE_PACKAGE_SUBFOLDERS";
static const char FieldEnumGenerationOption[] = "FIELD_ENUM";
static const char ExtraNamespaceGenerationOption[] = "EXTRA_NAMESPACE";
static const char ExportMacroGenerationOption[] = "EXPORT_MACRO";
static const char HeaderGuardOption[] = "HEADER_GUARD";
static const char MutableGetterConflicts[] = "ALLOW_MUTABLE_GETTER_CONFLICTS";
static const char GenerateNonFinalMessagesOption[] = "GENERATE_NON_FINAL_MESSAGES";

static const char ExportSuffix[] = "_exports.qpb.h";

static constexpr std::string_view HeaderGuardPragma = "pragma";
static constexpr std::string_view HeaderGuardProtoFilename = "filename";

Options::Options()
    : m_generateComments(false), m_isFolder(false), m_generateFieldEnum(true),
      m_generateMacroExportFile(false), m_qml(false), m_mutableGetterConflicts(false)
{
}

Options::~Options() = default;

Options &Options::mutableInstance()
{
    static Options _instance;
    return _instance;
}

const Options &Options::instance()
{
    return mutableInstance();
}

std::string extractCompositeOptionValue(const std::string &option)
{
    std::vector<std::string> compositeOption = utils::split(option, "=");
    if (compositeOption.size() != 2)
        return {};
    const std::string &optionValue = compositeOption.back();
    if (utils::startsWith(optionValue, '\"') && utils::endsWith(optionValue, '\"')) {
        return optionValue.substr(1, optionValue.size() - 2);
    }
    return optionValue;
}

void Options::setFromString(const std::string &options, GeneratorType /*unused*/,
                            std::string *error)
{
    Options &instance = mutableInstance();
    for (const auto &option : utils::split(options, ";")) {
        QT_PROTOBUF_DEBUG("option: " << option);
        if (option == CommentsGenerationOption) {
            QT_PROTOBUF_DEBUG("set m_generateComments: true");
            instance.m_generateComments = true;
        } else if (option == FolderGenerationOption) {
            QT_PROTOBUF_DEBUG("set m_isFolder: true");
            instance.m_isFolder = true;
        } else if (option == FieldEnumGenerationOption) {
            // TODO: Opt in this option by default since it's required for
            // Oneof fields.
            // QT_PROTOBUF_DEBUG("set m_generateFieldEnum: true");
            // instance.m_generateFieldEnum = true;
        } else if (option.find(ExtraNamespaceGenerationOption) == 0) {
            instance.m_extraNamespace = extractCompositeOptionValue(option);
            QT_PROTOBUF_DEBUG("set m_extraNamespace: " << instance.m_extraNamespace);
        } else if (option.find(ExportMacroGenerationOption) == 0) {
            auto export_macro_values = utils::split(extractCompositeOptionValue(option), ":");
            if (!export_macro_values.empty()) {
                static const std::regex valid_c_identifier("[a-zA-Z_][0-9a-zA-Z_]*");
                if (!std::regex_match(export_macro_values[0], valid_c_identifier)) {
                    *error = "EXPORT_MACRO '" + export_macro_values[0]
                        + "' is not a valid C identifier.";
                    return;
                }
                instance.m_exportMacro = export_macro_values[0];
                QT_PROTOBUF_DEBUG("set m_exportMacro: " << instance.m_exportMacro);
                if (export_macro_values.size() > 1) {
                    instance.m_exportMacroFilename = export_macro_values[1];
                    QT_PROTOBUF_DEBUG("set m_exportMacroFilename: "
                                      << instance.m_exportMacroFilename);
                    if (export_macro_values.size() > 2) {
                        instance.m_generateMacroExportFile = export_macro_values[2] == "true";
                        QT_PROTOBUF_DEBUG("set m_generateMacroExportFile: "
                                          << instance.m_generateMacroExportFile);
                    }
                }
                if (instance.m_exportMacroFilename.empty()) {
                    std::string exportMacroLower = instance.m_exportMacro;
                    utils::asciiToLower(exportMacroLower);
                    instance.m_exportMacroFilename = exportMacroLower + ExportSuffix;
                    QT_PROTOBUF_DEBUG("set m_exportMacroFilename: "
                                      << instance.m_exportMacroFilename);
                }
            }
        } else if (option == QmlPluginOption) {
            instance.m_qml = true;
            QT_PROTOBUF_DEBUG("set m_qml: true");
        } else if (option.find(HeaderGuardOption) == 0) {
            const auto headerGuardValue = extractCompositeOptionValue(option);
            if (headerGuardValue == HeaderGuardPragma) {
                instance.m_headerGuard = Options::HeaderGuardType::Pragma;
            } else if (headerGuardValue != HeaderGuardProtoFilename) {
                QT_PROTOBUF_DEBUG("Unknown HEADER_GUARD option value");
            }
        } else if (option == MutableGetterConflicts) {
            instance.m_mutableGetterConflicts = true;
        } else if (option == GenerateNonFinalMessagesOption) {
            instance.m_generateNonFinalMessages = true;
        }
    }
}
