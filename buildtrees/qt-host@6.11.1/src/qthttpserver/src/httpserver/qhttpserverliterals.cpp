// Copyright (C) 2019 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qhttpserverliterals_p.h"

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

// Don't use QByteArrayLiteral, as this library may be unloaded before all
// references to the data are destroyed - by allocating on the heap, the last
// user will free the data instead of referencing unloaded data

QByteArray QHttpServerLiterals::contentTypeXEmpty()
{
    static QByteArray ba("application/x-empty");
    return ba;
}

QByteArray QHttpServerLiterals::contentTypeTextHtml()
{
    static QByteArray ba("text/html");
    return ba;
}

QByteArray QHttpServerLiterals::contentTypeJson()
{
    static QByteArray ba("application/json");
    return ba;
}

QT_END_NAMESPACE
