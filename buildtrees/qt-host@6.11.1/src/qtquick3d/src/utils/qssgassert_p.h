// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGASSERT_P_H
#define QSSGASSERT_P_H

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

#include <QtCore/qglobal.h>
#include <QtQuick3DUtils/qtquick3dutilsexports.h>

QT_BEGIN_NAMESPACE

Q_QUICK3DUTILS_EXPORT void qssgWriteAssertLocation(const char *msg);

QT_END_NAMESPACE

#ifdef QT_DEBUG
#define QSSG_DEBUG_COND(cond) (cond)
#else
#define QSSG_DEBUG_COND(cond) (true || (cond))
#endif // QT_DEBUG

#define QSSG_ASSERT_STRINGIFY_HELPER(x) #x
#define QSSG_ASSERT_STRINGIFY(x) QSSG_ASSERT_STRINGIFY_HELPER(x)
#define QSSG_ASSERT_STRING_X(msg) QT_PREPEND_NAMESPACE(qssgWriteAssertLocation)(msg)
#define QSSG_ASSERT_STRING(cond) QSSG_ASSERT_STRING_X(\
    "\"" cond"\" in file " __FILE__ ", line " QSSG_ASSERT_STRINGIFY(__LINE__))

// The 'do {...} while (0)' idiom is not used for the main block here to be
// able to use 'break' and 'continue' as 'actions'.

#define QSSG_ASSERT(cond, action) if (Q_LIKELY(cond)) {} else { QSSG_ASSERT_STRING(#cond); action; } do {} while (0)
#define QSSG_ASSERT_X(cond, msg, action) if (Q_LIKELY(cond)) {} else { QSSG_ASSERT_STRING_X(msg); action; } do {} while (0)
#define QSSG_CHECK(cond) if (Q_LIKELY(cond)) {} else { QSSG_ASSERT_STRING(#cond); } do {} while (0)
#define QSSG_CHECK_X(cond, msg)  if (Q_LIKELY(cond)) {} else { QSSG_ASSERT_STRING_X(msg); } do {} while (0)
#define QSSG_GUARD(cond) ((Q_LIKELY(cond)) ? true : (QSSG_ASSERT_STRING(#cond), false))
#define QSSG_GUARD_X(cond, msg) ((Q_LIKELY(cond)) ? true : (QSSG_ASSERT_STRING_X(msg), false))

#endif // QSSGASSERT_P_H
