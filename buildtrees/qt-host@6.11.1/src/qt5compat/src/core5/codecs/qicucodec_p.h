// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QICUCODEC_P_H
#define QICUCODEC_P_H

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

extern "C" {
    typedef struct UConverter UConverter;
}

QT_REQUIRE_CONFIG(textcodec);

QT_BEGIN_NAMESPACE

class QIcuCodec : public QTextCodec
{
public:
    static QList<QByteArray> availableCodecs();
    static QList<int> availableMibs();

    static QTextCodec *defaultCodecUnlocked();

    static QTextCodec *codecForNameUnlocked(const char *name);
    static QTextCodec *codecForMibUnlocked(int mib);

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

private:
    QIcuCodec(const char *name);
    ~QIcuCodec();

    UConverter *getConverter(QTextCodec::ConverterState *state) const;

    const char *m_name;
};

QT_END_NAMESPACE

#endif
