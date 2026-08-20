// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Tatyana Borisova <tanusshhka@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "baseprinter.h"
#include "utils.h"
#include "commontemplates.h"
#include <google/protobuf/io/printer.h>

using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::qtprotoccommon;

BasePrinter::BasePrinter(std::shared_ptr<Printer> printer) : m_printer(std::move(printer)) { }

BasePrinter::~BasePrinter() = default;

void BasePrinter::printPublicBlock()
{
    m_printer->Print(CommonTemplates::PublicBlockTemplate());
}

void BasePrinter::printPrivateBlock()
{
    m_printer->Print(CommonTemplates::PrivateBlockTemplate());
}

void BasePrinter::printCommentsImpl(std::string comments)
{
    utils::trim(comments);
    if (!comments.empty()) {
        const bool isSingleLine = !utils::contains(comments, '\n');

        // Prepend a spacing if the comment does not start with a special character that is handled
        // by the documentation tool, like qdoc or doxygen.
        if (comments[0] != '!' && comments[0] != '*') {
            comments = " " + comments;
            if (!isSingleLine)
                comments = "\n" + comments;
        }

        // We print all comments using multi-line semantic.
        m_printer->Print("\n/*");
        if (isSingleLine) {
            m_printer->Print(comments.c_str());
        } else {
            // Align all comment lines by adding asterisks at the beginning of the line.
            comments = utils::replace(comments, "\n", "\n *");
            m_printer->Print(comments.c_str());
            if (!utils::endsWith(comments, '\n'))
                m_printer->Print("\n");
        }
        m_printer->Print(" */");
    }
}

void BasePrinter::Indent()
{
    // The protoc printer prints 2 spaces, so we call it twice
    m_printer->Indent();
    m_printer->Indent();
}

void BasePrinter::Outdent()
{
    m_printer->Outdent();
    m_printer->Outdent();
}
