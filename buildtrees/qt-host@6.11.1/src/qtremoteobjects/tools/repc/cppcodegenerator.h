// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPPCODEGENERATOR_H
#define CPPCODEGENERATOR_H

QT_BEGIN_NAMESPACE
class QJsonArray;
class QIODevice;

class CppCodeGenerator
{
public:
    CppCodeGenerator(QIODevice *outputDevice);

    void generate(const QJsonArray &classList, bool alwaysGenerateClass = false);

private:
    QIODevice *m_outputDevice;
};

QT_END_NAMESPACE

#endif // CPPCODEGENERATOR_H
