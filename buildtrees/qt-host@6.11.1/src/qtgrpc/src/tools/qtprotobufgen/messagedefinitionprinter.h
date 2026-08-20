// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGEDEFINITIONPRINTER_H
#define MESSAGEDEFINITIONPRINTER_H

#include "descriptorprinterbase.h"

namespace QtProtobuf {

class MessageDefinitionPrinter final
        : public qtprotoccommon::DescriptorPrinterBase<google::protobuf::Descriptor>
{
public:
    explicit MessageDefinitionPrinter(const google::protobuf::Descriptor *message,
                                      std::shared_ptr<::google::protobuf::io::Printer> printer);
    ~MessageDefinitionPrinter();

    void printClassDefinition();
    void printClassRegistration(::google::protobuf::io::Printer *printer);
    void printQmlPluginClassRegistration();

private:
    void printRegisterBody();
    void printFieldsOrdering();
    void printUintData(const char *templateString);
    void printCharData();
    size_t metaCharDataSize() const;
    size_t charDataSize() const;

    void printDataClass();
    void printDataClassConstructor();
    void printDataClassCopy();
    void printDataClassMove();
    void printClassMembers();

    void printConstructors();
    void printInitializationList();
    void printCopyFunctionality();
    void printMoveSemantic();
    void printQVariantOperator();
    void printComparisonOperators();
    void printGetters();
    void printDestructor();
    void printPrivateDurationExtras();
    void printPublicExtras();

    void printClassDefinitionPrivate();
};

} // namespace QtProtobuf
#endif // MESSAGEDEFINITIONPRINTER_H
