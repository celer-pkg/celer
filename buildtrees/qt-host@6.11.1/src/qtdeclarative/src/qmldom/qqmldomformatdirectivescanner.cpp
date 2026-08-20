// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qqmldomformatdirectivescanner_p.h"

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE
namespace QmlFormat {

static constexpr auto directive = QLatin1StringView("qmlformat");
static constexpr auto onCmd = QLatin1StringView("on");
static constexpr auto offCmd = QLatin1StringView("off");
static constexpr auto commentStartString = QLatin1StringView("//");

// Newline will be detected from document content
static QLatin1StringView detectNewLine(QStringView code) {
    // Look for first newline in document
    qsizetype idx = code.indexOf(u'\n');
    if (idx > 0 && code[idx - 1] == u'\r')
        return QLatin1StringView("\r\n");

    return QLatin1StringView("\n");
}

static quint32 findLineStartOffset(QStringView m_code, qsizetype pos, QLatin1StringView newLine)
{
    const auto startOffset = m_code.lastIndexOf(newLine, pos);
    // If not found, return start of document
    if (startOffset == -1)
        return 0;
    return quint32(startOffset + newLine.length());
}

static quint32 findDirectiveEndOffset(QStringView m_code, qsizetype offset, QLatin1StringView newLine)
{
    auto endOffset = m_code.indexOf(onCmd, offset) + onCmd.size();
    if (endOffset < onCmd.size())
        endOffset = m_code.size();

    // Also contain up to two newlines following the directive
    int newlineCount = 0;
    while (endOffset + newLine.size() <= m_code.size() &&
           m_code.mid(endOffset, newLine.size()) == newLine && newlineCount < 2) {
        endOffset += newLine.size();
        ++newlineCount;
    }
    return quint32(endOffset);
}

static QQmlJS::SourceLocation formRegion(QStringView code, const QQmlJS::SourceLocation &off,
                                         const QQmlJS::SourceLocation &on, QLatin1StringView newline)
{
    const auto startOffset = findLineStartOffset(code, off.offset, newline);
    const auto endOffset = findDirectiveEndOffset(code, on.offset, newline);
    return QQmlJS::SourceLocation{ startOffset, endOffset - startOffset, 0, 0 };
}

static QStringView commentLineFromLoc(QStringView code, const QQmlJS::SourceLocation &loc, QLatin1StringView newline)
{
    const auto lineStartOffset = findLineStartOffset(code, loc.offset, newline);
    return code.mid(lineStartOffset, loc.offset - lineStartOffset + loc.length);
};

DisabledRegions identifyDisabledRegions(QStringView code, const QList<QQmlJS::SourceLocation> &comments)
{
    const auto newline = detectNewLine(code);
    const auto isOffDirective = [&code, newline](const QQmlJS::SourceLocation &loc) {
        const auto commentLineString = commentLineFromLoc(code, loc, newline);
        const auto directive = directiveFromComment(commentLineString);
        return directive && *directive == Directive::Off;
    };

    const auto isOnDirective = [&code, newline](const QQmlJS::SourceLocation &loc) {
        const auto commentLineString = commentLineFromLoc(code, loc, newline);
        const auto directive = directiveFromComment(commentLineString);
        return directive && *directive == Directive::On;
    };

    DisabledRegions result;
    auto it = comments.begin();
    const auto end = comments.end();

    while (it != end) {
        // 1. Find the next "formatter-off" directive
        it = std::find_if(it, end, isOffDirective);

        if (it == end) break;
        const auto& off = *it;

        // 2. Find the next "formatter-on" directive
        it = std::find_if(it, end, isOnDirective);

        // 3. form region
        if (it == end) {
            // No corresponding "on"
            auto &&on = QQmlJS::SourceLocation(code.size(), 0, 0, 0);
            result.insert(
                    off.offset,
                    formRegion(code, off, std::move(on), newline));
            break;
        }
        const auto& on = *it;
        result.insert(off.offset, formRegion(code, off, on, newline));
    }

    return result;
}



std::optional<Directive> directiveFromComment(QStringView commentLine)
{
    // Split into words (directive and command)
    // It should be on its own line,
    const auto words = commentLine.trimmed().split(u' ', Qt::SkipEmptyParts);
    if (words.size() != 3)
        return std::nullopt;

    // Validate directive
    if (words.at(0) != commentStartString || words.at(1) != directive)
        return std::nullopt;

    return (words.at(2) == onCmd)     ? std::make_optional(Directive::On)
            : (words.at(2) == offCmd) ? std::make_optional(Directive::Off)
                                      : std::nullopt;
}

} // namespace QmlFormat

QT_END_NAMESPACE
