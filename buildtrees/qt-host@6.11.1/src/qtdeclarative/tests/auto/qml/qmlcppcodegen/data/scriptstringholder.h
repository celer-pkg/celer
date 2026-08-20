// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCRIPTSTRINGHOLDER_H
#define SCRIPTSTRINGHOLDER_H

#include <QQmlEngine>
#include <QQmlScriptString>

#include <qqmlregistration.h>

class ScriptStringHolder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQmlScriptString ss READ ss WRITE setSs)

public:
    QQmlScriptString ss() const { return m_ss; }
    void setSs(QQmlScriptString ss) { m_ss = ss; }

private:
    QQmlScriptString m_ss;
};

#endif // SCRIPTSTRINGHOLDER_H
