// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "messagedefinitionprinter.h"

#include <google/protobuf/descriptor.h>
#include "utils.h"
#include "commontemplates.h"
#include "generatorcommon.h"

#include <cassert>

using namespace ::QtProtobuf;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::io;

/*!
    \class MessageDefinitionPrinter
    \inmodule qtprotobufgen
    \private

    \brief Generates gRPC message class definition.
*/
MessageDefinitionPrinter::MessageDefinitionPrinter(const Descriptor *message,
                                                   std::shared_ptr<Printer> printer)
    : DescriptorPrinterBase<Descriptor>(message,
                                        std::move(printer),
                                        common::produceMessageTypeMap(message, nullptr))
{
}

MessageDefinitionPrinter::~MessageDefinitionPrinter() = default;

void MessageDefinitionPrinter::printClassMembers()
{
    Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                if (common::isOneofField(field))
                    return;
                if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap, CommonTemplates::MemberOptionalTemplate());
                } else if (common::isPureMessage(field)) {
                    m_printer->Print(propertyMap, CommonTemplates::MemberMessageTemplate());
                } else if (field->is_repeated() && !field->is_map()) {
                    m_printer->Print(propertyMap, CommonTemplates::MemberRepeatedTemplate());
                } else {
                    m_printer->Print(propertyMap, CommonTemplates::MemberTemplate());
                }
            });

    common::iterateOneofFields(
            m_descriptor, [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                m_printer->Print(propertyMap, CommonTemplates::MemberOneofTemplate());
            });
    Outdent();
}

void MessageDefinitionPrinter::printDataClass()
{
    m_printer->Print(m_typeMap, CommonTemplates::ClassMessageDataBeginDeclarationTemplate());
    printPublicBlock();
    printDataClassConstructor();
    printDataClassCopy();
    printClassMembers();
    encloseClass();
    m_printer->Print("\n");
}

void MessageDefinitionPrinter::printDataClassConstructor()
{
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::ConstructorMessageDataDefinitionTemplate());
    printInitializationList();
    m_printer->Print(CommonTemplates::EmptyBracesTemplate());
    Outdent();
}

void MessageDefinitionPrinter::printDataClassCopy()
{
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::CopyConstructorMessageDataDefinitionTemplate());
    Indent();
    m_printer->Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                if (common::isOneofField(field))
                    return;

                m_printer->Print(",\n");
                m_printer->Print(propertyMap,
                                 common::isPureMessage(field)
                                         ? CommonTemplates::CopyInitializerMemberMessageTemplate()
                                         : CommonTemplates::CopyInitializerMemberTemplate());
            });
    common::iterateOneofFields(
            m_descriptor, [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                m_printer->Print(",\n");
                m_printer->Print(propertyMap,
                                 CommonTemplates::CopyInitializerMemberOneofTemplate());
            });
    m_printer->Outdent();
    Outdent();
    m_printer->Print(CommonTemplates::EmptyBracesTemplate());
    Outdent();
}

void MessageDefinitionPrinter::printClassDefinitionPrivate()
{
    if (common::hasNestedMessages(m_descriptor)) {
        auto scopeNamespaces = qtprotoccommon::common::getNestedScopeNamespace(m_typeMap["classname"]);
        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceTemplate());
        common::iterateNestedMessages(m_descriptor, [this](const Descriptor *nestedMessage) {
            MessageDefinitionPrinter nestedPrinter(nestedMessage, m_printer);
            nestedPrinter.printClassDefinitionPrivate();
        });
        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceClosingTemplate());
    }
    printPrivateDurationExtras();
    printDataClass();

    printDestructor();
    printFieldsOrdering();
    printRegisterBody();
    printConstructors();
    printCopyFunctionality();
    printMoveSemantic();
    printQVariantOperator();
    printComparisonOperators();
    printGetters();
    printPublicExtras();
}

void MessageDefinitionPrinter::printClassDefinition()
{
    printClassDefinitionPrivate();
}

void MessageDefinitionPrinter::printRegisterBody()
{
    std::vector<std::string> registredMetaTypes;

    m_printer->Print(m_typeMap, CommonTemplates::MetaTypeRegistrationMessageDefinition());
    Indent();

    // Special case:
    if (m_descriptor->full_name() == "google.protobuf.Any")
        m_printer->Print("QT_PREPEND_NAMESPACE(QtProtobuf)::Any::registerTypes();\n");

    if (m_descriptor->file()->package() == "google.protobuf") {
        if (common::hasCustomJsonCoversion(m_descriptor)) {
            m_printer->Print({ { "type_name", m_descriptor->name() } },
                CommonTemplates::CustomJsonHanderTemplate());
        }
    }

    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                auto it = propertyMap.find("full_type");
                assert(it != propertyMap.end());
                const auto &fullTypeName = it->second;

                // Check if type is already registered
                if (utils::contains(registredMetaTypes, fullTypeName))
                    return;
                registredMetaTypes.push_back(fullTypeName);
                if (field->is_map()) {
                    m_printer->Print(propertyMap,
                                     CommonTemplates::MetaTypeRegistrationMapTemplate());
                }
            });

    int enumCount = m_descriptor->enum_type_count();
    for (int i = 0; i < enumCount; ++i) {
        m_printer->Print(common::produceEnumTypeMap(m_descriptor->enum_type(i), m_descriptor),
                         CommonTemplates::MetaTypeRegistrationLocalEnumTemplate());
    }

    Outdent();
    m_printer->Print(CommonTemplates::SimpleBlockEnclosureTemplate());
}

void MessageDefinitionPrinter::printFieldsOrdering()
{
    const int fieldCount = m_descriptor->field_count();
    constexpr int MetaFieldsCount = 4;
    constexpr int Version = 0;
    constexpr int NullTerminator = 1;
    constexpr int FakeJsonNameOffset = 1;

    int uint_dataOffset = 1;
    size_t metadataCharSize = metaCharDataSize();
    size_t char_dataSize = charDataSize();
    size_t char_dataSizeTotal = metadataCharSize + NullTerminator + char_dataSize;
    std::map<std::string, std::string> dataVariables = {
        { "version_number", std::to_string(Version) },
        { "classname", m_typeMap["classname"] },
        { "num_fields", std::to_string(fieldCount) },
        { "field_number_offset",
          std::to_string(fieldCount * (uint_dataOffset++) + NullTerminator) },
        { "property_index_offset",
          std::to_string(fieldCount * (uint_dataOffset++) + NullTerminator) },
        { "field_flags_offset", std::to_string(fieldCount * (uint_dataOffset++) + NullTerminator)},
        { "uint_size", std::to_string(fieldCount * MetaFieldsCount + FakeJsonNameOffset) },
        { "char_size", std::to_string(char_dataSizeTotal) },
        { "message_full_name_size", std::to_string(metadataCharSize) },
    };
    assert(uint_dataOffset == MetaFieldsCount);

    m_printer->Print(dataVariables, CommonTemplates::PropertyOrderingDataOpeningTemplate());
    Indent();
    // uint_data
    m_printer->Print("// uint_data\n{\n");
    Indent();

    m_printer->Print("// JSON name offsets:\n");
    printUintData(CommonTemplates::JsonNameOffsetsUintDataTemplate());
    // Include an extra offset which points to the end-of-string, so we can efficiently get the
    // length of all the strings by subtracting adjacent offsets:
    m_printer->Print({ { "json_name_offset", std::to_string(char_dataSizeTotal - NullTerminator) },
                       { "json_name", "end-of-string-marker" } },
                     CommonTemplates::JsonNameOffsetsUintDataTemplate());

    m_printer->Print("// Field numbers:\n");
    printUintData(CommonTemplates::FieldNumbersUintDataTemplate());

    m_printer->Print("// Property indices:\n");
    printUintData(CommonTemplates::QtPropertyIndicesUintDataTemplate());

    m_printer->Print("// Field flags:\n");
    printUintData(CommonTemplates::FieldFlagsUintDataTemplate());

    Outdent();
    m_printer->Print("},\n");

    // char_data
    printCharData();
    Outdent();

    m_printer->Print(m_typeMap, CommonTemplates::PropertyOrderingDataClosingTemplate());

    m_printer->Print(m_typeMap, CommonTemplates::PropertyOrderingDefinitionTemplate());
}

void MessageDefinitionPrinter::printUintData(const char *templateString)
{
    constexpr size_t NullTerminator = 1;
    // JSon data starts where string metadata ends
    size_t jsonOffset = metaCharDataSize() + NullTerminator;
    const int numFields = m_descriptor->field_count();
    for (int i = 0, propertyIndex = 0; i < numFields; ++i, ++propertyIndex) {
        const FieldDescriptor *field = m_descriptor->field(i);
        const std::map<std::string, std::string> variables = {
            { "json_name_offset", std::to_string(jsonOffset)        },
            { "field_number",     std::to_string(field->number())   },
            { "property_index",   std::to_string(propertyIndex)     },
            { "field_flags",      common::collectFieldFlags(field)  },
            { "json_name",        std::string{ field->json_name() } },
        };

        // Oneof and optional properties generate additional has<FieldName> property next to the
        // field property one.
        if (common::isOneofField(field) || common::isOptionalField(field) ||
            common::isPureMessage(field))
            ++propertyIndex;

        m_printer->Print(variables, templateString);

        const size_t length = field->json_name().length();
        jsonOffset += length + NullTerminator;
    }
}

void MessageDefinitionPrinter::printCharData()
{
    m_printer->Print("// char_data\n");

    m_printer->Print("/* metadata char_data: */\n\"");
    m_printer->Print(std::string{m_descriptor->full_name()}.c_str());
    m_printer->Print("\\0\" /* = full message name */\n");

    m_printer->Print("/* field char_data: */\n\"");
    const int numFields = m_descriptor->field_count();
    for (int i = 0; i < numFields; ++i) {
        const FieldDescriptor *field = m_descriptor->field(i);
        if (i && i % 5 == 0) // add some newlines to avoid overly long lines
            m_printer->Print("\"\n\"");
        m_printer->Print({ { "json_name", field->json_name() } }, "$json_name$\\0");
    }
    m_printer->Print("\"\n");
}

size_t MessageDefinitionPrinter::metaCharDataSize() const
{
    return m_descriptor->full_name().size();
}

size_t MessageDefinitionPrinter::charDataSize() const
{
    size_t size = 0;
    const int numFields = m_descriptor->field_count();
    for (int i = 0; i < numFields; ++i) {
        const FieldDescriptor *field = m_descriptor->field(i);
        size += field->json_name().length() + 1; // +1 for the embedded null terminator
    }
    return size + 1; // 1 for the trailing null terminator
}

void MessageDefinitionPrinter::printConstructors()
{
    m_printer->Print(m_typeMap, CommonTemplates::ConstructorMessageDefinitionTemplate());
    m_printer->Print(CommonTemplates::EmptyBracesTemplate());
}

void MessageDefinitionPrinter::printInitializationList()
{
    Indent();
    m_printer->Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, PropertyMap propertyMap) {
                if (field->is_repeated() || common::isOneofField(field)
                    || common::isOptionalField(field))
                    return;

                if (!propertyMap["initializer"].empty()) {
                    m_printer->Print(",\n");
                    m_printer->Print(propertyMap, CommonTemplates::InitializerMemberTemplate());
                }
            });
    m_printer->Outdent();
    Outdent();
}

void MessageDefinitionPrinter::printCopyFunctionality()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::CopyConstructorDefinitionTemplate());
    m_printer->Print(m_typeMap, CommonTemplates::AssignmentOperatorDefinitionTemplate());
}

void MessageDefinitionPrinter::printMoveSemantic()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::MoveConstructorDefinitionTemplate());
}

void MessageDefinitionPrinter::printQVariantOperator()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::QVariantOperatorDefinitionTemplate());
}

void MessageDefinitionPrinter::printComparisonOperators()
{
    assert(m_descriptor != nullptr);

    m_printer->Print(m_typeMap, CommonTemplates::ComparesEqualDefinitionTemplate());

    Indent();
    Indent();
    common::iterateMessageFields(
                m_descriptor, [&](const FieldDescriptor *field, PropertyMap &propertyMap) {
        if (common::isOneofField(field))
            return;
        m_printer->Print("\n&& ");
        if (common::isPureMessage(field)) {
            m_printer->Print(propertyMap,
                             CommonTemplates::EqualOperatorMemberMessageTemplate());
        } else if (field->type() == FieldDescriptor::TYPE_MESSAGE
                   && field->is_repeated()) {
            m_printer->Print(propertyMap,
                             CommonTemplates::EqualOperatorMemberRepeatedTemplate());
        } else {
            m_printer->Print(propertyMap,
                             CommonTemplates::EqualOperatorMemberTemplate());
        }
    });

    common::iterateOneofFields(
            m_descriptor, [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                m_printer->Print("\n&& ");
                m_printer->Print(propertyMap, CommonTemplates::EqualOperatorMemberOneofTemplate());
            });

    Outdent();
    Outdent();

    m_printer->Print(";\n");
    m_printer->Print(CommonTemplates::SimpleBlockEnclosureTemplate());
}

void MessageDefinitionPrinter::printGetters()
{
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, PropertyMap &propertyMap) {
                if (common::isOneofField(field)) {
                    if (common::isPureMessage(field)) {
                        m_printer->Print(
                                propertyMap,
                                CommonTemplates::PrivateGetterOneofMessageDefinitionTemplate());
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterOneofMessageDefinitionTemplate());
                    } else {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::PrivateGetterOneofDefinitionTemplate());
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterOneofDefinitionTemplate());
                    }
                    return;
                }

                if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap,
                                     CommonTemplates::PrivateGetterOptionalDefinitionTemplate());
                    m_printer->Print(propertyMap,
                                     CommonTemplates::GetterOptionalDefinitionTemplate());
                    return;
                }

                switch (field->type()) {
                case FieldDescriptor::TYPE_MESSAGE:
                    if (common::isPureMessage(field)) {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::PrivateGetterMessageDefinitionTemplate());
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterMessageDefinitionTemplate());
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterMessageMutableDefinitionTemplate());

                        m_printer->Print(propertyMap,
                                         CommonTemplates::ClearMessageDefinitionTemplate());
                    } else {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterComplexDefinitionTemplate());
                    }
                    break;
                case FieldDescriptor::FieldDescriptor::TYPE_STRING:
                case FieldDescriptor::FieldDescriptor::TYPE_BYTES:
                    m_printer->Print(propertyMap,
                                     CommonTemplates::GetterComplexDefinitionTemplate());
                    break;
                case FieldDescriptor::TYPE_FLOAT:
                case FieldDescriptor::TYPE_DOUBLE:
                default:
                    m_printer->Print(propertyMap,
                                     field->is_repeated()
                                         ? CommonTemplates::GetterComplexDefinitionTemplate()
                                         : CommonTemplates::GetterDefinitionTemplate());
                    break;
                }

            });

    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, PropertyMap &propertyMap) {
            bool isTriviallyCopyable = common::isTriviallyCopyable(field);
                if (common::isOneofField(field)) {
                    m_printer->Print(propertyMap,
                                     isTriviallyCopyable
                                         ? CommonTemplates::SetterOneofDefinitionTemplate()
                                         : CommonTemplates::SetterComplexOneofDefinitionTemplate());
                    m_printer->Print(
                            propertyMap,
                            common::isPureMessage(field)
                                    ? CommonTemplates::PrivateSetterOneofMessageDefinitionTemplate()
                                    : CommonTemplates::PrivateSetterOneofDefinitionTemplate());
                    return;
                }

                if (common::isOptionalField(field)) {

                    m_printer->Print(propertyMap,
                                     isTriviallyCopyable ?
                                         CommonTemplates::SetterOptionalDefinitionTemplate() :
                                         CommonTemplates::SetterComplexOptionalDefinitionTemplate());
                    m_printer->Print(propertyMap,
                                     CommonTemplates::PrivateSetterOptionalDefinitionTemplate());
                    m_printer->Print(propertyMap,
                                     CommonTemplates::ClearOptionalDefinitionTemplate());
                    return;
                }

                switch (field->type()) {
                case FieldDescriptor::TYPE_MESSAGE:
                    if (common::isPureMessage(field)) {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::PrivateSetterMessageDefinitionTemplate());
                        m_printer->Print(propertyMap,
                                         CommonTemplates::SetterMessageDefinitionTemplate());
                    } else {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::SetterComplexDefinitionTemplate());
                    }
                    break;
                case FieldDescriptor::FieldDescriptor::TYPE_STRING:
                case FieldDescriptor::FieldDescriptor::TYPE_BYTES:
                    m_printer->Print(propertyMap,
                                     CommonTemplates::SetterComplexDefinitionTemplate());
                    break;
                case FieldDescriptor::TYPE_FLOAT:
                case FieldDescriptor::TYPE_DOUBLE:
                    if (!field->is_repeated()) {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::SetterFloatingPointDefinitionTemplate());
                        break;
                    }
                    // fall through
                default:
                    m_printer->Print(propertyMap,
                                     field->is_repeated()
                                         ? CommonTemplates::SetterComplexDefinitionTemplate()
                                         : CommonTemplates::SetterDefinitionTemplate());
                    break;
                }
            });
    common::iterateOneofFields(
            m_descriptor, [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                m_printer->Print(propertyMap,
                                 CommonTemplates::GetterOneofFieldNumberDefinitionTemplate());
                m_printer->Print(propertyMap, CommonTemplates::ClearOneofDefinitionTemplate());
            });
}

void MessageDefinitionPrinter::printDestructor()
{
    m_printer->Print(m_typeMap, "$classname$::~$classname$() = default;\n\n");
}

void MessageDefinitionPrinter::printPrivateDurationExtras()
{
    if (m_descriptor->full_name() == "google.protobuf.Duration") {
        m_printer->Print(
            "constexpr qint64 SecondsRange = 315'576'000'000LL;\n"
            "constexpr qint32 NanosRange = 999'999'999;\n"
            "static constexpr bool checkDurationRange(qint64 seconds, qint32 nanos) noexcept\n"
            "{\n"
            "    if (seconds > SecondsRange || seconds < -SecondsRange) {\n"
            "        qWarning(\"Invalid Duration: seconds=%lld out of range [-%lld, +%lld]\",\n"
            "                 seconds, SecondsRange, -SecondsRange);\n"
            "        return false;\n"
            "    }\n"
            "    if (nanos > NanosRange || nanos < -NanosRange) {\n"
            "        qWarning(\"Invalid Duration: nanos=%d out of range [-%d, +%d]\",\n"
            "                 nanos, NanosRange, -NanosRange);\n"
            "        return false;\n"
            "    }\n"
            "    return true;\n"
            "}\n"
            "static constexpr bool checkSignMatch(qint64 seconds, qint32 nanos) noexcept\n"
            "{\n"
            "    const bool isSignMatch = !(seconds != 0 && nanos != 0 \n"
            "        && (seconds ^ qint64(nanos)) < 0);\n"
            "    if (!isSignMatch) {\n"
            "        qWarning(\"Invalid sign between seconds = %lld and nanos = %d\",\n"
            "                 seconds, nanos);\n"
            "    }\n"
            "    return isSignMatch;\n"
            "}\n\n");
    }
}

void MessageDefinitionPrinter::printPublicExtras()
{
    if (m_descriptor->full_name() == "google.protobuf.Timestamp") {
        m_printer->Print(
                "Timestamp Timestamp::fromDateTime(const QDateTime &dateTime)\n"
                "{\n"
                "    Timestamp ts;\n"
                "    ts.setSeconds(dateTime.toMSecsSinceEpoch() / 1000);\n"
                "    ts.setNanos((dateTime.toMSecsSinceEpoch() % 1000) * 1000000);\n"
                "    return ts;\n"
                "}\n\n"
                "QDateTime Timestamp::toDateTime() const\n"
                "{\n"
                "    return QDateTime::fromMSecsSinceEpoch(\n"
                "            seconds() * 1000 + nanos() / 1000000, QTimeZone(QTimeZone::UTC));\n"
                "}\n\n");
    } else if (m_descriptor->full_name() == "google.protobuf.Duration") {
        m_printer->Print(
                "bool Duration::isValid(QtProtobuf::int64 seconds, QtProtobuf::int32 nanos)\n"
                "{\n"
                "    return checkDurationRange(seconds, nanos) && checkSignMatch(seconds, nanos);\n"
                "}\n\n");
    }
}

void MessageDefinitionPrinter::printClassRegistration(Printer *printer)
{
    if (common::hasNestedMessages(m_descriptor)) {
        auto scopeNamespaces = common::getNestedScopeNamespace(m_typeMap["classname"]);
        printer->Print(scopeNamespaces, CommonTemplates::NamespaceTemplate());
        common::iterateNestedMessages(
                    m_descriptor, [this, &printer](const Descriptor *nestedMessage) {
            MessageDefinitionPrinter nestedPrinter(nestedMessage, m_printer);
            nestedPrinter.printClassRegistration(printer);
        });
        printer->Print(scopeNamespaces, CommonTemplates::NamespaceClosingTemplate());
    }

    printer->Print(m_typeMap, CommonTemplates::RegistrarTemplate());
}

void MessageDefinitionPrinter::printQmlPluginClassRegistration()
{
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::QmlRegisterMessageTypeTemplate());
    Outdent();
}
