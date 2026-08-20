// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SYNTAXHIGHLIGHTERDATA_H
#define SYNTAXHIGHLIGHTERDATA_H

#include <QByteArrayView>
#include <QList>

class SyntaxHighlighterData
{
public:
    SyntaxHighlighterData();

    static QList<QByteArrayView> reservedArgumentNames();
    static QList<QByteArrayView> reservedTagNames();
    static QList<QByteArrayView> reservedFunctionNames();
};

#endif // SYNTAXHIGHLIGHTERDATA_H
