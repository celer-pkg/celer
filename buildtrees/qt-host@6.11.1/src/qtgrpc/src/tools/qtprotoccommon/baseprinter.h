// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BASEPRINTER_H
#define BASEPRINTER_H

#include <google/protobuf/descriptor.h>
#include <memory>

#include "options.h"

namespace google::protobuf::io {
class Printer;
} // google::protobuf::io

namespace qtprotoccommon {

using PropertyMap = std::map<std::string, std::string>;

class BasePrinter
{
    BasePrinter(const BasePrinter &) = delete;
    BasePrinter &operator=(const BasePrinter &) = delete;
    BasePrinter(BasePrinter &&) = delete;
    BasePrinter &operator=(BasePrinter &&) = delete;
public:
    explicit BasePrinter(std::shared_ptr<::google::protobuf::io::Printer> printer);

    void printPublicBlock();
    void printPrivateBlock();

    template <typename T>
    void printComments(T *descriptor)
    {
        if (!Options::instance().generateComments())
            return;

        ::google::protobuf::SourceLocation loc;
        if (descriptor->GetSourceLocation(&loc))
            printCommentsImpl(loc.leading_comments);
    }
    void Indent();
    void Outdent();

private:
    void printCommentsImpl(std::string comments);

protected:
    ~BasePrinter();

    std::shared_ptr<::google::protobuf::io::Printer> m_printer;
};

} // namespace qtprotoccommon
#endif // BASEPRINTER_H
