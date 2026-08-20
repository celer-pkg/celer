// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "generatorbase.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>

#include "utils.h"
#include "commontemplates.h"
#include "generatorcommon.h"
#include "options.h"

#include <cassert>

using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::google::protobuf::compiler;
using namespace ::qtprotoccommon;

bool GeneratorBase::GenerateAll(const std::vector<const FileDescriptor *> &files,
                                const std::string &parameter, GeneratorContext *generatorContext,
                                std::string *error) const
{
    assert(!files.empty());
    assert(generatorContext != nullptr);

    Options::setFromString(parameter, Options::QtProtobufGen, error);
    if (!error->empty())
        return false;
    if (Options::instance().generateMacroExportFile()) {
        std::string exportMacroName = Options::instance().exportMacro();
        std::string exportMacroFilename = Options::instance().exportMacroFilename();

        assert(!exportMacroName.empty());
        assert(!exportMacroFilename.empty());

        std::unique_ptr<io::ZeroCopyOutputStream> headerStream(generatorContext
                                                                   ->Open(exportMacroFilename));
        std::shared_ptr<Printer> headerPrinter(new Printer(headerStream.get(), '$'));
        printDisclaimer(headerPrinter.get());
        headerPrinter->Print({ { "export_macro", exportMacroName } },
                             CommonTemplates::ExportMacroTemplate());
        headerPrinter->PrintRaw("\n");
    }
    if (!Options::instance().extraNamespace().empty()) {
        std::set<std::string> extraNamespaceFiles;
        for (const FileDescriptor *file : files) {
            assert(file != nullptr);
            extraNamespaceFiles.insert(std::string{ file->name() });
        }
        common::setExtraNamespacedFiles(extraNamespaceFiles);
    }
    return CodeGenerator::GenerateAll(files, parameter, generatorContext, error);
}

void GeneratorBase::printDisclaimer(Printer *printer)
{
    printer->Print(CommonTemplates::DisclaimerTemplate());
}

void GeneratorBase::OpenFileNamespaces(
        const FileDescriptor *file,
        google::protobuf::io::Printer *printer) const
{
    assert(printer != nullptr);
    assert(file != nullptr);
    const bool hasQtNamespace = (Options::instance().extraNamespace() == "QT_NAMESPACE");

    const std::string scopeNamespaces = common::getFullNamespace(std::string{ file->package() }
                                                                     + ".noop",
                                                                 "::", true);

    printer->Print("\n");
    if (hasQtNamespace || file->package() == "QtCore" || file->package() == "QtGui")
        printer->PrintRaw("QT_BEGIN_NAMESPACE\n");
    if (!scopeNamespaces.empty()) {
        printer->Print({ { "scope_namespaces", scopeNamespaces } },
                       CommonTemplates::NamespaceTemplate());
    }
}

void GeneratorBase::CloseFileNamespaces(
        const ::google::protobuf::FileDescriptor *file,
        google::protobuf::io::Printer *printer) const
{
    assert(printer != nullptr);
    const bool hasQtNamespace = (Options::instance().extraNamespace() == "QT_NAMESPACE");

    const std::string scopeNamespaces = common::getFullNamespace(std::string{ file->package() }
                                                                     + ".noop",
                                                                 "::", true);
    if (!scopeNamespaces.empty()) {
        printer->Print({ { "scope_namespaces", scopeNamespaces } },
                       CommonTemplates::NamespaceClosingTemplate());
    }
    if (hasQtNamespace || file->package() == "QtCore" || file->package() == "QtGui")
        printer->PrintRaw("QT_END_NAMESPACE\n");
    printer->Print("\n");
}

void GeneratorBase::printIncludes(google::protobuf::io::Printer *printer,
                                  const std::set<std::string> &internal,
                                  const utils::ExternalIncludesOrderedSet &external,
                                  const std::set<std::string> &system)
{
    if (!internal.empty())
        printer->Print("\n");
    for (const auto &header : internal)
        printer->Print({ {"include", header } }, CommonTemplates::InternalIncludeTemplate());

    std::string_view includeFirstPart;
    for (const auto &header : external) {
        const auto firstPartPos = header.find_first_of('/');
        if (firstPartPos == std::string::npos) {
            includeFirstPart = {};
            printer->Print("\n");
        } else if (std::string_view currentIncludeFirstPart(header.data(), firstPartPos);
                   currentIncludeFirstPart != includeFirstPart) {
            includeFirstPart = currentIncludeFirstPart;
            printer->Print("\n");
        }
        printer->Print({ {"include", header } }, CommonTemplates::ExternalIncludeTemplate());
    }

    if (!system.empty())
        printer->Print("\n");
    for (const auto &header : system)
        printer->Print({ {"include", header } }, CommonTemplates::ExternalIncludeTemplate());
}

void GeneratorBase::printHeaderGuardBegin(google::protobuf::io::Printer *printer,
                                          const std::string &guard)
{
    switch (Options::instance().headerGuard()) {
    case Options::HeaderGuardType::Pragma:
        printer->Print(CommonTemplates::PragmaOnce());
        break;
    default:
        printer->Print({ { "header_guard", guard } }, CommonTemplates::HeaderGuardBeginTemplate());
        break;
    }
}

void GeneratorBase::printHeaderGuardEnd(google::protobuf::io::Printer *printer,
                                        const std::string &guard)
{
    if (Options::instance().headerGuard() == Options::HeaderGuardType::Pragma)
        return;
    printer->Print({ { "header_guard", guard } }, CommonTemplates::HeaderGuardEndTemplate());
}
