// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qwindowscodec_p.h"
#include "private/qstringconverter_p.h"

QT_BEGIN_NAMESPACE

QWindowsLocalCodec::QWindowsLocalCodec()
{
}

QWindowsLocalCodec::~QWindowsLocalCodec()
{
}

QString QWindowsLocalCodec::convertToUnicode(const char *chars, int length, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QLocal8Bit::convertToUnicode(QByteArrayView(chars, length), state);
}

QByteArray QWindowsLocalCodec::convertFromUnicode(const QChar *ch, int uclen, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QLocal8Bit::convertFromUnicode(QStringView(ch, uclen), state);
}

QByteArray QWindowsLocalCodec::name() const
{
    return "System";
}

int QWindowsLocalCodec::mibEnum() const
{
    return 0;
}

QT_END_NAMESPACE
