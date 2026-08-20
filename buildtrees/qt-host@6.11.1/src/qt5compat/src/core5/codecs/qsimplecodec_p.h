// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:header-decls-only

#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

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

#include <private/qtextcodec_p.h>

QT_REQUIRE_CONFIG(textcodec);

QT_BEGIN_NAMESPACE

template <typename T> class QAtomicPointer;

class QSimpleTextCodec: public QTextCodec
{
public:
    enum { numSimpleCodecs = 30 };
    explicit QSimpleTextCodec(int);
    ~QSimpleTextCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

private:
    int forwardIndex;
    mutable QAtomicPointer<QByteArray> reverseMap;
};

QT_END_NAMESPACE

#endif // QSIMPLECODEC_P_H
