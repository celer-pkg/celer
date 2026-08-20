// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCANUNIQUEIDDESCRIPTION_HELPERS_H
#define QCANUNIQUEIDDESCRIPTION_HELPERS_H

#include <QtSerialBus/qcanuniqueiddescription.h>

QT_BEGIN_NAMESPACE

inline bool equals(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs) noexcept
{
    return lhs.source() == rhs.source()
            && lhs.startBit() == rhs.startBit()
            && lhs.bitLength() == rhs.bitLength()
            && lhs.endian() == rhs.endian();
}

QT_END_NAMESPACE

#endif // QCANUNIQUEIDDESCRIPTION_HELPERS_H
