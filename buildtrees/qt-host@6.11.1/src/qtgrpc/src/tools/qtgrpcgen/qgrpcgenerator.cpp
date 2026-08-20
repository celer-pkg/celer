// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "qgrpcgenerator.h"
#include "clientdeclarationprinter.h"
#include "clientdefinitionprinter.h"
#include "qmlclientdeclarationprinter.h"
#include "qmlclientdefinitionprinter.h"
#include "serverdeclarationprinter.h"

#include "grpctemplates.h"
#include "utils.h"
#include "options.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>

using namespace ::QtGrpc;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::google::protobuf::compiler;

namespace {

const utils::ExternalIncludesOrderedSet &externalQmlIncludes()
{
    static const utils::ExternalIncludesOrderedSet externalQmlIncludes{
        "QtQml/qqmlengine.h",
        "QtQml/qjsengine.h",
        "QtQml/qjsvalue.h",
        "QtGrpcQuick/qqmlgrpcfunctionalhandlers.h",
        "QtGrpcQuick/qtqmlgrpcstreamsender.h",
        "QtGrpcQuick/qqmlgrpccalloptions.h"
    };

    return externalQmlIncludes;
}

const utils::ExternalIncludesOrderedSet &externalIncludes()
{
    static const utils::ExternalIncludesOrderedSet externalIncludes{
        "QtGrpc/qgrpcclientbase.h",
        "QtGrpc/qgrpccallreply.h",
        "QtGrpc/qgrpcstream.h",
    };
    return externalIncludes;
}

const std::set<std::string> &systemIncludes()
{
    static const std::set<std::string> systemIncludes{
        "memory",
    };
    return systemIncludes;
}

} // namespace

QGrpcGenerator::QGrpcGenerator() : GeneratorBase()
{}

QGrpcGenerator::~QGrpcGenerator() = default;

bool QGrpcGenerator::Generate(const FileDescriptor *file,
                              [[maybe_unused]] const std::string &parameter,
                              GeneratorContext *generatorContext,
                              [[maybe_unused]] std::string *error) const
{
    assert(file != nullptr && generatorContext != nullptr);

    return GenerateClientServices(file, generatorContext);
}

std::set<std::string> QGrpcGenerator::GetInternalIncludes(const FileDescriptor *file)
{
    std::set<std::string> includes;
    std::string fullSuffix = CommonTemplates::ProtoFileSuffix();
    fullSuffix += CommonTemplates::HeaderSuffix();

    assert(file != nullptr);
    for (int i = 0; i < file->service_count(); ++i) {
        const ServiceDescriptor *service = file->service(i);
        for (int j = 0; j < service->method_count(); ++j) {
            const MethodDescriptor *method = service->method(j);
            if (method->input_type()->file() != service->file()) {
                includes.insert(utils::removeFileSuffix(method->input_type()->file()->name())
                                + fullSuffix);
            }

            if (method->output_type()->file() != service->file()) {
                includes.insert(utils::removeFileSuffix(method->output_type()->file()->name())
                                + fullSuffix);
            }
        }
    }
    if (file->message_type_count() > 0) {
        includes.insert(common::generateRelativeFilePath(file,
                                                         utils::extractFileBasename(file->name()))
                        + fullSuffix);
    }
    return includes;
}

template <typename ServicePrinterT>
void QGrpcGenerator::RunPrinter(const FileDescriptor *file, std::shared_ptr<Printer> printer) const
{
    assert(file != nullptr);
    OpenFileNamespaces(file, printer.get());
    for (int i = 0; i < file->service_count(); ++i) {
        const ServiceDescriptor *service = file->service(i);

        ServicePrinterT servicePrinter(service, printer);
        servicePrinter.run();
    }
    CloseFileNamespaces(file, printer.get());
}

void QGrpcGenerator::GenerateQmlClientServices(
        const ::google::protobuf::FileDescriptor *file,
        ::google::protobuf::compiler::GeneratorContext *generatorContext) const
{
    assert(file != nullptr);
    assert(generatorContext != nullptr);

    const std::string qmlPrefix = "qml";

    const std::string basename = utils::extractFileBasename(file->name()) +
        GrpcTemplates::GrpcClientFileSuffix() + CommonTemplates::ProtoFileSuffix();
    const std::string qmlBasename = qmlPrefix + basename;

    const std::string realtivePath = common::generateRelativeFilePath(file, basename);
    const std::string qmlRealtivePath = common::generateRelativeFilePath(file, qmlBasename);

    // QML registered client class
    std::unique_ptr<ZeroCopyOutputStream>
        clientQmlHeaderStream(generatorContext->Open(qmlRealtivePath
                                                     + CommonTemplates::HeaderSuffix()));
    std::unique_ptr<ZeroCopyOutputStream> clientQmlSourceStream(
                generatorContext->Open(qmlRealtivePath + ".cpp"));

    std::shared_ptr<Printer> qmlHeaderPrinter(new Printer(clientQmlHeaderStream.get(), '$'));
    std::shared_ptr<Printer> qmlSourcePrinter(new Printer(clientQmlSourceStream.get(), '$'));

    printDisclaimer(qmlHeaderPrinter.get());
    printDisclaimer(qmlSourcePrinter.get());

    std::string headerGuard = common::headerGuardFromFilename(qmlBasename
                                                              + CommonTemplates::HeaderSuffix());
    QGrpcGenerator::printHeaderGuardBegin(qmlHeaderPrinter.get(), headerGuard);

    printIncludes(qmlHeaderPrinter.get(), { realtivePath + CommonTemplates::HeaderSuffix() },
                  externalQmlIncludes(), {});

    qmlSourcePrinter->Print(
        {
            { "include", qmlRealtivePath + CommonTemplates::HeaderSuffix() }
    },
        CommonTemplates::InternalIncludeTemplate());

    QGrpcGenerator::RunPrinter<QmlClientDeclarationPrinter>(file, qmlHeaderPrinter);
    QGrpcGenerator::RunPrinter<QmlClientDefinitionPrinter>(file, qmlSourcePrinter);

    QGrpcGenerator::printHeaderGuardEnd(qmlHeaderPrinter.get(), headerGuard);
}

bool QGrpcGenerator::GenerateClientServices(const FileDescriptor *file,
                                            GeneratorContext *generatorContext) const
{
    assert(file != nullptr && generatorContext != nullptr);
    if (file->service_count() <= 0)
        return true;

    const std::string basename = utils::extractFileBasename(file->name()) +
        GrpcTemplates::GrpcClientFileSuffix() + CommonTemplates::ProtoFileSuffix();
    std::string identifier = utils::toValidIdentifier(basename);
    const std::string realtivePath = common::generateRelativeFilePath(file, basename);

    // Generate QML class
    if (Options::instance().hasQml())
        GenerateQmlClientServices(file, generatorContext);

    // CPP client class
    std::unique_ptr<ZeroCopyOutputStream>
        clientHeaderStream(generatorContext->Open(realtivePath + CommonTemplates::HeaderSuffix()));
    std::unique_ptr<ZeroCopyOutputStream> clientSourceStream(
                generatorContext->Open(realtivePath + ".cpp"));

    std::shared_ptr<Printer> clientHeaderPrinter(new Printer(clientHeaderStream.get(), '$'));
    std::shared_ptr<Printer> clientSourcePrinter(new Printer(clientSourceStream.get(), '$'));

    printDisclaimer(clientHeaderPrinter.get());
    printDisclaimer(clientSourcePrinter.get());

    const std::string
        headerGuard = common::headerGuardFromFilename(identifier + CommonTemplates::HeaderSuffix());
    QGrpcGenerator::printHeaderGuardBegin(clientHeaderPrinter.get(), headerGuard);

    clientSourcePrinter->Print(
        {
            { "include", realtivePath + CommonTemplates::HeaderSuffix() }
    },
        CommonTemplates::InternalIncludeTemplate());

    std::set<std::string> internalIncludes = QGrpcGenerator::GetInternalIncludes(file);
    if (auto macroFilename = Options::instance().exportMacroFilename(); !macroFilename.empty())
        internalIncludes.emplace(std::move(macroFilename));

    printIncludes(clientHeaderPrinter.get(), internalIncludes, externalIncludes(),
                  systemIncludes());

    QGrpcGenerator::RunPrinter<ClientDeclarationPrinter>(file, clientHeaderPrinter);
    QGrpcGenerator::RunPrinter<ClientDefinitionPrinter>(file, clientSourcePrinter);

    QGrpcGenerator::printHeaderGuardEnd(clientHeaderPrinter.get(), headerGuard);
    return true;
}

bool QGrpcGenerator::GenerateAll(const std::vector<const FileDescriptor *> &files,
                                 const std::string &parameter, GeneratorContext *generatorContext,
                                 std::string *error) const
{
    Options::setFromString(parameter, qtprotoccommon::Options::QtGrpcGen, error);
    if (!error->empty())
        return false;
    return GeneratorBase::GenerateAll(files, parameter, generatorContext, error);
}
