// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qiodevice.h>
#include <qjsonarray.h>
#include <qjsonvalue.h>

#include "cppcodegenerator.h"
#include "utils.h"

QT_BEGIN_NAMESPACE

CppCodeGenerator::CppCodeGenerator(QIODevice *outputDevice)
    : m_outputDevice(outputDevice)
{
    Q_ASSERT(m_outputDevice);
}

void CppCodeGenerator::generate(const QJsonArray &classList, bool alwaysGenerateClass /* = false */)
{
    for (const QJsonValue cdef : classList)
        m_outputDevice->write(generateClass(cdef, alwaysGenerateClass));

    m_outputDevice->write("\n");
}

QT_END_NAMESPACE
