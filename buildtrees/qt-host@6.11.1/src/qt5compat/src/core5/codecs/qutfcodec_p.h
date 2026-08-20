// Copyright (C) 2018 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only
// ### de-inline everything

#ifndef QUTFCODEC_P_H
#define QUTFCODEC_P_H

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


#include <QtCore/qendian.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

#include <private/qstringconverter_p.h>
#include <private/qtextcodec_p.h>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(textcodec)

class QUtf8Codec : public QTextCodec {
public:
    ~QUtf8Codec();

    QByteArray name() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;
    void convertToUnicode(QString *target, const char *, int, ConverterState *) const;
};

class QUtf16Codec : public QTextCodec {
protected:
public:
    QUtf16Codec() { e = DetectEndianness; }
    ~QUtf16Codec();

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

protected:
    DataEndianness e;
};

class QUtf16BECodec : public QUtf16Codec {
public:
    QUtf16BECodec() : QUtf16Codec() { e = BigEndianness; }
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf16LECodec : public QUtf16Codec {
public:
    QUtf16LECodec() : QUtf16Codec() { e = LittleEndianness; }
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf32Codec : public QTextCodec {
public:
    QUtf32Codec() { e = DetectEndianness; }
    ~QUtf32Codec();

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

protected:
    DataEndianness e;
};

class QUtf32BECodec : public QUtf32Codec {
public:
    QUtf32BECodec() : QUtf32Codec() { e = BigEndianness; }
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf32LECodec : public QUtf32Codec {
public:
    QUtf32LECodec() : QUtf32Codec() { e = LittleEndianness; }
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

#endif // textcodec

QT_END_NAMESPACE

#endif // QUTFCODEC_P_H
