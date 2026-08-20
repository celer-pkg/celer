// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "descriptorprinterbase.h"

#include <memory>

namespace google::protobuf {
class ServiceDescriptor;
class Message;
}
namespace io {
class Printer;
}

namespace QtGrpc {

class ClientDeclarationPrinter :
        public qtprotoccommon::DescriptorPrinterBase<::google::protobuf::ServiceDescriptor>
{
public:
    ClientDeclarationPrinter(const ::google::protobuf::ServiceDescriptor *service,
                             const std::shared_ptr<::google::protobuf::io::Printer> &printer);
    ~ClientDeclarationPrinter() = default;

    void run()
    {
        printOpenNamespace();
        printClientClass();
        printPublicBlock();
        printConstructorDestructor();
        printClientMethodsDeclaration();
        encloseClass();
        printCloseNamespace();
    }

private:
    void printOpenNamespace();
    void printClientClass();
    void printConstructorDestructor();
    void printClientMethodsDeclaration();
    void printCloseNamespace();
};

} // namespace QtGrpc
