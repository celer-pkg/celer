// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpcsc_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace QPcsc {

QString errorMessage(LONG error)
{
#ifdef Q_OS_WIN
    return (u"0x%1"_s).arg(error, 8, 16, QLatin1Char('0'));
#else
    return QString::fromUtf8(pcsc_stringify_error(error));
#endif
}

} // namespace QPcsc

qsizetype QPcscSlotName::nameSize(QPcscSlotName::CPtr p)
{
#ifdef Q_OS_WIN
    return wcslen(p);
#else
    return strlen(p);
#endif
}

QT_END_NAMESPACE
