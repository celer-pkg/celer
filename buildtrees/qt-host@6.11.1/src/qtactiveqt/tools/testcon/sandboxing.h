// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SANDBOXING_H
#define SANDBOXING_H
#include <QString>
#include <QAxSelect>


class Sandboxing
{
public:
    static std::unique_ptr<Sandboxing> Create(QAxSelect::SandboxingLevel level, const QString &clsid);

    Sandboxing() {}

    virtual ~Sandboxing() {}
};

#endif // SANDBOXING_H
