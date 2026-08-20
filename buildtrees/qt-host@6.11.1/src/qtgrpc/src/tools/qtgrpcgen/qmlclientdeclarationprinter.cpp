// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "qmlclientdeclarationprinter.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "grpctemplates.h"

using namespace ::QtGrpc;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::compiler;

/*!
   \class QmlClientDeclarationPrinter
   \inmodule qtgrpcgen
   \private

\brief Generates gRPC QML client class declaration.
*/

QmlClientDeclarationPrinter::QmlClientDeclarationPrinter(
        const ::google::protobuf::ServiceDescriptor *service,
        const std::shared_ptr<::google::protobuf::io::Printer> &printer)
    : DescriptorPrinterBase<::google::protobuf::ServiceDescriptor>(
            service, printer, common::produceQmlClientTypeMap(service, nullptr))

{
}

void QmlClientDeclarationPrinter::printOpenNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceTemplate());
    m_printer->Print(m_typeMap,
                     CommonTemplates::ClassMessageForwardDeclarationTemplate());
}

void QmlClientDeclarationPrinter::printStreamSenders()
{
    for (int i = 0; i < m_descriptor->method_count(); ++i) {
        const MethodDescriptor *method = m_descriptor->method(i);
        MethodMap parameters = common::produceMethodMap(method, m_typeMap["classname"]);
        if (method->client_streaming())
            m_printer->Print(parameters, GrpcTemplates::StreamSenderDeclarationQmlTemplate());
    }
}

void QmlClientDeclarationPrinter::printClientClass()
{
    m_printer->Print(m_typeMap, GrpcTemplates::ChildClassDeclarationTemplate());
    m_printer->Print(m_typeMap, GrpcTemplates::ClientQmlDeclarationTemplate());
}

void QmlClientDeclarationPrinter::printConstructor()
{
    Indent();
    m_printer->Print(m_typeMap, GrpcTemplates::ClientConstructorDeclarationTemplate());
    Outdent();
}

void QmlClientDeclarationPrinter::printClientMethodsDeclaration()
{
    Indent();
    for (int i = 0; i < m_descriptor->method_count(); ++i) {
        const MethodDescriptor *method = m_descriptor->method(i);
        MethodMap parameters = common::produceMethodMap(method, m_typeMap["classname"]);
        if (method->server_streaming() && method->client_streaming()) {
            m_printer->Print(parameters,
                             GrpcTemplates::ClientMethodBidiStreamDeclarationQmlTemplate());
        } else if (method->server_streaming()) {
            m_printer->Print(parameters,
                             GrpcTemplates::ClientMethodServerStreamDeclarationQmlTemplate());
        } else if (method->client_streaming()) {
            m_printer->Print(parameters,
                             GrpcTemplates::ClientMethodClientStreamDeclarationQmlTemplate());
        } else {
            m_printer->Print(parameters, GrpcTemplates::ClientMethodDeclarationQmlTemplate());
        }
    }
    Outdent();
}

void QmlClientDeclarationPrinter::printCloseNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceClosingTemplate());
}
