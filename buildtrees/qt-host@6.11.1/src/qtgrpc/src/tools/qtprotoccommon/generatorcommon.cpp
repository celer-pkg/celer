// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "generatorcommon.h"
#include "options.h"
#include "utils.h"
#include "commontemplates.h"

#include "qtprotocdefs.h"

#ifndef HAVE_PROTOBUF_SYNC_PIPER
#  include <google/protobuf/descriptor.pb.h>
#endif

#include <array>
#include <cassert>
#include <string_view>

using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::qtprotoccommon;

namespace {
/*
    Returns the default initializer string for the corresponding field type.
*/
std::string_view getInitializerByFieldType(FieldDescriptor::Type type)
{
    static constexpr std::string_view FloatingPointInitializer = "0.0";
    static constexpr std::string_view IntegerInitializer = "0";
    static constexpr std::string_view BoolInitializer = "false";
    switch (type) {
    case FieldDescriptor::TYPE_DOUBLE:
    case FieldDescriptor::TYPE_FLOAT:
        return FloatingPointInitializer;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_INT32:
    case FieldDescriptor::TYPE_INT64:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_UINT64:
        return IntegerInitializer;
    case FieldDescriptor::TYPE_BOOL:
        return BoolInitializer;
    default:
        break;
    }
    return {};
}
} // namespace

std::set<std::string> common::m_extraNamespacedFiles;

/*
    Constructs a C++ namespace from the full protobuf descriptor name. E.g. for
    the message descriptor "test.protobuf.MessageType" the function
    returns "test::protobuf", if the separator is "::".
*/
std::string common::getFullNamespace(std::string_view fullDescriptorName,
                                     std::string_view separator,
                                     bool extraScope)
{
    std::string output = (extraScope) ? Options::instance().extraNamespace() : "";
    std::string::size_type nameIndex = fullDescriptorName.rfind('.');
    if (nameIndex == std::string::npos)
        return output;
    std::string namespacesStr =
            utils::replace(fullDescriptorName.substr(0, nameIndex), ".", separator);
    if (namespacesStr == "QtCore" || namespacesStr == "QtGui")
        namespacesStr = "QtProtobufPrivate" + std::string(separator) + namespacesStr;
    if (!output.empty() && !namespacesStr.empty())
        output += separator;
    output += namespacesStr;
    return output;
}

std::string common::buildExportMacro(bool addTrailingSpace)
{
    static const std::string &macroOption = Options::instance().exportMacro();
    if (macroOption.empty())
        return macroOption;

    static std::string macro = "QPB_" + macroOption + "_EXPORT";
    std::string resultingMacro = macro;
    if (addTrailingSpace)
        resultingMacro += ' ';
    return resultingMacro;
}
/*
    Constructs a C++ namespace for wrapping nested message types.
    E.g. for the message descriptor with name "test.protobuf.MessageType.NestedMessageType" the
    function returns "test::protobuf::MessageType_QtProtobufNested", if the separator
    is "::".
*/
std::string common::getNestedNamespace(const Descriptor *type, std::string_view separator)
{
    assert(type != nullptr);
    std::string namespaces{ type->file()->package() };

    std::string nestingNamespaces;
    const Descriptor *containingType = type->containing_type();
    while (containingType) {
        nestingNamespaces.insert(0,
                                 std::string(separator)
                                         + utils::capitalizeAsciiName(containingType->name())
                                         + CommonTemplates::QtProtobufNestedNamespace());
        containingType = containingType->containing_type();
    }
    if (!nestingNamespaces.empty())
        namespaces += nestingNamespaces;
    return utils::replace(namespaces, ".", separator);
}

/*
    Cuts the prepending 'scope' namespaces from the original string to create the minimum required
    C++ identifier that can be used inside the scope namespace. Both strings should be C++
    namespaces separated by double colon.
    E.g. for the original namespace "test::protobuf" with the "test" scope the function should
    return "protobuf".
*/
std::string common::getScopeNamespace(std::string_view original, std::string_view scope)
{
    if (scope.empty())
        return std::string(original);

    if (original == scope)
        return "";

    std::string scopeWithSeparator;
    scopeWithSeparator.reserve(scope.size() + 2);
    scopeWithSeparator += scope;
    scopeWithSeparator += "::";

    if (utils::startsWith(original, scopeWithSeparator))
        return std::string(original.substr(scopeWithSeparator.size()));

    return std::string(original);
}

std::map<std::string, std::string> common::getNestedScopeNamespace(const std::string &className)
{
    return { { "scope_namespaces", className + CommonTemplates::QtProtobufNestedNamespace() } };
}

TypeMap common::produceQtTypeMap(const Descriptor *type, const Descriptor *scope)
{
    std::string scopeNamespaces = getScopeNamespace(type, scope);
    std::string qmlPackage = getFullNamespace(type, ".");

    std::string name{ type->name() };
    std::string fullName = name;
    std::string scopeName = name;

    std::string listName = "QList<" + name + ">";

    return { { "type", name },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "list_type", listName },
             { "full_list_type", listName },
             { "scope_list_type", listName },
             { "scope_namespaces", scopeNamespaces },
             { "qml_package", qmlPackage },
             { "property_type", fullName },
             { "property_list_type", listName },
             { "getter_type", scopeName },
             { "setter_type", scopeName } };
}

TypeMap common::produceOverriddenTypeMap(const Descriptor *type, const Descriptor *scope)
{
    std::string namespaces = getFullNamespace(type, "::");
    std::string qmlPackage = getFullNamespace(type, ".");

    std::string name{ type->name() };
    std::string listName;
    if (type->full_name() == "google.protobuf.Any") {
        namespaces = "QtProtobuf";
        name = "QtProtobuf::Any";
        listName = std::string("QList<QtProtobuf::Any>");
        qmlPackage = "QtProtobuf";
    } else {
        listName = std::string("QList<") + name + ">";
    }
    const std::string scopeNamespaces =
            getScopeNamespace(namespaces, getFullNamespace(scope, "::"));
    const std::string fullName = name;
    const std::string scopeName = fullName;

    return { { "type", name },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "list_type", listName },
             { "full_list_type", listName },
             { "scope_list_type", listName },
             { "scope_namespaces", scopeNamespaces },
             { "qml_package", qmlPackage },
             { "property_type", fullName },
             { "property_list_type", listName },
             { "getter_type", scopeName },
             { "setter_type", scopeName },
             { "initializer", "{}" } };
}

TypeMap common::produceMessageTypeMap(const Descriptor *type, const Descriptor *scope)
{
    std::string namespaces = getFullNamespace(type, "::");
    std::string nestedNamespaces = isNested(type) ? getNestedNamespace(type, "::") : namespaces;
    std::string scopeNamespaces = getScopeNamespace(type, scope);
    std::string qmlPackage = getFullNamespace(type, ".");
    if (qmlPackage.empty())
        qmlPackage = "QtProtobuf";

    std::string name = utils::capitalizeAsciiName(type->name());
    std::string fullName = namespaces.empty() ? name : (namespaces + "::" + name);
    std::string scopeName = scopeNamespaces.empty() ? name : (scopeNamespaces + "::" + name);

    std::string listName = "QList<" + name + ">";
    std::string fullListName = namespaces.empty() ? listName : "QList<" + fullName + ">";
    std::string scopeListName = scopeNamespaces.empty() ? listName : "QList<" + scopeName + ">";

    std::string exportMacro = common::buildExportMacro();

    const std::string initializer = "nullptr";
    return { { "classname", name },
             { "dataclassname", name + CommonTemplates::DataClassName() },
             { "classname_low_case", utils::deCapitalizeAsciiName(type->name()) },
             { "type", name },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "list_type", listName },
             { "full_list_type", fullListName },
             { "scope_list_type", scopeListName },
             { "scope_namespaces", scopeNamespaces },
             { "qml_package", qmlPackage },
             { "property_type", fullName },
             { "property_list_type", fullListName },
             { "getter_type", scopeName },
             { "setter_type", scopeName },
             { "export_macro", exportMacro },
             { "initializer", initializer } };
}

TypeMap common::produceEnumTypeMap(const EnumDescriptor *type, const Descriptor *scope)
{
    EnumVisibility visibility = enumVisibility(type, scope);
    std::string namespaces = getFullNamespace(type, "::");

    std::string name = utils::capitalizeAsciiName(type->name());
    // qml package should consist only from proto package
    std::string qmlPackage = getFullNamespace(type, ".");
    if (qmlPackage.empty())
        qmlPackage = "QtProtobuf";

    std::string scopeNamespaces = getScopeNamespace(type, scope);
    std::string enumGadget = scope != nullptr ? utils::capitalizeAsciiName(scope->name()) : "";
    if (visibility == GLOBAL_ENUM) {
        enumGadget = name + CommonTemplates::EnumClassSuffix();
        namespaces += "::";
        namespaces += enumGadget; // Global enums are stored in helper Gadget
        scopeNamespaces = getScopeNamespace(namespaces, getFullNamespace(scope, "::"));
    }

    std::string fullName = namespaces.empty() ? name : (namespaces + "::" + name);
    std::string scopeName = scopeNamespaces.empty() ? name : (scopeNamespaces + "::" + name);

    std::string listName = "QList<" + name + '>';
    std::string fullListName = namespaces.empty() ? listName : "QList<" + fullName + '>';
    std::string scopeListName = scopeNamespaces.empty() ? listName : "QList<" + scopeName + '>';

    // Note: For local enum classes it's impossible to use class name space in Q_PROPERTY
    // declaration. So please avoid addition of namespaces in line bellow
    std::string propertyType = visibility == LOCAL_ENUM ? name : fullName;
    std::string exportMacro =  common::buildExportMacro();

    std::string initializer = scopeName + "::" + common::qualifiedCppName(type->value(0)->name());
    return { { "classname", name },
             { "classname_low_case", utils::deCapitalizeAsciiName(name) },
             { "type", name },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "list_type", listName },
             { "full_list_type", fullListName },
             { "scope_list_type", scopeListName },
             { "scope_namespaces", scopeNamespaces },
             { "qml_package", qmlPackage },
             { "property_type", propertyType },
             { "property_list_type", fullListName },
             { "getter_type", scopeName },
             { "setter_type", scopeName },
             { "enum_gadget", enumGadget },
             { "export_macro", exportMacro },
             { "initializer", initializer } };
}

TypeMap common::produceSimpleTypeMap(FieldDescriptor::Type type)
{
    std::string namespaces;
    if (type != FieldDescriptor::TYPE_STRING && type != FieldDescriptor::TYPE_BYTES
        && type != FieldDescriptor::TYPE_BOOL && type != FieldDescriptor::TYPE_FLOAT
        && type != FieldDescriptor::TYPE_DOUBLE) {
        namespaces = CommonTemplates::QtProtobufNamespace();
    }

    std::string name;
    std::string qmlPackage = CommonTemplates::QtProtobufNamespace();

    auto it = CommonTemplates::TypeReflection().find(type);
    if (it != std::end(CommonTemplates::TypeReflection()))
        name = it->second;
    else
        assert(name.empty());

    std::string fullName = namespaces.empty() ? name : (namespaces + "::" + name);
    std::string listName = name + "List";
    using namespace std::string_literals;
    std::string fullListName = listName;
    if (type != FieldDescriptor::TYPE_STRING && type != FieldDescriptor::TYPE_BYTES)
        fullListName = CommonTemplates::QtProtobufNamespace() + "::"s + listName;
    std::string scopeListName = fullListName;
    const std::string initializer(getInitializerByFieldType(type));
    return { { "type", name },
             { "full_type", fullName },
             { "scope_type", fullName },
             { "list_type", listName },
             { "full_list_type", fullListName },
             { "scope_list_type", scopeListName },
             { "scope_namespaces", namespaces },
             { "qml_package", qmlPackage },
             { "property_type", fullName },
             { "property_list_type", fullListName },
             { "getter_type", fullName },
             { "setter_type", fullName },
             { "initializer", initializer } };
}

MethodMap common::produceMethodMap(const MethodDescriptor *method, const std::string &scope)
{
    std::string inputTypeName{ utils::replace(method->input_type()->full_name(), ".", "::") };
    std::string outputTypeName{ utils::replace(method->output_type()->full_name(), ".", "::")  };
    std::string methodName{ method->name() };
    std::string methodNameUpper{ method->name() };
    methodNameUpper[0] = static_cast<char>(utils::toAsciiUpper(methodNameUpper[0]));

    std::string senderName = std::move(methodNameUpper) + "Sender";

    std::string serviceName{ method->service()->name() };

    //Make sure that we don't clash the same stream names from different services
    std::string senderQmlName = serviceName;
    senderQmlName += senderName;

    std::string streamType;
    if (method->client_streaming() && method->server_streaming()) {
        streamType = "Bidi";
    } else if (method->server_streaming()) {
        streamType = "Server";
    } else if (method->client_streaming()) {
        streamType = "Client";
    }

    std::string streamTypeLower = utils::deCapitalizeAsciiName(streamType);

    static const std::string exportMacro = common::buildExportMacro();

    return {
        {"classname",           scope                              },
        { "sender_class_name",  senderName                         },
        { "sender_qml_name",    senderQmlName                      },
        { "return_type",        outputTypeName                     },
        { "classname_low_case", utils::deCapitalizeAsciiName(scope)},
        { "method_name",        methodName                         },
        { "param_type",         inputTypeName                      },
        { "param_name",         "arg"                              },
        { "stream_type",        streamType                         },
        { "stream_type_lower",  streamTypeLower                    },
        { "return_name",        "ret"                              },
        { "export_macro",       exportMacro                        },
        { "service_name",       serviceName                        },
    };
}

TypeMap common::produceServiceTypeMap(const ServiceDescriptor *service, const Descriptor *scope)
{
    const std::string name{ "Service" };
    const std::string fullName{ "Service" };
    const std::string scopeName{ service->name() };
    static const std::string exportMacro = common::buildExportMacro();

    const std::string namespaces = getFullNamespace(service, "::");
    const std::string scopeNamespaces = getScopeNamespace(namespaces,
                                                          getFullNamespace(scope, "::"));

    return { { "classname", name },
             { "classname_low_case", utils::deCapitalizeAsciiName(name) },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "scope_namespaces", scopeNamespaces },
             { "parent_class", "QAbstractGrpcService" },
             { "export_macro", exportMacro } };
}

TypeMap common::produceClientTypeMap(const ServiceDescriptor *service, const Descriptor *scope)
{
    const std::string name{ "Client" };
    const std::string fullName{ "Client" };
    const std::string scopeName{ service->name() };
    static const std::string exportMacro = common::buildExportMacro();

    const std::string namespaces = getFullNamespace(service, "::");
    const std::string scopeNamespaces = getScopeNamespace(namespaces,
                                                          getFullNamespace(scope, "::"));

    const std::string serviceName{ service->full_name() };
    return { { "classname", name },
             { "classname_low_case", utils::deCapitalizeAsciiName(name) },
             { "full_type", fullName },
             { "scope_type", scopeName },
             { "scope_namespaces", scopeNamespaces },
             { "parent_class", "QGrpcClientBase" },
             { "service_name", serviceName },
             { "export_macro", exportMacro } };
}

TypeMap common::produceQmlClientTypeMap(const ServiceDescriptor *service, const Descriptor *scope)
{
    const std::string name{ "QmlClient" };
    const std::string fullName{ "QmlClient" };
    const std::string serviceName{ service->name() };
    static const std::string exportMacro = common::buildExportMacro();

    const std::string namespaces = getFullNamespace(service, "::");
    const std::string scopeNamespaces = getScopeNamespace(namespaces,
                                                          getFullNamespace(scope, "::"));

    return { { "classname", name },
             { "classname_low_case", utils::deCapitalizeAsciiName(name) },
             { "full_type", fullName },
             { "scope_type", serviceName },
             { "service_name", serviceName },
             { "scope_namespaces", scopeNamespaces },
             { "parent_class", "Client" },
             { "export_macro", exportMacro } };
}

bool common::isQtType(const FieldDescriptor *field)
{
    const auto fullName = field->message_type()->full_name();
    const auto package = field->file()->package();
    return (utils::startsWith(fullName, "QtCore.") || utils::startsWith(fullName, "QtGui."))
            && package != "QtCore" && package != "QtGui";
}

bool common::isOverridden(const FieldDescriptor *field)
{
    return field->type() == FieldDescriptor::TYPE_MESSAGE
            && field->message_type()->full_name() == "google.protobuf.Any";
}

bool common::isPureMessage(const FieldDescriptor *field)
{
    return field->type() == FieldDescriptor::TYPE_MESSAGE && !field->is_map()
            && !field->is_repeated() && !common::isQtType(field) && !common::isOverridden(field);
}

void common::iterateMessageFields(const Descriptor *message, const IterateMessageLogic &callback)
{
    int numFields = message->field_count();
    for (int i = 0; i < numFields; ++i) {
        const FieldDescriptor *field = message->field(i);
        auto propertyMap = common::producePropertyMap(field, message);
        callback(field, propertyMap);
    }
}

void common::iterateOneofFields(const Descriptor *message, const IterateOneofCallback &callback)
{
    int numFields =
#ifdef HAVE_REAL_ONEOF_DECL
        message->real_oneof_decl_count();
#else
        message->oneof_decl_count();
#endif

    for (int i = 0; i < numFields; ++i) {
        const OneofDescriptor *field =
#ifdef HAVE_REAL_ONEOF_DECL
            message->real_oneof_decl(i);
#else
            message->oneof_decl(i);
#endif
#if defined(HAVE_PROTOBUF_SYNC_PIPER) && !defined(HAVE_REAL_ONEOF_DECL)
        if (field->is_synthetic())
            continue;
#endif
        auto propertyMap = common::producePropertyMap(field, message);
        callback(field, propertyMap);
    }
}

TypeMap common::produceTypeMap(const FieldDescriptor *field, const Descriptor *scope)
{
    assert(field != nullptr);

    switch (field->type()) {
    case FieldDescriptor::TYPE_MESSAGE:
        if (isQtType(field))
            return produceQtTypeMap(field->message_type(), nullptr);
        if (isOverridden(field))
            return produceOverriddenTypeMap(field->message_type(), nullptr);
        return produceMessageTypeMap(field->message_type(), scope);
    case FieldDescriptor::TYPE_ENUM:
        return produceEnumTypeMap(field->enum_type(), scope);
    default:
        break;
    }

    return produceSimpleTypeMap(field->type());
}

PropertyMap common::producePropertyMap(const OneofDescriptor *oneof, const Descriptor *scope)
{
    assert(oneof != nullptr);
    assert(scope != nullptr);

    PropertyMap propertyMap;
    propertyMap["optional_property_name"] = qualifiedCppName(qualifiedQmlName(oneof->name()));
    propertyMap["optional_property_name_cap"] = utils::capitalizeAsciiName(oneof->name());
    auto scopeTypeMap = produceMessageTypeMap(scope, nullptr);
    propertyMap["classname"] = scopeTypeMap["classname"];
    propertyMap["dataclassname"] = propertyMap["classname"] + CommonTemplates::DataClassName();
    propertyMap["type"] = propertyMap["optional_property_name_cap"] + "Fields";
    propertyMap["export_macro"] = common::buildExportMacro();

    return propertyMap;
}

PropertyMap common::producePropertyMap(const FieldDescriptor *field, const Descriptor *scope)
{
    assert(field != nullptr);
    assert(scope != nullptr);

    PropertyMap propertyMap = produceTypeMap(field, scope);

    std::string scriptable = "true";
    if (!field->is_map() && !field->is_repeated()
        && (field->type() == FieldDescriptor::TYPE_INT64
            || field->type() == FieldDescriptor::TYPE_SINT64
            || field->type() == FieldDescriptor::TYPE_FIXED64
            || field->type() == FieldDescriptor::TYPE_SFIXED64)) {
        scriptable = "false";
    }

    std::string propertyName = qualifiedCppName(
            qualifiedQmlName(utils::deCapitalizeAsciiName(field->camelcase_name())));
    std::string propertyNameCap = utils::capitalizeAsciiName(propertyName);

    propertyMap["property_name"] = propertyName;
    propertyMap["property_name_cap"] = propertyNameCap;
    propertyMap["mutable_getter_name"] = CommonTemplates::MutableGetterPrefix();
    propertyMap["mutable_getter_name"] += propertyNameCap;

    propertyMap["scriptable"] = scriptable;
    propertyMap["export_macro"] = common::buildExportMacro();

    auto scopeTypeMap = produceMessageTypeMap(scope, nullptr);
    propertyMap["key_type"] = "";
    propertyMap["value_type"] = "";
    propertyMap["classname"] = scopeTypeMap["classname"];
    propertyMap["dataclassname"] = propertyMap["classname"] + CommonTemplates::DataClassName();
    propertyMap["number"] = std::to_string(field->number());

    if (common::isOneofField(field)) {
        propertyMap["optional_property_name"] =
                qualifiedCppName(qualifiedQmlName(field->containing_oneof()->name()));
        propertyMap["optional_property_name_cap"] =
                utils::capitalizeAsciiName(field->containing_oneof()->name());
    } else if (common::isOptionalField(field)) {
        propertyMap["optional_property_name"] = propertyName;
        propertyMap["optional_property_name_cap"] = propertyNameCap;
    }

    if (field->is_map()) {
        const Descriptor *type = field->message_type();
        auto keyMap = common::producePropertyMap(type->field(0), scope);
        auto valueMap = common::producePropertyMap(type->field(1), scope);
        propertyMap["key_type"] = keyMap["scope_type"];
        propertyMap["value_type"] = valueMap["scope_type"];
        propertyMap["value_list_type"] = valueMap["scope_list_type"];
    } else if (field->is_repeated()) {
        propertyMap["getter_type"] = propertyMap["scope_list_type"];
        propertyMap["setter_type"] = propertyMap["scope_list_type"];
    }

    return propertyMap;
}

std::string common::qualifiedCppName(std::string_view name)
{
    return utils::contains(CommonTemplates::ListOfCppExceptions(), name) ? std::string{ name } + "_"
                                                                         : std::string{ name };
}

std::string common::qualifiedQmlName(std::string_view name)
{
    std::string fieldName(name);
    const std::vector<std::string> &searchExceptions = CommonTemplates::ListOfQmlExceptions();

    if (utils::contains(searchExceptions, fieldName))
        return fieldName.append(CommonTemplates::ProtoSuffix());
    return fieldName;
}

bool common::isOneofField(const FieldDescriptor *field)
{
#ifdef HAVE_PROTOBUF_SYNC_PIPER
    return field->real_containing_oneof() != nullptr;
#else
    return field->containing_oneof() != nullptr;
#endif
}

bool common::isTriviallyCopyable(const FieldDescriptor *field)
{
    return !field->is_repeated() &&
        field->type() != FieldDescriptor::TYPE_MESSAGE &&
        field->type() != FieldDescriptor::FieldDescriptor::TYPE_STRING &&
        field->type() != FieldDescriptor::FieldDescriptor::TYPE_BYTES;
}

bool common::isOptionalField(const FieldDescriptor *field)
{
#ifdef HAVE_PROTOBUF_SYNC_PIPER
    bool hasOptional = field->has_presence() && !field->real_containing_oneof();
#else
    bool hasOptional = field->file()->syntax() == FileDescriptor::SYNTAX_PROTO2
        && field->is_optional() && !field->containing_oneof();
#endif
    return field->type() != FieldDescriptor::TYPE_MESSAGE && hasOptional;
}

bool common::isLocalEnum(const EnumDescriptor *type, const Descriptor *scope)
{
    assert(type != nullptr);

    return scope != nullptr && type->containing_type() == scope;
}

common::EnumVisibility common::enumVisibility(const EnumDescriptor *type, const Descriptor *scope)
{
    assert(type != nullptr);

    if (type->containing_type() == nullptr)
        return GLOBAL_ENUM;

    if (isLocalEnum(type, scope)) {
        return LOCAL_ENUM;
    }

    return NEIGHBOR_ENUM;
}

bool common::hasQmlAlias(const FieldDescriptor *field)
{
    return !field->is_map() && !field->is_repeated()
            && (field->type() == FieldDescriptor::TYPE_INT32
                || field->type() == FieldDescriptor::TYPE_SFIXED32
                || field->type() == FieldDescriptor::TYPE_FIXED32)
            && Options::instance().hasQml();
}

bool common::hasNestedTypes(const Descriptor *type)
{
    return common::hasNestedMessages(type) || type->enum_type_count() > 0
            || Options::instance().generateFieldEnum() || type->oneof_decl_count() > 0;
}

void common::iterateMessages(const FileDescriptor *file,
                             const std::function<void(const Descriptor *)> &callback)
{
    int numMessageTypes = file->message_type_count();
    for (int i = 0; i < numMessageTypes; ++i)
        callback(file->message_type(i));
}

void common::iterateNestedMessages(const Descriptor *message,
                                   const std::function<void(const Descriptor *)> &callback)
{
    int numNestedTypes = message->nested_type_count();
    for (int i = 0; i < numNestedTypes; ++i) {
        const Descriptor *nestedMessage = message->nested_type(i);
        if (!isMap(nestedMessage)) {
            callback(nestedMessage);
            continue;
        }
    }
}

bool common::hasNestedMessages(const Descriptor *message)
{
    int numNestedTypes = message->nested_type_count();
    if (numNestedTypes > 0 && message->field_count() == 0)
        return true;

    for (int i = 0; i < numNestedTypes; ++i) {
        const Descriptor *nestedMessage = message->nested_type(i);
        if (!isMap(nestedMessage))
            return true;
    }

    return false;
}

bool common::isNested(const Descriptor *message)
{
    if (message->containing_type() == nullptr)
        return false;

    const Descriptor *containingType = message->containing_type();

    int numFields = containingType->field_count();
    for (int i = 0; i < numFields; ++i) {
        const FieldDescriptor *field = containingType->field(i);
        if (field->message_type() == message) {
            return !field->is_map();
        }
    }

    return true;
}

bool common::isMap(const Descriptor *message)
{
    assert(message != nullptr);
#ifdef HAVE_PROTOBUF_SYNC_PIPER
    return message->map_key() != nullptr;
#else
    return message->options().map_entry();
#endif
}

const Descriptor *common::findHighestMessage(const Descriptor *message)
{
    const Descriptor *highestMessage = message;
    while (highestMessage->containing_type() != nullptr)
        highestMessage = highestMessage->containing_type();
    return highestMessage;
}

std::string common::collectFieldFlags(const FieldDescriptor *field)
{
    constexpr std::string_view flagsConstuctor = "uint(";
    std::string_view separator = " | ";
    std::string_view active_separator;
    std::string flags(flagsConstuctor);

    auto writeFlag = [&](const char *flag) {
        flags += active_separator;
        flags += "QtProtobufPrivate::FieldFlag::";
        flags += flag;
        active_separator = separator;
    };

    if (field->type() != FieldDescriptor::TYPE_STRING
        && field->type() != FieldDescriptor::TYPE_BYTES
        && field->type() != FieldDescriptor::TYPE_MESSAGE
        && !field->is_map() && field->is_repeated()
        && !field->is_packed()) {
        writeFlag("NonPacked");
    }

    if (common::isOneofField(field))
        writeFlag("Oneof");

    if (common::isOptionalField(field))
        writeFlag("Optional");

    if (common::isOneofField(field) || common::isOptionalField(field)
        || common::isPureMessage(field)) {
        writeFlag("ExplicitPresence");
    }

    if (field->is_map())
        writeFlag("Map");

    if (field->is_repeated() && !field->is_map())
        writeFlag("Repeated");

    if (field->type() == FieldDescriptor::TYPE_ENUM)
        writeFlag("Enum");

    if (field->type() == FieldDescriptor::TYPE_MESSAGE)
        writeFlag("Message");

    if (flags == flagsConstuctor)
        writeFlag("NoFlags");
    flags += ")";
    return flags;
}

bool common::isExtraNamespacedFile(std::string_view file)
{
    return m_extraNamespacedFiles.find(std::string{ file }) != m_extraNamespacedFiles.end();
}

void common::setExtraNamespacedFiles(const std::set<std::string> &files)
{
    if (!files.empty() && files != m_extraNamespacedFiles)
        m_extraNamespacedFiles = files;
}

std::string common::headerGuardFromFilename(std::string fileName)
{
    utils::asciiToUpper(fileName);
    return utils::replace(fileName, ".", "_");
}

std::string common::generateRelativeFilePath(const FileDescriptor *file, const std::string &name)
{
    std::string outFileBasename;
    if (Options::instance().isFolder()) {
        outFileBasename = file->package();
        if (!outFileBasename.empty()) {
            outFileBasename = utils::replace(outFileBasename, ".", "/");
            outFileBasename += '/';
        }
    }
    outFileBasename += name;

    return outFileBasename;
}

bool common::hasCustomJsonCoversion(const Descriptor *message)
{
    static constexpr std::array<std::string_view, 11> TypesSupportingOptionalCoversion{
        "Timestamp",   "Duration",   "BoolValue",   "Int32Value", "Int64Value",  "UInt32Value",
        "UInt64Value", "FloatValue", "DoubleValue", "BytesValue", "StringValue",
    };

    return std::find(TypesSupportingOptionalCoversion.begin(),
                     TypesSupportingOptionalCoversion.end(), std::string_view(message->name()))
        != TypesSupportingOptionalCoversion.end();
}
