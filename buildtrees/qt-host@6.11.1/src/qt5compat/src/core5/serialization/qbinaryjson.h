// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:header-decls-only

#ifndef QBINARYJSON_H
#define QBINARYJSON_H

#include <QtCore5Compat/qcore5global.h>
#include <QtCore/qjsondocument.h>

#if 0
// This is needed for generating the QBinaryJson forward header
#pragma qt_class(QBinaryJson)
#endif

QT_BEGIN_NAMESPACE

namespace QBinaryJson {

enum DataValidation {
    Validate,
    BypassValidation
};

Q_CORE5COMPAT_EXPORT QJsonDocument fromRawData(const char *data, int size, DataValidation validation = Validate);
Q_CORE5COMPAT_EXPORT const char *toRawData(const QJsonDocument &document, int *size);

Q_CORE5COMPAT_EXPORT QJsonDocument fromBinaryData(const QByteArray &data, DataValidation validation = Validate);
Q_CORE5COMPAT_EXPORT QByteArray toBinaryData(const QJsonDocument &document);

} // QBinaryJson

QT_END_NAMESPACE

#endif // QBINARYJSON_H
