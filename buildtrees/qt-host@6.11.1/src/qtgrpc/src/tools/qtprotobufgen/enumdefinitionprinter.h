// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ENUMDEFINITIONPRINTER_H
#define ENUMDEFINITIONPRINTER_H

#include "descriptorprinterbase.h"

namespace QtProtobuf {
class EnumDefinitionPrinter final
        : public qtprotoccommon::DescriptorPrinterBase<google::protobuf::EnumDescriptor>
{
public:
    explicit EnumDefinitionPrinter(const google::protobuf::EnumDescriptor *descriptor,
                                   std::shared_ptr<::google::protobuf::io::Printer> printer);
    ~EnumDefinitionPrinter();

    void run() { printRegisterBody(); }

    void printRegisterBody();
    void printQmlPluginRegisterBody();
};

} // namespace QtProtobuf

#endif // ENUMDEFINITIONPRINTER_H
