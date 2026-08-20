// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "serverdeclarationprinter.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "generatorcommon.h"
#include "grpctemplates.h"

using namespace ::QtGrpc;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;

/*!
    \class ServerDeclarationPrinter
    \inmodule qtprotobufgen
    \private

    \brief Generates gRPC server class declaration.
*/

ServerDeclarationPrinter::ServerDeclarationPrinter(
        const ::google::protobuf::ServiceDescriptor *service,
        const std::shared_ptr<::google::protobuf::io::Printer> &printer)
    : DescriptorPrinterBase<::google::protobuf::ServiceDescriptor>(
          service, printer, common::produceServiceTypeMap(service, nullptr))
{
}

void ServerDeclarationPrinter::printOpenNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceTemplate());
}

void ServerDeclarationPrinter::printClassName()
{
    m_printer->Print(m_typeMap, GrpcTemplates::ChildClassDeclarationTemplate());
}

void ServerDeclarationPrinter::printMethodsDeclaration(const char *methodTemplate,
                                                       const char *methodAsyncTemplate,
                                                       const char *methodAsync2Template)
{
    Indent();
    for (int i = 0; i < m_descriptor->method_count(); ++i) {
        const MethodDescriptor *method = m_descriptor->method(i);
        std::map<std::string, std::string> parameters = common::produceMethodMap(
                method, m_typeMap["classname"]);
        m_printer->Print(parameters, methodTemplate);
        m_printer->Print(parameters, methodAsyncTemplate);
        m_printer->Print(parameters, methodAsync2Template);
    }
    Outdent();
}

void ServerDeclarationPrinter::printCloseNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceClosingTemplate());
}
