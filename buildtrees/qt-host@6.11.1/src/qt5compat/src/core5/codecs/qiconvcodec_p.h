// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QICONVCODEC_P_H
#define QICONVCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtextcodec_p.h>

QT_REQUIRE_CONFIG(iconv);

#include <iconv.h>

QT_BEGIN_NAMESPACE

class QIconvCodec: public QTextCodec
{
private:
    mutable QTextCodec *utf16Codec;

public:
    QIconvCodec();
    ~QIconvCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

    QByteArray name() const override;
    int mibEnum() const override;

    void init() const;
    iconv_t createIconv_t(const char *to, const char *from) const;

    class IconvState
    {
    public:
        IconvState(iconv_t x);
        ~IconvState();
        ConverterState internalState;
        char *buffer;
        int bufferLen;
        iconv_t cd;

        char array[8];

        void saveChars(const char *c, int count);
    };
};

QT_END_NAMESPACE

#endif // QICONVCODEC_P_H
