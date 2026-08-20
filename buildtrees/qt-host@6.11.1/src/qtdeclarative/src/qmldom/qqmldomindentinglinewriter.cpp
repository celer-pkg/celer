// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qqmldomindentinglinewriter_p.h"
#include "qqmldomlinewriter_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE
namespace QQmlJS {
namespace Dom {

// If one of these tokens is the split token on a line, the line should be split after it.
static constexpr bool shouldSplitAfterToken(int kind)
{
    switch (kind) {
    case Lexer::T_COMMA:
    case Lexer::T_COLON:
    case Lexer::T_SEMICOLON:
    case Lexer::T_LBRACE:
    case Lexer::T_LPAREN:
    case Lexer::T_LBRACKET:
        return true;
    default:
        break;
    }
    return false;
}

FormatPartialStatus &IndentingLineWriter::fStatus()
{
    if (!m_fStatusValid) {
        m_fStatus = formatCodeLine(m_currentLine, m_options.formatOptions, m_preCachedStatus);
        m_fStatusValid = true;
    }
    return m_fStatus;
}

void IndentingLineWriter::willCommit()
{
    m_preCachedStatus = fStatus().currentStatus;
}

void IndentingLineWriter::reindentAndSplit(const QString &eol, bool eof)
{
    // maybe re-indent
    if (m_reindent && m_columnNr == 0)
        setLineIndent(fStatus().indentLine());

    if (!eol.isEmpty() || eof)
        handleTrailingSpace();

    // maybe split long line
    if (m_options.maxLineLength > 0 && m_currentLine.size() > m_options.maxLineLength)
        splitOnMaxLength(eol, eof);

    // maybe write out
    if (!eol.isEmpty() || eof)
        commitLine(eol);
}

void IndentingLineWriter::handleTrailingSpace()
{
    LineWriterOptions::TrailingSpace trailingSpace;
    if (!m_currentLine.isEmpty() && m_currentLine.trimmed().isEmpty()) {
        // space only line
        const Scanner::State &oldState = m_preCachedStatus.lexerState;
        if (oldState.isMultilineComment())
            trailingSpace = m_options.commentTrailingSpace;
        else if (oldState.isMultiline())
            trailingSpace = m_options.stringTrailingSpace;
        else
            trailingSpace = m_options.codeTrailingSpace;
        // in the LSP we will probably want to treat is specially if it is the line with the
        // cursor,  of if indentation of it is requested
    } else {
        const Scanner::State &currentState = fStatus().currentStatus.lexerState;
        if (currentState.isMultilineComment()) {
            trailingSpace = m_options.commentTrailingSpace;
        } else if (currentState.isMultiline()) {
            trailingSpace = m_options.stringTrailingSpace;
        } else {
            const int kind =
                    (fStatus().lineTokens.isEmpty() ? Lexer::T_EOL
                                                    : fStatus().lineTokens.last().lexKind);
            if (Token::lexKindIsComment(kind)) {
                // a // comment...
                trailingSpace = m_options.commentTrailingSpace;
                Q_ASSERT(fStatus().currentStatus.state().type
                                    != FormatTextStatus::StateType::MultilineCommentCont
                            && fStatus().currentStatus.state().type
                                    != FormatTextStatus::StateType::
                                            MultilineCommentStart); // these should have been
                                                                    // handled above
            } else {
                trailingSpace = m_options.codeTrailingSpace;
            }
        }
    }
    LineWriter::handleTrailingSpace(trailingSpace);
}

int IndentingLineWriter::findSplitLocation(const QList<Token> &tokens, int minSplitLength)
{
    // Reverse search to find the "last"
    auto lastDelimiterTokenBeforeLineSplitIt =
            std::find_if(tokens.crbegin(), tokens.crend(), [&](auto &&t) {
                return Token::lexKindIsDelimiter(t.lexKind)
                        && column(t.end()) >= minSplitLength;
            });

    if (lastDelimiterTokenBeforeLineSplitIt == tokens.crend())
        return -1;

    if (shouldSplitAfterToken(lastDelimiterTokenBeforeLineSplitIt->lexKind)) {
        return lastDelimiterTokenBeforeLineSplitIt->end();
    } else {
        // split before the token (after the previous token)
        auto previousTokenIt = std::next(lastDelimiterTokenBeforeLineSplitIt);
        return previousTokenIt != tokens.crend() ? previousTokenIt->end()
                                                 : lastDelimiterTokenBeforeLineSplitIt->begin();
    }
}

void IndentingLineWriter::splitOnMaxLength(const QString &eol, bool eof)
{
    if (fStatus().lineTokens.size() <= 1)
        return;
    // {}[] should already be handled (handle also here?)
    int minLen = 0;
    while (minLen < m_currentLine.size() && m_currentLine.at(minLen).isSpace())
        ++minLen;
    minLen = column(minLen) + m_options.minContentLength;
    int possibleSplit = findSplitLocation(fStatus().lineTokens, minLen);
    if (possibleSplit > 0) {
        lineChanged();
        commitLine(eolToWrite(), TextAddType::NewlineSplit, possibleSplit);
        setReindent(true);
        reindentAndSplit(eol, eof);
    }
}

} // namespace Dom
} // namespace QQmlJS
QT_END_NAMESPACE
