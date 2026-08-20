// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "qmlclientdefinitionprinter.h"

#include <google/protobuf/io/zero_copy_stream.h>

#include "generatorcommon.h"
#include "commontemplates.h"
#include "grpctemplates.h"

using namespace ::QtGrpc;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::compiler;

/*!
    \class QmlClientDefinitionPrinter
    \inmodule qtgrpcgen
    \private

\brief Generates gRPC QML client class definition.
*/
QmlClientDefinitionPrinter::QmlClientDefinitionPrinter(
        const google::protobuf::ServiceDescriptor *service,
        const std::shared_ptr<::google::protobuf::io::Printer> &printer)
    : DescriptorPrinterBase<google::protobuf::ServiceDescriptor>(
            service, printer, common::produceQmlClientTypeMap(service, nullptr))
{
}

void QmlClientDefinitionPrinter::printOpenNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceTemplate());
    m_printer->Print({ { "namespace", "Qt::StringLiterals" } }, CommonTemplates::UseNamespace());
}

void QmlClientDefinitionPrinter::printMethods()
{
    for (int i = 0; i < m_descriptor->method_count(); ++i) {
        const MethodDescriptor *method = m_descriptor->method(i);
        printMethod(method);
    }
}

void QmlClientDefinitionPrinter::printMethod(const MethodDescriptor *method)
{
    MethodMap parameters = common::produceMethodMap(method, m_typeMap["classname"]);
    if (method->server_streaming() && method->client_streaming()) {
        m_printer->Print(parameters, GrpcTemplates::ClientMethodBidiStreamDefinitionQmlTemplate());
    } else if (method->server_streaming()) {
        m_printer->Print(parameters,
                         GrpcTemplates::ClientMethodServerStreamDefinitionQmlTemplate());
    } else if (method->client_streaming()) {
        m_printer->Print(parameters,
                         GrpcTemplates::ClientMethodClientStreamDefinitionQmlTemplate());
    } else {
        m_printer->Print(parameters, GrpcTemplates::ClientMethodDefinitionQmlTemplate());
    }
}

void QmlClientDefinitionPrinter::printConstructor()
{
    m_printer->Print({ { "classname", m_typeMap["classname"] },
                       { "parent_class", m_typeMap["parent_class"] },
                       { "service_name", m_descriptor->full_name() } },
                     GrpcTemplates::ClientQmlConstructorDefinitionTemplate());
}

void QmlClientDefinitionPrinter::printCloseNamespace()
{
    m_printer->Print({ { "scope_namespaces", m_typeMap["scope_type"] } },
                     CommonTemplates::NamespaceClosingTemplate());
}
