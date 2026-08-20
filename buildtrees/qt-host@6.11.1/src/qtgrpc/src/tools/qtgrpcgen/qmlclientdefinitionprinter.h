// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "descriptorprinterbase.h"

namespace QtGrpc {

class QmlClientDefinitionPrinter :
        public qtprotoccommon::DescriptorPrinterBase<google::protobuf::ServiceDescriptor>
{
public:
    QmlClientDefinitionPrinter(const google::protobuf::ServiceDescriptor *service,
                               const std::shared_ptr<::google::protobuf::io::Printer> &printer);
    void run()
    {
        printOpenNamespace();
        printConstructor();
        printMethods();
        printCloseNamespace();
    }
    void printOpenNamespace();
    void printConstructor();
    void printMethods();
    void printCloseNamespace();
private:
    void printMethod(const ::google::protobuf::MethodDescriptor *method);
};

} // namespace QtGrpc
