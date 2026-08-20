// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPCSC_P_H
#define QPCSC_P_H

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

#ifdef Q_OS_WIN
#    include <qt_windows.h>
#endif
#include <QtCore/QtGlobal>
#ifdef Q_OS_DARWIN
#    include <PCSC/winscard.h>
#    include <PCSC/wintypes.h>
#else
#    include <winscard.h>
#endif
#include <QtCore/QByteArray>
#include <QtCore/qhashfunctions.h>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace QPcsc {
struct RawCommandResult
{
    LONG ret = SCARD_E_READER_UNAVAILABLE;
    QByteArray response;

    bool isOk() const { return ret == SCARD_S_SUCCESS; }
};

QString errorMessage(LONG error);

} // namespace QPcsc

using QPcscSlotNameBase =
#ifdef Q_OS_WIN
                      QString
#else
                      QByteArray
#endif
                      ;

class QPcscSlotName : public QPcscSlotNameBase
{
public:
#ifdef Q_OS_WIN
    using CPtr = LPCWSTR;
    using Ptr = LPWSTR;
    explicit QPcscSlotName(CPtr p) : QString(reinterpret_cast<const QChar *>(p)) { }
#else
    using CPtr = LPCSTR;
    using Ptr = LPSTR;
    explicit QPcscSlotName(CPtr p) : QByteArray(p) { }
#endif

    CPtr ptr() const noexcept { return reinterpret_cast<CPtr>(constData()); }
    Ptr ptr() { return reinterpret_cast<Ptr>(data()); }

    static qsizetype nameSize(CPtr p);

    friend size_t qHash(const QPcscSlotName &key, size_t seed = 0) noexcept
    {
        const QPcscSlotNameBase &base = key;
        return qHash(base, seed);
    }
};

QT_END_NAMESPACE

#endif // QPCSC_P_H
