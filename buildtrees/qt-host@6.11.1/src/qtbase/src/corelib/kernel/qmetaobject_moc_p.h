// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#if !defined(QMETAOBJECT_P_H) && !defined(UTILS_H)
# error "Include qmetaobject_p.h (or moc's utils.h) before including this file."
#endif

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

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

static void normalizeTypeInternal(QByteArrayView in, QByteArray &appendTo);

// This function is shared with moc.cpp. This file should be included where needed.
static QByteArray normalizeTypeInternal(const char *t, const char *e)
{
    QByteArrayView in{t, e};
    QByteArray appendTo;
    normalizeTypeInternal(in, appendTo);
    return appendTo;
}

static void normalizeTypeInternal(QByteArrayView in, QByteArray &appendTo)
{
    const char *t = in.begin();
    const char *e = in.end();
    int len = QtPrivate::qNormalizeType(t, e, nullptr);
    if (len == 0)
        return;
    qsizetype oldLen = appendTo.size();
    appendTo.resize(appendTo.size() + len);
    QtPrivate::qNormalizeType(t, e, appendTo.begin() + oldLen);
}

QT_END_NAMESPACE
