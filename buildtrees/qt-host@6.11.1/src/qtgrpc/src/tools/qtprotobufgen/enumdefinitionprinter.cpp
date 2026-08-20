// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "enumdefinitionprinter.h"

using namespace ::QtProtobuf;
using namespace ::qtprotoccommon;
using namespace ::google::protobuf;
using namespace ::google::protobuf::io;

EnumDefinitionPrinter::EnumDefinitionPrinter(const EnumDescriptor *descriptor,
                                             std::shared_ptr<Printer> printer)
    : DescriptorPrinterBase<EnumDescriptor>(descriptor, std::move(printer),
                                            common::produceEnumTypeMap(descriptor, nullptr))
{
}

EnumDefinitionPrinter::~EnumDefinitionPrinter() = default;

void EnumDefinitionPrinter::printRegisterBody()
{
    m_printer->Print(m_typeMap, CommonTemplates::RegistrarEnumTemplate());
    m_printer->Print(m_typeMap, CommonTemplates::MetaTypeRegistrationGlobalEnumDefinition());
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::MetaTypeRegistrationGlobalEnumTemplate());
    m_printer->Print(m_typeMap, CommonTemplates::RegisterEnumSerializersTemplate());
    Outdent();
    m_printer->Print(CommonTemplates::SimpleBlockEnclosureTemplate());
}

void EnumDefinitionPrinter::printQmlPluginRegisterBody()
{
    Indent();
    m_printer->Print(m_typeMap, CommonTemplates::QmlRegisterGlobalEnumTypeTemplate());
    Outdent();
}
