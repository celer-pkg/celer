// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qutfcodec_p.h"
#include "qlist.h"
#include "qendian.h"
#include "qchar.h"

#include "private/qsimd_p.h"
#include "private/qstringiterator_p.h"

QT_BEGIN_NAMESPACE

#if QT_CONFIG(textcodec)

QUtf8Codec::~QUtf8Codec()
{
}

QByteArray QUtf8Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf8::convertFromUnicode(QStringView(uc, len), state);
}

void QUtf8Codec::convertToUnicode(QString *target, const char *chars, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    *target += QUtf8::convertToUnicode(QByteArrayView(chars, len), state);
}

QString QUtf8Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf8::convertToUnicode(QByteArrayView(chars, len), state);
}

QByteArray QUtf8Codec::name() const
{
    return "UTF-8";
}

int QUtf8Codec::mibEnum() const
{
    return 106;
}

QUtf16Codec::~QUtf16Codec()
{
}

QByteArray QUtf16Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf16::convertFromUnicode(QStringView(uc, len), state, e);
}

QString QUtf16Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf16::convertToUnicode(QByteArrayView(chars, len), state, e);
}

int QUtf16Codec::mibEnum() const
{
    return 1015;
}

QByteArray QUtf16Codec::name() const
{
    return "UTF-16";
}

QList<QByteArray> QUtf16Codec::aliases() const
{
    return QList<QByteArray>();
}

int QUtf16BECodec::mibEnum() const
{
    return 1013;
}

QByteArray QUtf16BECodec::name() const
{
    return "UTF-16BE";
}

QList<QByteArray> QUtf16BECodec::aliases() const
{
    QList<QByteArray> list;
    return list;
}

int QUtf16LECodec::mibEnum() const
{
    return 1014;
}

QByteArray QUtf16LECodec::name() const
{
    return "UTF-16LE";
}

QList<QByteArray> QUtf16LECodec::aliases() const
{
    QList<QByteArray> list;
    return list;
}

QUtf32Codec::~QUtf32Codec()
{
}

QByteArray QUtf32Codec::convertFromUnicode(const QChar *uc, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf32::convertFromUnicode(QStringView(uc, len), state, e);
}

QString QUtf32Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    ConverterState s(QStringConverter::Flag::Stateless);
    if (!state)
        state = &s;
    return QUtf32::convertToUnicode(QByteArrayView(chars, len), state, e);
}

int QUtf32Codec::mibEnum() const
{
    return 1017;
}

QByteArray QUtf32Codec::name() const
{
    return "UTF-32";
}

QList<QByteArray> QUtf32Codec::aliases() const
{
    QList<QByteArray> list;
    return list;
}

int QUtf32BECodec::mibEnum() const
{
    return 1018;
}

QByteArray QUtf32BECodec::name() const
{
    return "UTF-32BE";
}

QList<QByteArray> QUtf32BECodec::aliases() const
{
    QList<QByteArray> list;
    return list;
}

int QUtf32LECodec::mibEnum() const
{
    return 1019;
}

QByteArray QUtf32LECodec::name() const
{
    return "UTF-32LE";
}

QList<QByteArray> QUtf32LECodec::aliases() const
{
    QList<QByteArray> list;
    return list;
}

#endif // textcodec

QT_END_NAMESPACE
