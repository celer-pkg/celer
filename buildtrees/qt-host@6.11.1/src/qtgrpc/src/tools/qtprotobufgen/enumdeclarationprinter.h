// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ENUMDECLARATIONPRINTER_H
#define ENUMDECLARATIONPRINTER_H

#include "descriptorprinterbase.h"

namespace QtProtobuf {
class EnumDeclarationPrinter final
        : public qtprotoccommon::DescriptorPrinterBase<google::protobuf::EnumDescriptor>
{
public:
    explicit EnumDeclarationPrinter(
            const google::protobuf::EnumDescriptor *descriptor,
            std::shared_ptr<::google::protobuf::io::Printer> printer);
    ~EnumDeclarationPrinter();

    void run()
    {
        startEnum();
        printEnum();
        encloseEnum();
    }

private:
    void startEnum();
    void printEnum();
    void encloseEnum();
};

} // namespace QtProtobuf

#endif // ENUMDECLARATIONPRINTER_H
