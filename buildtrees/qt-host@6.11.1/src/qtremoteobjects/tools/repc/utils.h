// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>

QT_BEGIN_NAMESPACE
class QJsonValue;
class QJsonArray;
struct AST;

QByteArray generateClass(const QJsonValue &cls, bool alwaysGenerateClass = false);
AST classList2AST(const QJsonArray &classes);
QT_END_NAMESPACE

#endif // UTILS_H
