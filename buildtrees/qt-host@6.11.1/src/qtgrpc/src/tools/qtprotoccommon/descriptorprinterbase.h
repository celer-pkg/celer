// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DESCRIPTORPRINTERBASE_H
#define DESCRIPTORPRINTERBASE_H

#include "baseprinter.h"

#include "commontemplates.h"
#include "generatorcommon.h"

namespace qtprotoccommon {

template<typename T>
class DescriptorPrinterBase : public BasePrinter
{
public:
    explicit DescriptorPrinterBase(const T *descriptor,
                                   std::shared_ptr<::google::protobuf::io::Printer> printer,
                                   TypeMap &&typeMap)
        : BasePrinter(printer), m_descriptor(descriptor), m_typeMap(std::move(typeMap))
    {
    }
    ~DescriptorPrinterBase() = default;

    void encloseClass()
    {
        m_printer->Print(CommonTemplates::SemicolonBlockEnclosureTemplate());
    }

protected:
    const T *m_descriptor;
    TypeMap m_typeMap;
};
} // namespace qtprotoccommon

#endif // DESCRIPTORPRINTERBASE_H
