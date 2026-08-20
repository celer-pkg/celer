// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "messagedeclarationprinter.h"
#include "utils.h"
#include "options.h"
#include "generatorcommon.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#include <cassert>

using namespace ::QtProtobuf;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::google::protobuf::compiler;

MessageDeclarationPrinter::MessageDeclarationPrinter(const Descriptor *message,
                                                     std::shared_ptr<Printer> printer)
    : DescriptorPrinterBase<Descriptor>(message,
                                        std::move(printer),
                                        common::produceMessageTypeMap(message, nullptr))
{
}

MessageDeclarationPrinter::~MessageDeclarationPrinter() = default;

void MessageDeclarationPrinter::printClassForwardDeclarationPrivate()
{
    m_printer->Print(m_typeMap, CommonTemplates::ClassMessageForwardDeclarationTemplate());

    if (common::hasNestedTypes(m_descriptor)) {
        auto scopeNamespaces = common::getNestedScopeNamespace(m_typeMap["classname"]);
        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceTemplate());

        m_printer->Print({ { "type", CommonTemplates::QtProtobufFieldEnum() } },
                         CommonTemplates::EnumClassForwardDeclarationTemplate());

        for (int i = 0; i < m_descriptor->enum_type_count(); ++i) {
            m_printer->Print(common::produceEnumTypeMap(m_descriptor->enum_type(i), nullptr),
                             CommonTemplates::EnumForwardDeclarationTemplate());
        }

        common::iterateOneofFields(
                m_descriptor, [this](const OneofDescriptor *, PropertyMap typeMap) {
                    m_printer->Print(typeMap,
                                     CommonTemplates::EnumClassForwardDeclarationTemplate());
                });

        common::iterateNestedMessages(m_descriptor, [this](const Descriptor *nestedMessage) {
            MessageDeclarationPrinter nesterPrinter(nestedMessage, m_printer);
            nesterPrinter.printClassForwardDeclarationPrivate();
        });

        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceClosingTemplate());
        m_printer->Print("\n");
    }
}

void MessageDeclarationPrinter::printClassForwardDeclaration()
{
    printClassForwardDeclarationPrivate();
}

void MessageDeclarationPrinter::printClassDeclaration()
{
    printClassDeclarationPrivate();
}

void MessageDeclarationPrinter::printClassDeclarationPrivate()
{
    printComments(m_descriptor);
    printClassDeclarationBegin();
    printClassBody();
    encloseClass();

    if (common::hasNestedTypes(m_descriptor)) {
        auto scopeNamespaces = common::getNestedScopeNamespace(m_typeMap["classname"]);
        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceTemplate());

        static const std::string exportMacro = common::buildExportMacro(false);
        if (!exportMacro.empty()) {
            m_printer->Print({{ "export_macro", exportMacro }},
                             CommonTemplates::QNamespaceDeclarationTemplate());
        } else {
            m_printer->Print(m_typeMap, CommonTemplates::QNamespaceDeclarationNoExportTemplate());
        }

        if (Options::instance().hasQml())
            m_printer->Print(m_typeMap, CommonTemplates::QmlNamedElement());

        m_printer->Print("\n");

        printEnums();

        common::iterateNestedMessages(m_descriptor, [this](const Descriptor *nestedMessage) {
            MessageDeclarationPrinter nesterPrinter(nestedMessage, m_printer);
            nesterPrinter.printClassDeclarationPrivate();
        });
        m_printer->Print(scopeNamespaces, CommonTemplates::NamespaceClosingTemplate());
    }
}

void MessageDeclarationPrinter::printCopyFunctionality()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::CopyConstructorDeclarationTemplate());

    m_printer->Print(m_typeMap, CommonTemplates::AssignmentOperatorDeclarationTemplate());
}

void MessageDeclarationPrinter::printMoveSemantic()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::MoveConstructorDeclarationTemplate());

    m_printer->Print(m_typeMap, CommonTemplates::MoveAssignmentOperatorDeclarationTemplate());

    m_printer->Print(m_typeMap, CommonTemplates::SwapDeclarationTemplate());
}

void MessageDeclarationPrinter::printQVariantOperator()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::QVariantOperatorDeclarationTemplate());
}

void MessageDeclarationPrinter::printComparisonOperators()
{
    assert(m_descriptor != nullptr);
    m_printer->Print(m_typeMap, CommonTemplates::EqualityDeclarationTemplate());
}

void MessageDeclarationPrinter::printConstructors()
{
    m_printer->Print(m_typeMap, CommonTemplates::ConstructorMessageDeclarationTemplate());
}

void MessageDeclarationPrinter::printMaps()
{
    Indent();
    const int numFields = m_descriptor->field_count();
    for (int i = 0; i < numFields; ++i) {
        const FieldDescriptor *field = m_descriptor->field(i);
        if (field->is_map()) {
            m_printer->Print(common::producePropertyMap(field, m_descriptor),
                             CommonTemplates::UsingMapTemplate());
        }
    }
    Outdent();
}

void MessageDeclarationPrinter::printNested()
{
    Indent();
    m_printer->Print({ { "type", CommonTemplates::QtProtobufFieldEnum() },
                       { "scope_namespaces",
                         m_typeMap["classname"] + CommonTemplates::QtProtobufNestedNamespace() } },
                     CommonTemplates::UsingEnumTemplate());

    for (int i = 0; i < m_descriptor->enum_type_count(); ++i) {
        const auto *enumDescr = m_descriptor->enum_type(i);
        auto typeMap = common::produceEnumTypeMap(enumDescr, m_descriptor);
        m_printer->Print(typeMap, CommonTemplates::UsingEnumTemplate());
    }

    common::iterateOneofFields(m_descriptor, [this](const OneofDescriptor *, PropertyMap typeMap) {
        typeMap["scope_namespaces"] =
                m_typeMap["classname"] + CommonTemplates::QtProtobufNestedNamespace();
        m_printer->Print(typeMap, CommonTemplates::UsingEnumTemplate());
    });

    common::iterateNestedMessages(m_descriptor, [&](const Descriptor *nestedMessage) {
        m_printer->Print(common::produceMessageTypeMap(nestedMessage, m_descriptor),
                        CommonTemplates::UsingNestedMessageTemplate());
    });

    Outdent();
}

void MessageDeclarationPrinter::printClassDeclarationBegin()
{
    if (Options::instance().generateNonFinalMessages()) {
        m_printer->Print(m_typeMap, CommonTemplates::NonFinalClassMessageBeginDeclarationTemplate());
    } else {
        m_printer->Print(m_typeMap, CommonTemplates::ClassMessageBeginDeclarationTemplate());
    }

    Indent();
    static const std::string exportMacro = common::buildExportMacro(false);
    if (exportMacro.empty()) {
        m_printer->Print({{ "export_macro", exportMacro }},
                         CommonTemplates::Q_PROTOBUF_OBJECTMacro());
    } else {
        m_printer->Print({{ "export_macro", exportMacro }},
                         CommonTemplates::Q_PROTOBUF_OBJECT_EXPORTMacro());
    }
    Outdent();

    if (Options::instance().hasQml()) {
        m_printer->Print(m_typeMap, CommonTemplates::ClassMessageQmlBeginDeclarationTemplate());
    }
}

void MessageDeclarationPrinter::printProperties()
{
    assert(m_descriptor != nullptr);
    // private section
    Indent();

    const int numFields = m_descriptor->field_count();
    common::iterateMessageFields(
        m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
            const char *propertyTemplate = CommonTemplates::PropertyTemplate();
            if (common::isOneofField(field)) {
                m_printer->Print(propertyMap,
                                 common::isPureMessage(field)
                                     ? CommonTemplates::PropertyOneofMessageTemplate()
                                     : CommonTemplates::PropertyOneofTemplate());
                m_printer->Print(propertyMap, CommonTemplates::PropertyHasFieldTemplate());
                return;
            }

            if (common::isOptionalField(field)) {
                m_printer->Print(propertyMap, CommonTemplates::PropertyOneofTemplate());
                m_printer->Print(propertyMap, CommonTemplates::PropertyHasFieldTemplate());
                return;
            }

            if (common::isPureMessage(field)) {
                m_printer->Print(propertyMap, CommonTemplates::PropertyMessageTemplate());
                m_printer->Print(propertyMap, CommonTemplates::PropertyHasFieldTemplate());
                return;
            }

            if (field->is_repeated() && !field->is_map()) {
                // Non-message list properties don't require an extra QQmlListProperty to access
                // their data, so the property name should not contain the 'Data' suffix
                if (field->type() == FieldDescriptor::TYPE_MESSAGE)
                    propertyTemplate = CommonTemplates::PropertyRepeatedMessageTemplate();
                else
                    propertyTemplate = CommonTemplates::PropertyRepeatedTemplate();
            }
            m_printer->Print(propertyMap, propertyTemplate);
        });

    // Generate extra QML property, that can be used in QML context
    if (Options::instance().hasQml()) {
        for (int i = 0; i < numFields; ++i) {
            const FieldDescriptor *field = m_descriptor->field(i);
            if (common::isPureMessage(field)) {
                m_printer->Print(common::producePropertyMap(field, m_descriptor),
                                 CommonTemplates::PropertyQmlMessageTemplate());
            }
        }
    }

    Outdent();
}

void MessageDeclarationPrinter::printGetters()
{
    Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                printComments(field);
                m_printer->Print("\n");
                if (common::isOneofField(field)) {
                    m_printer->Print(
                            propertyMap,
                            common::isPureMessage(field)
                                    ? CommonTemplates::GetterOneofMessageDeclarationTemplate()
                                    : CommonTemplates::GetterOneofDeclarationTemplate());
                    return;
                }
                if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap,
                                     CommonTemplates::GetterOneofDeclarationTemplate());
                    return;
                }

                switch (field->type()) {
                case FieldDescriptor::TYPE_MESSAGE:
                    if (common::isPureMessage(field)) {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterMessageDeclarationTemplate());
                        m_printer
                            ->Print(propertyMap,
                                    CommonTemplates::GetterMessageMutableDeclarationTemplate());
                        m_printer->Print(propertyMap,
                                         Options::instance().hasQml()
                                             ? CommonTemplates::ClearQmlMessageDeclarationTemplate()
                                             : CommonTemplates::ClearMessageDeclarationTemplate());
                    } else {
                        m_printer->Print(propertyMap,
                                         CommonTemplates::GetterComplexDeclarationTemplate());
                    }
                    break;
                case FieldDescriptor::FieldDescriptor::TYPE_STRING:
                case FieldDescriptor::FieldDescriptor::TYPE_BYTES:
                    m_printer->Print(propertyMap,
                                     CommonTemplates::GetterComplexDeclarationTemplate());
                    break;
                case FieldDescriptor::TYPE_FLOAT:
                case FieldDescriptor::TYPE_DOUBLE:
                default:
                    m_printer->Print(propertyMap,
                                     field->is_repeated()
                                         ? CommonTemplates::GetterComplexDeclarationTemplate()
                                         : CommonTemplates::GetterDeclarationTemplate());
                    break;
                }
            });
    common::iterateOneofFields(
            m_descriptor, [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                m_printer->Print(propertyMap,
                                 CommonTemplates::GetterOneofFieldNumberDeclarationTemplate());
            });
    Outdent();
}

void MessageDeclarationPrinter::printSetters()
{
    Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                bool isTriviallyCopyable = common::isTriviallyCopyable(field);

                if (common::isOneofField(field)) {
                    m_printer
                        ->Print(propertyMap,
                                isTriviallyCopyable
                                    ? CommonTemplates::SetterOneofDeclarationTemplate()
                                    : CommonTemplates::SetterComplexOneofDeclarationTemplate());
                    return;
                }
                if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap,
                                     isTriviallyCopyable
                                         ? CommonTemplates::SetterDeclarationTemplate()
                                         : CommonTemplates::SetterComplexDeclarationTemplate());
                    m_printer->Print(propertyMap,
                                     Options::instance().hasQml()
                                         ? CommonTemplates::ClearQmlOneofDeclarationTemplate()
                                         : CommonTemplates::ClearOneofDeclarationTemplate());
                    return;
                }

                switch (field->type()) {
                case FieldDescriptor::TYPE_MESSAGE:
                    if (!field->is_map() && !field->is_repeated() && !common::isQtType(field)) {
                        m_printer->Print(propertyMap,
                                        CommonTemplates::SetterMessageDeclarationTemplate());
                    } else {
                        m_printer->Print(propertyMap,
                                        CommonTemplates::SetterComplexDeclarationTemplate());
                    }
                    break;
                case FieldDescriptor::FieldDescriptor::TYPE_STRING:
                case FieldDescriptor::FieldDescriptor::TYPE_BYTES:
                    m_printer->Print(propertyMap,
                                     CommonTemplates::SetterComplexDeclarationTemplate());
                    break;
                default:
                    m_printer->Print(propertyMap,
                                     field->is_repeated()
                                         ? CommonTemplates::SetterComplexDeclarationTemplate()
                                         : CommonTemplates::SetterDeclarationTemplate());
                    break;
                }
            });
    common::
        iterateOneofFields(m_descriptor,
                           [&](const OneofDescriptor *, const PropertyMap &propertyMap) {
                               m_printer->Print(propertyMap,
                                           Options::instance().hasQml()
                                               ? CommonTemplates::ClearQmlOneofDeclarationTemplate()
                                               : CommonTemplates::ClearOneofDeclarationTemplate());
                           });
    Outdent();
}

void MessageDeclarationPrinter::printPrivateGetters()
{
    Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                if (common::isOneofField(field)) {
                    m_printer->Print(
                            propertyMap,
                            common::isPureMessage(field)
                                    ? CommonTemplates::
                                            PrivateGetterOneofMessageDeclarationTemplate()
                                    : CommonTemplates::PrivateGetterOneofDeclarationTemplate());
                } else if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap,
                                     CommonTemplates::PrivateGetterOneofDeclarationTemplate());
                } else if (common::isPureMessage(field)) {
                    m_printer->Print(propertyMap,
                                    CommonTemplates::PrivateGetterMessageDeclarationTemplate());
                }
            });
    Outdent();
}

void MessageDeclarationPrinter::printPrivateSetters()
{
    Indent();
    common::iterateMessageFields(
            m_descriptor, [&](const FieldDescriptor *field, const PropertyMap &propertyMap) {
                if (common::isOneofField(field)) {
                    m_printer->Print(
                            propertyMap,
                            common::isPureMessage(field)
                                    ? CommonTemplates::
                                            PrivateSetterOneofMessageDeclarationTemplate()
                                    : CommonTemplates::PrivateSetterOneofDeclarationTemplate());
                } else if (common::isOptionalField(field)) {
                    m_printer->Print(propertyMap,
                                     CommonTemplates::PrivateSetterOneofDeclarationTemplate());
                } else if (common::isPureMessage(field)) {
                    m_printer->Print(propertyMap,
                                    CommonTemplates::PrivateSetterMessageDeclarationTemplate());
                }
            });
    Outdent();
}

void MessageDeclarationPrinter::printEnums()
{
    if (Options::instance().generateFieldEnum())
        printFieldEnum();

    printQEnums();
    printOneofEnums();
}

void MessageDeclarationPrinter::printQEnums()
{
    for (int i = 0; i < m_descriptor->enum_type_count(); ++i) {
        const auto *enumDescr = m_descriptor->enum_type(i);
        auto typeMap = common::produceEnumTypeMap(enumDescr, m_descriptor);
        m_printer->Print(typeMap, CommonTemplates::EnumClassDefinitionTemplate());

        Indent();
        for (int j = 0; j < enumDescr->value_count(); ++j) {
            const auto *valueDescr = enumDescr->value(j);
            m_printer->Print({ { "enumvalue", common::qualifiedCppName(valueDescr->name()) },
                               { "value", std::to_string(valueDescr->number()) } },
                             CommonTemplates::EnumFieldTemplate());
        }
        Outdent();
        m_printer->Print(CommonTemplates::SemicolonBlockEnclosureTemplate());
        m_printer->Print(typeMap, CommonTemplates::QEnumNSTemplate());
    }
}

void MessageDeclarationPrinter::printOneofEnums()
{
    common::iterateOneofFields(
            m_descriptor, [this](const OneofDescriptor *oneofDescr, const PropertyMap &typeMap) {
                m_printer->Print(typeMap, CommonTemplates::EnumClassDefinitionTemplate());
                Indent();
                m_printer->Print({ { "enumvalue", "UninitializedField" },
                                   { "value", "QtProtobuf::InvalidFieldNumber" } },
                                 CommonTemplates::EnumFieldTemplate());
                for (int j = 0; j < oneofDescr->field_count(); ++j) {
                    const auto *valueDescr = oneofDescr->field(j);
                    m_printer->Print({ { "enumvalue",
                                         utils::capitalizeAsciiName(valueDescr->camelcase_name()) },
                                       { "value", std::to_string(valueDescr->number()) } },
                                     CommonTemplates::EnumFieldTemplate());
                }
                Outdent();
                m_printer->Print(CommonTemplates::SemicolonBlockEnclosureTemplate());
                m_printer->Print(typeMap, CommonTemplates::QEnumNSTemplate());
            });
}

void MessageDeclarationPrinter::printPublicExtras()
{
    if (m_descriptor->full_name() == "google.protobuf.Timestamp") {
        static const std::string exportMacro = common::buildExportMacro();
        m_printer->Print({ { "export_macro", exportMacro } },
                         CommonTemplates::QDateTimeExtrasTemplate());
    } else if (m_descriptor->full_name() == "google.protobuf.Duration") {
        static const std::string exportMacro = common::buildExportMacro();
        m_printer->Print({ { "export_macro", exportMacro } },
                         CommonTemplates::DurationExtrasTemplate());
    }
}

void MessageDeclarationPrinter::printClassBody()
{
    printProperties();

    printPublicBlock();
    printNested();
    printMaps();

    Indent();
    printConstructors();
    printDestructor();

    printCopyFunctionality();
    printMoveSemantic();
    printQVariantOperator();

    Outdent();

    printGetters();
    printSetters();

    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::MetaTypeRegistrationDeclaration());
    printPublicExtras();
    Outdent();

    printPrivateBlock();
    Indent();
    printComparisonOperators();
    Outdent();
    printPrivateGetters();
    printPrivateSetters();

    printSharedDataPointer();
}

void MessageDeclarationPrinter::printSharedDataPointer()
{
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::MemberSharedDataPointerTemplate());
    Outdent();
}

void MessageDeclarationPrinter::printDestructor()
{
    m_printer->Print(m_typeMap, CommonTemplates::DestructorMessageDeclarationTemplate());
}

void MessageDeclarationPrinter::printFieldEnum()
{
    if (m_descriptor->field_count() > 0) {
        m_printer->Print(CommonTemplates::FieldEnumTemplate());
        Indent();
        common::iterateMessageFields(m_descriptor,
                                    [&](const FieldDescriptor *, const PropertyMap &propertyMap) {
                                        m_printer->Print(propertyMap,
                                                         CommonTemplates::FieldNumberTemplate());
                                    });
        Outdent();
        m_printer->Print(CommonTemplates::SemicolonBlockEnclosureTemplate());
        m_printer->Print({ { "type", CommonTemplates::QtProtobufFieldEnum() } },
                         CommonTemplates::QEnumNSTemplate());
    }
}
