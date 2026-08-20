// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef LECMACCALCULATOR_H
#define LECMACCALCULATOR_H

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

#include <QtCore/private/qglobal_p.h>
#include <QtCore/quuid.h>

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT LeCmacCalculator
{
public:
    LeCmacCalculator();
    ~LeCmacCalculator();

    static QByteArray createFullMessage(const QByteArray &message, quint32 signCounter);

    quint64 calculateMac(const QByteArray &message, QUuid::Id128Bytes csrk) const;

    // Convenience function.
    bool verify(const QByteArray &message, QUuid::Id128Bytes csrk, quint64 expectedMac) const;

private:
    int m_baseSocket = -1;
};


QT_END_NAMESPACE

#endif // Header guard
