// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCANSIGNALDESCRIPTION_HELPERS_H
#define QCANSIGNALDESCRIPTION_HELPERS_H

#include <QtCore/qnumeric.h>

#include <QtSerialBus/qcansignaldescription.h>

QT_BEGIN_NAMESPACE

// copied from qtbase/src/testlib/qtestcase.cpp
template <typename T>
bool floatingCompare(const T &actual, const T &expected)
{
    switch (qFpClassify(expected))
    {
    case FP_INFINITE:
        return (expected < 0) == (actual < 0) && qFpClassify(actual) == FP_INFINITE;
    case FP_NAN:
        return qFpClassify(actual) == FP_NAN;
    default:
        if (!qFuzzyIsNull(expected))
            return qFuzzyCompare(actual, expected);
        Q_FALLTHROUGH();
    case FP_SUBNORMAL: // subnormal is always fuzzily null
    case FP_ZERO:
        return qFuzzyIsNull(actual);
    }
}

inline bool equals(const QCanSignalDescription &lhs, const QCanSignalDescription &rhs) noexcept
{
    return lhs.name() == rhs.name()
            && lhs.physicalUnit() == rhs.physicalUnit()
            && lhs.receiver() == rhs.receiver()
            && lhs.comment() == rhs.comment()
            && lhs.dataSource() == rhs.dataSource()
            && lhs.dataEndian() == rhs.dataEndian()
            && lhs.dataFormat() == rhs.dataFormat()
            && lhs.startBit() == rhs.startBit()
            && lhs.bitLength() == rhs.bitLength()
            && floatingCompare(lhs.factor(), rhs.factor())
            && floatingCompare(lhs.offset(), rhs.offset())
            && floatingCompare(lhs.scaling(), rhs.scaling())
            && floatingCompare(lhs.minimum(), rhs.minimum())
            && floatingCompare(lhs.maximum(), rhs.maximum())
            && lhs.multiplexState() == rhs.multiplexState()
            && lhs.multiplexSignals() == rhs.multiplexSignals();
}

inline bool equals(const QList<QCanSignalDescription> &lhs,
                   const QList<QCanSignalDescription> &rhs) noexcept
{
    if (lhs.size() != rhs.size())
        return false;

    for (qsizetype idx = 0; idx < lhs.size(); ++idx) {
        if (!equals(lhs.at(idx), rhs.at(idx)))
            return false;
    }

    return true;
}

QT_END_NAMESPACE

#endif // QCANSIGNALDESCRIPTION_HELPERS_H
