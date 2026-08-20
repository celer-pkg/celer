// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QCANMESSAGEDESCRIPTION_HELPERS_H
#define QCANMESSAGEDESCRIPTION_HELPERS_H

#include <QtSerialBus/qcanmessagedescription.h>

#include "qcansignaldescription_helpers.h"

QT_BEGIN_NAMESPACE

inline bool equals(const QCanMessageDescription &lhs, const QCanMessageDescription &rhs) noexcept
{
    return lhs.uniqueId() == rhs.uniqueId()
            && lhs.name() == rhs.name()
            && lhs.size() == rhs.size()
            && lhs.transmitter() == rhs.transmitter()
            && lhs.comment() == rhs.comment()
            && equals(lhs.signalDescriptions(), rhs.signalDescriptions());
}

inline bool equals(const QList<QCanMessageDescription> &lhs,
                   const QList<QCanMessageDescription> &rhs) noexcept
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

#endif // QCANMESSAGEDESCRIPTION_HELPERS_H
