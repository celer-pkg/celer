// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:header-decls-only

#ifndef QWINDOWSCODEC_P_H
#define QWINDOWSCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtextcodec_p.h>

QT_REQUIRE_CONFIG(textcodec);

QT_BEGIN_NAMESPACE

class QWindowsLocalCodec : public QTextCodec
{
public:
    QWindowsLocalCodec();
    ~QWindowsLocalCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

    QByteArray name() const override;
    int mibEnum() const override;

};

QT_END_NAMESPACE

#endif
