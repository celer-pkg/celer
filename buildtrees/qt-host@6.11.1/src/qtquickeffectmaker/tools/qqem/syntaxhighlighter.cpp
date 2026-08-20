// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "syntaxhighlighter.h"
#include "syntaxhighlighterdata.h"

#include <QtCore/qregularexpression.h>

#ifdef QQEM_SYNTAX_HIGHLIGHTING_ENABLED
#include <QtQuick3DGlslParser/private/glsllexer_p.h>
#include <QtQuick3DGlslParser/private/glslparser_p.h>
#endif

// Text color and style categories
enum TextStyle : quint8 {
    C_VISUAL_WHITESPACE,
    C_PREPROCESSOR,
    C_NUMBER,
    C_COMMENT,
    C_KEYWORD,
    C_PARAMETER,
    C_REMOVED_LINE,
    C_TAG
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
        fmt.setForeground(QColor::fromRgb(0x80, 0x80, 0x80));
        return fmt;
     }
    case C_KEYWORD:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0xff, 0xff, 0x55));
        return fmt;
    }
    case C_TAG:
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor::fromRgb(0xaa, 0xff, 0xaa));
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

#ifdef QQEM_SYNTAX_HIGHLIGHTING_ENABLED
void SyntaxHighlighter::highlightBlock(const QString &text)
{
    if (m_tagKeywords.isEmpty()) {
        const auto tagKeywords = SyntaxHighlighterData::reservedTagNames();
        for (const auto &kw : tagKeywords)
            m_tagKeywords.insert(kw);
    }
    if (m_argumentKeywords.isEmpty()) {
        const auto args = SyntaxHighlighterData::reservedArgumentNames();
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
            setFormat(0, text.length(), formatForCategory(C_VISUAL_WHITESPACE));
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

        } else if (tk.is(GLSL::Parser::T_IDENTIFIER)) {
            int kind = lex.findKeyword(data.constData() + tk.position, tk.length);
            if (kind == GLSL::Parser::T_IDENTIFIER) {
                if (m_argumentKeywords.contains(QByteArrayView(data.constData() + tk.position, tk.length)))
                    setFormat(tk.position, tk.length, formatForCategory(C_PARAMETER));
            }
            if (kind == GLSL::Parser::T_RESERVED) {
                setFormat(tk.position, tk.length, formatForCategory(GLSLReservedKeyword));
            } else if (kind != GLSL::Parser::T_IDENTIFIER) {
                setFormat(tk.position, tk.length, formatForCategory(C_KEYWORD));
            } else {
                const auto tagStartPos = tk.position - 1;
                bool isTag = data.at(tagStartPos) == '@';
                if (isTag) {
                    const auto tagLength = tk.length + 1;
                    auto line = QByteArrayView(data.constData() + tagStartPos, tagLength).trimmed();
                    auto firstSpace = line.indexOf(' ');
                    QByteArrayView firstWord;
                    if (firstSpace > 0)
                        firstWord = line.sliced(0, firstSpace);
                    if (m_tagKeywords.contains(line) || (!firstWord.isEmpty() && m_tagKeywords.contains(firstWord)))
                        setFormat(tagStartPos, tagLength, formatForCategory(C_TAG));
                }
            }
        } else if (tk.is(GLSL::Parser::T_COMMENT)) {
            highlightLine(text, tk.begin(), tk.length, formatForCategory(C_COMMENT));
        }
    }

    // mark the trailing white spaces
    {
        const GLSL::Token tk = tokens.last();
        const int lastTokenEnd = tk.begin() + tk.length;
        if (text.length() > lastTokenEnd)
            highlightLine(text, lastTokenEnd, text.length() - lastTokenEnd, QTextCharFormat());
    }

    // Multi-line comments
    static QRegularExpression startExpression("/\\*");
    static QRegularExpression endExpression("\\*/");

    setCurrentBlockState(None);

    int startIndex = 0;
    if (previousBlockState() != Comment)
        startIndex = text.indexOf(startExpression);

    // Check multi-line comment blocks
    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(endExpression, startIndex, &endMatch);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(Comment);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, formatForCategory(C_COMMENT));
        startIndex = text.indexOf(startExpression,
                                  startIndex + commentLength);
    }
}

#else

// Dummy version without glslparser dependency
void SyntaxHighlighter::highlightBlock(const QString &text)
{
    Q_UNUSED(text)
}

#endif

void SyntaxHighlighter::highlightLine(const QString &text, int position, int length,
                                      const QTextCharFormat &format)
{
    const QTextCharFormat visualSpaceFormat = formatForCategory(C_VISUAL_WHITESPACE);

    const int end = position + length;
    int index = position;

    while (index != end) {
        const bool isSpace = text.at(index).isSpace();
        const int start = index;

        do {
            ++index;
        } while (index != end && text.at(index).isSpace() == isSpace);

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
