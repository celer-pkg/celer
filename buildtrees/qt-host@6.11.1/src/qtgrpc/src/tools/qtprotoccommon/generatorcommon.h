// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENERATORCOMMON_H
#define GENERATORCOMMON_H

#include <google/protobuf/io/printer.h>

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <functional>
#include <cassert>

#include "commontemplates.h"
#include "utils.h"

#include <google/protobuf/descriptor.h>

namespace qtprotoccommon {

using TypeMap = std::map<std::string, std::string>;
using PropertyMap = std::map<std::string, std::string>;
using MethodMap = std::map<std::string, std::string>;

struct common {
    using Descriptor = ::google::protobuf::Descriptor;
    using FieldDescriptor = ::google::protobuf::FieldDescriptor;
    using EnumDescriptor = ::google::protobuf::EnumDescriptor;
    using FileDescriptor = ::google::protobuf::FileDescriptor;
    using OneofDescriptor = ::google::protobuf::OneofDescriptor;
    using MethodDescriptor = ::google::protobuf::MethodDescriptor;
    using ServiceDescriptor = ::google::protobuf::ServiceDescriptor;

    enum EnumVisibility {
        GLOBAL_ENUM,
        LOCAL_ENUM,
        NEIGHBOR_ENUM
    };

    static std::string buildExportMacro(bool addTrailingSpace = true);

    static std::string getFullNamespace(std::string_view fullDescriptorName,
                                        std::string_view separator,
                                        bool extraScope);
    template<typename T>
    static std::string getFullNamespace(const T *type, std::string_view separator)
    {
        if (type == nullptr)
            return {};

        std::string nestingNamespaces;
        if constexpr (std::is_same<T, Descriptor>::value
                      || std::is_same<T, EnumDescriptor>::value) {
            const Descriptor *containingType = type->containing_type();
            while (containingType) {
                nestingNamespaces.insert(
                        0,
                        std::string(separator)
                                + utils::capitalizeAsciiName(containingType->name()));
                containingType = containingType->containing_type();
            }
        }

        return getFullNamespace(std::string{ type->file()->package() } + nestingNamespaces + '.'
                                    + std::string{ type->name() },
                                separator, common::isExtraNamespacedFile(type->file()->name()));
    }

    template<typename T>
    static std::string getScopeNamespace(const T *type, const Descriptor *scope)
    {
        assert(type != nullptr);
        // If types locate in different packages, message can only be identified using
        // full namespace
        if (scope == nullptr || scope->file()->package() != type->file()->package())
            return getFullNamespace(type, "::");

        // All nested messages locate under the namespace but not inside the original message body.
        // This is done due to Qt 'moc' limitations. When calculating the nested namespace we should
        // use this namespace name but not the original containing_type name.
        std::string nestedNameSpaceSuffix = CommonTemplates::QtProtobufNestedNamespace();
        std::string suffix;
        if constexpr (std::is_same<T, Descriptor>::value)
            suffix = !isMap(type) ? CommonTemplates::QtProtobufNestedNamespace() : "";
        else if constexpr (std::is_same<T, EnumDescriptor>::value)
            suffix = CommonTemplates::QtProtobufNestedNamespace();

        const Descriptor *containingType = type->containing_type();
        std::string nestingNamespaces;
        bool first = true;
        while (containingType) {
            nestingNamespaces.insert(0,
                                     utils::capitalizeAsciiName(containingType->name()) + suffix);
            // Scope is detected as parent, it doesn't make sense to go deeper.
            if (containingType == scope)
                break;
            if (first) {
                suffix = nestedNameSpaceSuffix + "::";
                first = false;
            }
            containingType = containingType->containing_type();
        }

        return nestingNamespaces;
    }

    static std::string getNestedNamespace(const Descriptor *type, std::string_view separator);
    static std::string getScopeNamespace(std::string_view original, std::string_view scope);
    static std::map<std::string, std::string> getNestedScopeNamespace(const std::string &className);
    static TypeMap produceQtTypeMap(const Descriptor *type, const Descriptor *scope);
    static TypeMap produceOverriddenTypeMap(const Descriptor *type, const Descriptor *scope);
    static TypeMap produceMessageTypeMap(const Descriptor *type, const Descriptor *scope);
    static TypeMap produceEnumTypeMap(const EnumDescriptor *type, const Descriptor *scope);
    static TypeMap produceSimpleTypeMap(FieldDescriptor::Type type);
    static TypeMap produceTypeMap(const FieldDescriptor *field, const Descriptor *scope);
    static PropertyMap producePropertyMap(const FieldDescriptor *field, const Descriptor *scope);
    static PropertyMap producePropertyMap(const OneofDescriptor *oneof, const Descriptor *scope);
    static MethodMap produceMethodMap(const MethodDescriptor *method, const std::string &scope);
    static TypeMap produceServiceTypeMap(const ServiceDescriptor *service, const Descriptor *scope);
    static TypeMap produceClientTypeMap(const ServiceDescriptor *service, const Descriptor *scope);
    static TypeMap produceQmlClientTypeMap(const ServiceDescriptor *service,
                                           const Descriptor *scope);
    static std::string qualifiedCppName(std::string_view name);
    static std::string qualifiedQmlName(std::string_view name);
    static bool isOneofField(const FieldDescriptor *field);
    static bool isOptionalField(const FieldDescriptor *field);
    static bool isTriviallyCopyable(const FieldDescriptor *field);
    static bool isLocalEnum(const EnumDescriptor *type, const google::protobuf::Descriptor *scope);
    static EnumVisibility enumVisibility(const EnumDescriptor *type, const Descriptor *scope);
    static bool hasQmlAlias(const FieldDescriptor *field);
    static bool hasNestedTypes(const Descriptor *type);
    static bool isQtType(const FieldDescriptor *field);
    static bool isOverridden(const FieldDescriptor *field);
    static bool isPureMessage(const FieldDescriptor *field);

    using IterateMessageLogic = std::function<void(const FieldDescriptor *, PropertyMap &)>;
    static void iterateMessageFields(const Descriptor *message, const IterateMessageLogic &callback);

    using IterateOneofCallback = std::function<void(const OneofDescriptor *, PropertyMap &)>;
    static void iterateOneofFields(const Descriptor *message, const IterateOneofCallback &callback);

    static void iterateMessages(const FileDescriptor *file,
                                const std::function<void(const Descriptor *)> &callback);
    static void iterateNestedMessages(const Descriptor *message,
                                      const std::function<void(const Descriptor *)> &callback);

    static bool hasNestedMessages(const Descriptor *message);

    static bool isNested(const Descriptor *message);
    static bool isMap(const Descriptor *message);
    static const Descriptor *findHighestMessage(const Descriptor *message);

    static std::string collectFieldFlags(const google::protobuf::FieldDescriptor *field);

    static bool isExtraNamespacedFile(std::string_view file);
    static void setExtraNamespacedFiles(const std::set<std::string> &files);

    static std::string headerGuardFromFilename(std::string fileName);

    static std::string generateRelativeFilePath(const ::google::protobuf::FileDescriptor *file,
                                                const std::string &name);

    static bool hasCustomJsonCoversion(const Descriptor *message);

private:
    static std::set<std::string> m_extraNamespacedFiles;
};
} // namespace qtprotoccommon

#endif // GENERATORCOMMON_H
