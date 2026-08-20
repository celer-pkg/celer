// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef __MOC__
#define __MOC__

#include <QString>

QT_FORWARD_DECLARE_CLASS(QMetaEnum);
QT_FORWARD_DECLARE_CLASS(QTextStream);

QT_BEGIN_NAMESPACE

QByteArray setterName(const QByteArray &propertyName);

void formatCppEnum(QTextStream &str, const QMetaEnum &metaEnum);

QString mocCode(const QMetaObject *, const QString &qualifiedClassName,
                QString *errorString);

QT_END_NAMESPACE

#endif //  __MOC__
