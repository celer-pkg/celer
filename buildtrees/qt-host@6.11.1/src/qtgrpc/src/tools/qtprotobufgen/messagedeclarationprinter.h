// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MESSAGEDECLARATIONPRINTER_H
#define MESSAGEDECLARATIONPRINTER_H

#include "descriptorprinterbase.h"

namespace QtProtobuf {

class MessageDeclarationPrinter final
        : public qtprotoccommon::DescriptorPrinterBase<google::protobuf::Descriptor>
{
public:
    explicit MessageDeclarationPrinter(const ::google::protobuf::Descriptor *message,
                                       std::shared_ptr<::google::protobuf::io::Printer> printer);
    ~MessageDeclarationPrinter();

    void printClassDeclaration();
    void printClassForwardDeclaration();

private:
    void printCopyFunctionality();
    void printMoveSemantic();
    void printQVariantOperator();
    void printComparisonOperators();
    void printClassBody();
    void printProperties();
    void printGetters();
    void printSetters();
    void printPrivateGetters();
    void printPrivateSetters();
    void printSharedDataPointer();
    void printConstructors();
    void printDestructor();
    void printMaps();
    void printNested();
    void printClassDeclarationBegin();

    void printEnums();
    void printFieldEnum();
    void printQEnums();
    void printOneofEnums();
    void printPublicExtras();

    //Recursive functionality
    void printClassDeclarationPrivate();
    void printClassForwardDeclarationPrivate();
};

} // namespace QtProtobuf

#endif // MESSAGEDECLARATIONPRINTER_H
