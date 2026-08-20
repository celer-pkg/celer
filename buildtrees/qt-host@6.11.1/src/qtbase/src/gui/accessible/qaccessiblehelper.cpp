// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qaccessiblehelper_p.h"

#include <QtGui/qtextcursor.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/* This function will return the offset of the '&' in the text that would be
   preceding the accelerator character.
   If this text does not have an accelerator, -1 will be returned. */
static qsizetype qt_accAmpIndex(const QString &text)
{
#ifndef QT_NO_SHORTCUT
    if (text.isEmpty())
        return -1;

    qsizetype fa = 0;
    while ((fa = text.indexOf(u'&', fa)) != -1) {
        ++fa;
        if (fa < text.size()) {
            // ignore "&&"
            if (text.at(fa) == u'&') {
                ++fa;
                continue;
            } else {
                return fa - 1;
                break;
            }
        }
    }

    return -1;
#else
    Q_UNUSED(text);
    return -1;
#endif
}

QString qt_accStripAmp(const QString &text)
{
    QString newText(text);
    qsizetype ampIndex = qt_accAmpIndex(newText);
    if (ampIndex != -1)
        newText.remove(ampIndex, 1);

    return newText.replace("&&"_L1, "&"_L1);
}

QString qt_accTextBeforeOffsetHelper(const QAccessibleTextInterface &textInterface,
                                     const QTextCursor &textCursor, int offset,
                                     QAccessible::TextBoundaryType boundaryType, int *startOffset,
                                     int *endOffset)
{
    Q_ASSERT(startOffset);
    Q_ASSERT(endOffset);
    *startOffset = *endOffset = -1;

    QTextCursor cursor = textCursor;
    cursor.setPosition(offset);
    std::pair<int, int> boundaries =
            QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
    if (boundaries.second > offset) {
        cursor.setPosition(boundaries.first);
        while (boundaries.second > offset) {
            if (!cursor.movePosition(QTextCursor::PreviousCharacter))
                return QString();
            boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
        }
    }

    *startOffset = boundaries.first;
    *endOffset = boundaries.second;

    return textInterface.text(boundaries.first, boundaries.second);
}

QString qt_accTextAfterOffsetHelper(const QAccessibleTextInterface &textInterface,
                                    const QTextCursor &textCursor, int offset,
                                    QAccessible::TextBoundaryType boundaryType, int *startOffset,
                                    int *endOffset)
{
    Q_ASSERT(startOffset);
    Q_ASSERT(endOffset);
    *startOffset = *endOffset = -1;

    QTextCursor cursor = textCursor;
    cursor.setPosition(offset);
    std::pair<int, int> boundaries =
            QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
    if (boundaries.first <= offset) {
        cursor.setPosition(boundaries.second);
        while (boundaries.first <= offset) {
            if (!cursor.movePosition(QTextCursor::NextCharacter))
                return QString();
            boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
        }
    }

    *startOffset = boundaries.first;
    *endOffset = boundaries.second;

    return textInterface.text(boundaries.first, boundaries.second);
}

QString qt_accTextAtOffsetHelper(const QAccessibleTextInterface &textInterface,
                                 const QTextCursor &textCursor, int offset,
                                 QAccessible::TextBoundaryType boundaryType, int *startOffset,
                                 int *endOffset)
{
    Q_ASSERT(startOffset);
    Q_ASSERT(endOffset);

    QTextCursor cursor = textCursor;
    cursor.setPosition(offset);
    std::pair<int, int> boundaries =
            QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);

    *startOffset = boundaries.first;
    *endOffset = boundaries.second;

    return textInterface.text(boundaries.first, boundaries.second);
}

QT_END_NAMESPACE
