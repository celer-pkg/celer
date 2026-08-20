// Copyright (C) 2023 The Qt Company Ltd.
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

class QmlClientDeclarationPrinter :
        public qtprotoccommon::DescriptorPrinterBase<::google::protobuf::ServiceDescriptor>
{
public:
    QmlClientDeclarationPrinter(const ::google::protobuf::ServiceDescriptor *service,
                             const std::shared_ptr<::google::protobuf::io::Printer> &printer);
    ~QmlClientDeclarationPrinter() = default;

    void run()
    {
        printOpenNamespace();
        printStreamSenders();
        printClientClass();
        printPublicBlock();
        printConstructor();
        printClientMethodsDeclaration();
        encloseClass();
        printCloseNamespace();
    }

private:
    void printOpenNamespace();
    void printStreamSenders();
    void printClientClass();
    void printConstructor();
    void printClientMethodsDeclaration();
    void printCloseNamespace();
};

} // namespace QtGrpc
