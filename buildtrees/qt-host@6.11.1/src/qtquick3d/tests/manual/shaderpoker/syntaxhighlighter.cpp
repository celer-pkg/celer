// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "syntaxhighlighter.h"

#include <QtCore/qregularexpression.h>

#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>

#include <QtQuick3DGlslParser/private/glsllexer_p.h>
#include <QtQuick3DGlslParser/private/glslparser_p.h>

QT_BEGIN_NAMESPACE

// Text color and style categories
enum TextStyle : quint8 {
    C_VISUAL_WHITESPACE,
    C_PREPROCESSOR,
    C_NUMBER,
    C_COMMENT,
    C_KEYWORD,
    C_PARAMETER,
    C_REMOVED_LINE
};

static constexpr TextStyle GLSLReservedKeyword = C_REMOVED_LINE;

static QTextCharFormat formatForCategory(TextStyle category)
{
    switch (category) {
    case C_PARAMETER:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0xaa, 0xaa, 0xff));
        return fmt;
    }
    case C_PREPROCESSOR:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0x55, 0x55, 0xff));
        return fmt;
    }
    case C_NUMBER:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0xff, 0x55, 0xff));
        return fmt;
    }
    case C_COMMENT:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0x55, 0xff, 0xff));
        return fmt;
     }
    case C_KEYWORD:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0xff, 0xff, 0x55));
        return fmt;
    }
    case C_REMOVED_LINE:
        Q_FALLTHROUGH();
    case C_VISUAL_WHITESPACE:
        break;
    }

    return QTextCharFormat();
}

SyntaxHighlighter::SyntaxHighlighter(QObject *p)
    : QSyntaxHighlighter(p)
{

}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    if (m_keywords.isEmpty()) {
        const auto keywords = QtQuick3DEditorHelpers::CustomMaterial::preprocessorVars();
        for (const auto &kw : keywords)
            m_keywords.insert(kw);
    }
    if (m_argumentKeywords.isEmpty()) {
        const auto args = QtQuick3DEditorHelpers::CustomMaterial::reservedArgumentNames();
        for (const auto &arg : args)
            m_argumentKeywords.insert(arg);
    }
    const int previousState = previousBlockState();
    int state = 0;
    if (previousState != -1)
        state = previousState & 0xff;

    const QByteArray data = text.toLatin1();
    GLSL::Lexer lex(/*engine=*/ nullptr, data.constData(), data.size());
    lex.setState(state);
    lex.setScanKeywords(false);
    lex.setScanComments(true);

    lex.setVariant(GLSL::Lexer::Variant_GLSL_400);

    QList<GLSL::Token> tokens;
    GLSL::Token tk;
    do {
        lex.yylex(&tk);
        tokens.append(tk);
    } while (tk.isNot(GLSL::Parser::EOF_SYMBOL));

    state = lex.state(); // refresh the state

    if (tokens.isEmpty()) {
        setCurrentBlockState(previousState);
        if (!text.isEmpty()) // the empty line can still contain whitespace
            setFormat(0, text.size(), formatForCategory(C_VISUAL_WHITESPACE));
        return;
    }

    for (int i = 0, end = tokens.size(); i != end; ++i) {
        const GLSL::Token &tk = tokens.at(i);

        int previousTokenEnd = 0;
        if (i != 0) {
            // mark the whitespaces
            previousTokenEnd = tokens.at(i - 1).begin() + tokens.at(i - 1).length;
        }

        if (previousTokenEnd != tk.begin())
            setFormat(previousTokenEnd, tk.begin() - previousTokenEnd, formatForCategory(C_VISUAL_WHITESPACE));

        if (tk.is(GLSL::Parser::T_NUMBER)) {
            setFormat(tk.begin(), tk.length, formatForCategory(C_NUMBER));
        } else if (tk.is(GLSL::Parser::T_COMMENT)) {
            highlightLine(text, tk.begin(), tk.length, formatForCategory(C_COMMENT));
        } else if (tk.is(GLSL::Parser::T_IDENTIFIER)) {
            int kind = lex.findKeyword(data.constData() + tk.position, tk.length);
            if (kind == GLSL::Parser::T_IDENTIFIER) {
                if (m_keywords.contains(QByteArrayView(data.constData() + tk.position, tk.length)))
                    setFormat(tk.position, tk.length, formatForCategory(C_PREPROCESSOR));
                if (m_argumentKeywords.contains(QByteArrayView(data.constData() + tk.position, tk.length)))
                    setFormat(tk.position, tk.length, formatForCategory(C_PARAMETER));
            }
            if (kind == GLSL::Parser::T_RESERVED)
                setFormat(tk.position, tk.length, formatForCategory(GLSLReservedKeyword));
            else if (kind != GLSL::Parser::T_IDENTIFIER)
                setFormat(tk.position, tk.length, formatForCategory(C_KEYWORD));
        }
    }

    // mark the trailing white spaces
    {
        const GLSL::Token tk = tokens.last();
        const int lastTokenEnd = tk.begin() + tk.length;
        if (text.size() > lastTokenEnd)
            highlightLine(text, lastTokenEnd, text.size() - lastTokenEnd, QTextCharFormat());
    }
}

void SyntaxHighlighter::highlightLine(const QString &text, int position, int length,
                                      const QTextCharFormat &format)
{
    const QTextCharFormat visualSpaceFormat = formatForCategory(C_VISUAL_WHITESPACE);

    const int end = position + length;
    int index = position;

    while (index != end) {
        const bool isSpace = text.at(index).isSpace();
        const int start = index;

        do { ++index; }
        while (index != end && text.at(index).isSpace() == isSpace);

        const int tokenLength = index - start;
        if (isSpace)
            setFormat(start, tokenLength, visualSpaceFormat);
        else if (format.isValid())
            setFormat(start, tokenLength, format);
    }
}

QQuickTextDocument *SyntaxHighlighter::document() const
{
    return m_quickTextDocument;
}

void SyntaxHighlighter::setDocument(QQuickTextDocument *newDocument)
{
    if (m_quickTextDocument == newDocument)
        return;

    m_quickTextDocument = newDocument;
    QSyntaxHighlighter::setDocument(m_quickTextDocument != nullptr ? m_quickTextDocument->textDocument() : nullptr);

    emit documentChanged();
}

QT_END_NAMESPACE
