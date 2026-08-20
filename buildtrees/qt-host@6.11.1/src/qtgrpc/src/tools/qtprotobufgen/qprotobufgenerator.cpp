// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "qprotobufgenerator.h"
#include "enumdeclarationprinter.h"
#include "enumdefinitionprinter.h"
#include "messagedeclarationprinter.h"
#include "messagedefinitionprinter.h"

#include "commontemplates.h"
#include "utils.h"
#include "options.h"

#include <cassert>
#include <sstream>
#include <unordered_set>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.h>

using namespace ::QtProtobuf;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::google::protobuf::compiler;

QProtobufGenerator::QProtobufGenerator() : GeneratorBase()
{}

QProtobufGenerator::~QProtobufGenerator() = default;

bool QProtobufGenerator::Generate(const FileDescriptor *file,
                                  [[maybe_unused]] const std::string &parameter,
                                  GeneratorContext *generatorContext, std::string *error) const
{
    assert(file != nullptr);
    assert(generatorContext != nullptr);
    return GenerateMessages(file, generatorContext, error);
}

void QProtobufGenerator::GenerateSources(const FileDescriptor *file,
                                         GeneratorContext *generatorContext, std::string *) const
{
    assert(file != nullptr);
    assert(generatorContext != nullptr);

    std::string basename = utils::extractFileBasename(file->name());
    std::string identifier = utils::toValidIdentifier(basename);
    std::string relativePath = common::generateRelativeFilePath(file, basename);
    std::unique_ptr<io::ZeroCopyOutputStream> sourceStream(
                generatorContext->Open(relativePath + CommonTemplates::ProtoFileSuffix() + ".cpp"));
    std::unique_ptr<io::ZeroCopyOutputStream> registrationStream(
                generatorContext->Open(relativePath + "_qtprotoreg.cpp"));

    std::shared_ptr<Printer> sourcePrinter(new Printer(sourceStream.get(), '$'));
    std::shared_ptr<Printer> registrationPrinter(new Printer(registrationStream.get(), '$'));

    printDisclaimer(sourcePrinter.get());

    utils::ExternalIncludesOrderedSet externalIncludes{ "QtProtobuf/qprotobufregistration.h" };
    std::set<std::string> internalIncludes{ relativePath + CommonTemplates::ProtoFileSuffix()
                                            + CommonTemplates::HeaderSuffix() };

    printIncludes(registrationPrinter.get(), internalIncludes, externalIncludes, {});

    common::iterateMessages(file, [&](const Descriptor *message) {
        if (message->full_name() == "google.protobuf.Timestamp") {
            externalIncludes.insert("QtCore/QTimeZone");
            externalIncludes.insert("QtProtobufWellKnownTypes/private/"
                                    "qprotobufwellknowntypesjsonserializers_p.h");
        } else if (common::hasCustomJsonCoversion(message)) {
            externalIncludes.insert("QtProtobufWellKnownTypes/private/"
                                    "qprotobufwellknowntypesjsonserializers_p.h");
        }
    });

    printIncludes(sourcePrinter.get(), internalIncludes, externalIncludes, { "cmath" });

    OpenFileNamespaces(file, sourcePrinter.get());
    OpenFileNamespaces(file, registrationPrinter.get());

    for (int i = 0; i < file->enum_type_count(); ++i) {
        EnumDefinitionPrinter enumSourceDef(file->enum_type(i), sourcePrinter);
        enumSourceDef.run();
    }

    common::iterateMessages(
                file,
                [&sourcePrinter, &registrationPrinter](const Descriptor *message) {
        MessageDefinitionPrinter messageDef(message, sourcePrinter);
        messageDef.printClassDefinition();
        messageDef.printClassRegistration(registrationPrinter.get());
    });

    registrationPrinter->Print({{"proto_name", utils::capitalizeAsciiName(identifier)}},
                               CommonTemplates::ProtobufTypeRegistrarTemplate());

    CloseFileNamespaces(file, registrationPrinter.get());
    CloseFileNamespaces(file, sourcePrinter.get());

    // Include the moc file:
    sourcePrinter->Print({{"source_file",
                           "moc_" + basename + CommonTemplates::ProtoFileSuffix() + ".cpp"}},
                         CommonTemplates::MocIncludeTemplate());

}

void QProtobufGenerator::GenerateHeader(const FileDescriptor *file,
                                        GeneratorContext *generatorContext,
                                        std::string *error) const
{
    assert(file != nullptr);
    assert(generatorContext != nullptr);
    assert(error != nullptr);

    const std::string basename = utils::extractFileBasename(file->name()) +
        CommonTemplates::ProtoFileSuffix();
    std::string identifier = utils::toValidIdentifier(basename);
    std::string relativePath = common::generateRelativeFilePath(file, basename);

    std::unique_ptr<io::ZeroCopyOutputStream>
        headerStream(generatorContext->Open(relativePath + CommonTemplates::HeaderSuffix()));
    std::shared_ptr<Printer> headerPrinter(new Printer(headerStream.get(), '$'));

    printDisclaimer(headerPrinter.get());

    std::set<std::string> internalIncludes;
    utils::ExternalIncludesOrderedSet externalIncludes;
    std::set<std::string> systemIncludes;

    const std::string
        headerGuard = common::headerGuardFromFilename(identifier + CommonTemplates::HeaderSuffix());
    QProtobufGenerator::printHeaderGuardBegin(headerPrinter.get(), headerGuard);
    if (!Options::instance().exportMacroFilename().empty()) {
        std::string exportMacroFilename = Options::instance().exportMacroFilename();
        internalIncludes.insert(exportMacroFilename);
    }

    externalIncludes.insert("QtCore/qbytearray.h");
    externalIncludes.insert("QtCore/qlist.h");
    externalIncludes.insert("QtCore/qmetatype.h");
    externalIncludes.insert("QtCore/qshareddata.h");
    externalIncludes.insert("QtCore/qstring.h");

    externalIncludes.insert("QtProtobuf/qprotobuflazymessagepointer.h");
    externalIncludes.insert("QtProtobuf/qprotobufmessage.h");
    externalIncludes.insert("QtProtobuf/qprotobufobject.h");
    externalIncludes.insert("QtProtobuf/qtprotobuftypes.h");

    if (Options::instance().hasQml()) {
        externalIncludes.insert("QtQml/qqmlregistration.h");
        externalIncludes.insert("QtQml/qqmllist.h");
    }

    std::unordered_set<std::string> qtTypesSet;

    std::string mutFieldsClashError;
    const auto specialMessageHandling = [&](const Descriptor *message) {
        if (message->oneof_decl_count() > 0)
            externalIncludes.insert("QtProtobuf/qprotobufoneof.h");

        if (message->full_name() == "google.protobuf.Timestamp")
            externalIncludes.insert("QtCore/QDateTime");

        if (message->full_name() == "google.protobuf.Any")
            externalIncludes.insert("QtProtobufWellKnownTypes/qprotobufanysupport.h");

        // We collect the 'mut'-prefixed names of either message fields or
        // the respective mutable getters that will be generated. The name
        // strings are scoped in lambda and we cannot hold any kind of
        // references so store the copy.
        std::unordered_set<std::string> mutPrefixedNames;
        common::iterateMessageFields(
            message, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                if (common::isPureMessage(field)) {
                    const auto getterNameIt = propertyMap.find("mutable_getter_name");
                    assert(getterNameIt != propertyMap.end());

                    const auto propertyNameIt = propertyMap.find("property_name");
                    assert(propertyNameIt != propertyMap.end());

                    // Ensure mut prefix doesn't lead to the name clashing
                    if (mutPrefixedNames.find(getterNameIt->second) != mutPrefixedNames.end()
                        || mutPrefixedNames.find(propertyNameIt->second)
                            != mutPrefixedNames.end()) {
                        std::ostringstream e;
                        e << "Message '" << message->full_name() << "': ";
                        e << "Field '" << field->name() << "' causes a naming conflict";
                        e << "with a mutable getter. This may lead to unintended behavior.";
                        e << "Consider reviewing the guidelines for handling mutable getters: ";
                        e << "https://doc.qt.io/qt-6/qtprotobuf-mutable-getters.html\n";
                        mutFieldsClashError = e.str();
                    }
                    mutPrefixedNames.insert(getterNameIt->second);

                    // We only care about field names that potentially clash.
                    if (utils::startsWith(propertyNameIt->second,
                                          CommonTemplates::MutableGetterPrefix()))
                        mutPrefixedNames.insert(propertyNameIt->second);
                }

                // Collect the special includes
                if (field->type() == FieldDescriptor::TYPE_MESSAGE && !field->is_map()
                    && !field->is_repeated() && common::isQtType(field)) {
                    const std::string package{ field->message_type()->file()->package() };
                    externalIncludes.insert(package + "/"
                                            + std::string{ field->message_type()->name() });
                    qtTypesSet.insert(package);
                }

                if (common::isOptionalField(field))
                    systemIncludes.insert("optional");
            });
    };

    common::iterateMessages(file, [&specialMessageHandling](const Descriptor *message) {
        specialMessageHandling(message);
        common::iterateNestedMessages(message,
                                      [&specialMessageHandling](const Descriptor *message) {
                                          specialMessageHandling(message);
                                      });
    });

    if (!mutFieldsClashError.empty()) {
        if (Options::instance().mutableGetterConflicts()) {
            std::clog << mutFieldsClashError << std::endl;
        } else {
            *error += mutFieldsClashError;
            return;
        }
    }

    for (const auto &qtTypeInclude: qtTypesSet) {
        std::string qtTypeLower = qtTypeInclude;
        std::transform(qtTypeLower.begin(), qtTypeLower.end(),
                       qtTypeLower.begin(), utils::toAsciiLower);
        externalIncludes.insert("QtProtobuf" + qtTypeInclude
                                + "Types/qtprotobuf" + qtTypeLower + "types.h");
    }

    for (int i = 0; i < file->dependency_count(); ++i) {
        if (file->dependency(i)->name() == "QtCore/QtCore.proto"
                || file->dependency(i)->name() == "QtGui/QtGui.proto") {
            continue;
        }
        // Override the any.proto include with our own specific support
        if (file->dependency(i)->name() == "google/protobuf/any.proto") {
            externalIncludes.insert("QtProtobufWellKnownTypes/qprotobufanysupport.h");
            continue;
        }
        internalIncludes.insert(utils::removeFileSuffix(file->dependency(i)->name())
                                + CommonTemplates::ProtoFileSuffix()
                                + CommonTemplates::HeaderSuffix());
    }

    printIncludes(headerPrinter.get(), internalIncludes, externalIncludes, systemIncludes);

    OpenFileNamespaces(file, headerPrinter.get());

    for (int i = 0; i < file->enum_type_count(); ++i) {
        EnumDeclarationPrinter enumDecl(file->enum_type(i), headerPrinter);
        enumDecl.run();
    }

    common::iterateMessages(file, [&headerPrinter](const Descriptor *message) {
        MessageDeclarationPrinter messageDecl(message, headerPrinter);
        messageDecl.printClassForwardDeclaration();
    });

    headerPrinter->Print("#ifdef QT_USE_PROTOBUF_LIST_ALIASES\n");
    common::iterateMessages(file, [&headerPrinter](const Descriptor *message) {
        headerPrinter->Print(common::produceMessageTypeMap(message, nullptr),
                             CommonTemplates::UsingListTemplate());
    });
    headerPrinter->Print("#endif // QT_USE_PROTOBUF_LIST_ALIASES\n");

    common::iterateMessages(
                file,
                [&headerPrinter](const Descriptor *message) {
        MessageDeclarationPrinter messageDecl(message, headerPrinter);
        messageDecl.printClassDeclaration();
    });

    CloseFileNamespaces(file, headerPrinter.get());

    common::iterateMessages(file, [&headerPrinter](const Descriptor *message) {
        MessageDeclarationPrinter messageDef(message, headerPrinter);
    });

    QProtobufGenerator::printHeaderGuardEnd(headerPrinter.get(), headerGuard);
}

bool QProtobufGenerator::GenerateMessages(const FileDescriptor *file,
                                          GeneratorContext *generatorContext,
                                          std::string *error) const
{
    assert(file != nullptr);
    assert(generatorContext != nullptr);
    assert(error != nullptr);

    if (file->message_type_count() <= 0 && file->enum_type_count() <= 0)
        return true;

    GenerateHeader(file, generatorContext, error);
    if (!error->empty())
        return false;

    GenerateSources(file, generatorContext, error);
    return true;
}
