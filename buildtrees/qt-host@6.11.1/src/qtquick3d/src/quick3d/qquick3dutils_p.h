// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DUTILS_P_H
#define QQUICK3DUTILS_P_H

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

#include <type_traits>

#include <QtCore/QtGlobal>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

template <typename T, typename = void>
struct hasFuzzyCompare : std::false_type {};

template <typename T>
struct hasFuzzyCompare<T, std::void_t<decltype(qFuzzyCompare(std::declval<T &>(), std::declval<T &>()))>> : std::true_type {};

// Assigns 'updated' to 'orig' and returns true if they are different
template <typename T, std::enable_if_t<!hasFuzzyCompare<T>::value, bool> = true>
bool qUpdateIfNeeded(T &orig, T updated)
{
    if (orig == updated)
        return false;
    orig = updated;
    return true;
}

// Assigns 'updated' to 'orig' and returns true if they are different, compared with qFuzzyCompare
template <typename T, std::enable_if_t<hasFuzzyCompare<T>::value, bool> = true>
bool qUpdateIfNeeded(T &orig, T updated)
{
    if (qFuzzyCompare(orig, updated))
        return false;
    orig = updated;
    return true;
}

QT_END_NAMESPACE

#endif // QQUICK3DUTILS_P_H
