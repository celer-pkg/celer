// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQMLDIFFER_P_H
#define QQMLDIFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// This code is adapted from QtCreator/src/libs/utils/differ.

#include <QHash>
#include <QString>

QT_BEGIN_NAMESPACE

namespace QQmlLSUtils {

class Diff
{
public:
    enum Command {
        Delete,
        Insert,
        Equal
    };
    Command command = Equal;
    QString text;
    Diff() = default;
    Diff(Command com, const QString &txt = {});
    bool operator==(const Diff &other) const;
    bool operator!=(const Diff &other) const;
};

class Differ
{
public:
    enum DiffMode
    {
        CharMode,
        WordMode,
        LineMode
    };
    Differ();
    QList<Diff> diff(const QString &text1, const QString &text2);
    QList<Diff> unifiedDiff(const QString &text1, const QString &text2);
    void setDiffMode(DiffMode mode);
    DiffMode diffMode() const;
    static QList<Diff> merge(const QList<Diff> &diffList);

private:
    QList<Diff> preprocess1AndDiff(const QString &text1, const QString &text2);
    QList<Diff> preprocess2AndDiff(const QString &text1, const QString &text2);
    QList<Diff> diffMyers(const QString &text1, const QString &text2);
    QList<Diff> diffMyersSplit(const QString &text1, qsizetype x,
                               const QString &text2, qsizetype y);
    QList<Diff> diffNonCharMode(const QString &text1, const QString &text2);
    QStringList encode(const QString &text1,
                       const QString &text2,
                       QString *encodedText1,
                       QString *encodedText2);
    QString encode(const QString &text,
                   QStringList *lines,
                   QHash<QString, qsizetype> *lineToCode);
    qsizetype findSubtextEnd(const QString &text,
                       qsizetype subTextStart);

    DiffMode m_diffMode = Differ::LineMode;
    DiffMode m_currentDiffMode = Differ::LineMode;
};

} // namespace QQmlLSUtils

QT_END_NAMESPACE

#endif // QQMLDIFFER_P_H
