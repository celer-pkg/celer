// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANCOMMONDEFINITIONS_H
#define QCANCOMMONDEFINITIONS_H

#include <QtCore/qendian.h>
#include <QtCore/qsysinfo.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtypes.h>

#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

namespace QtCanBus {

enum class DataSource : quint8 {
    Payload = 0,
    FrameId,
};

enum class DataFormat : quint8 {
    SignedInteger = 0,
    UnsignedInteger,
    Float,
    Double,
    AsciiString,
};

enum class MultiplexState : quint8 {
    None = 0x00,
    MultiplexorSwitch = 0x01,
    MultiplexedSignal = 0x02,
    SwitchAndSignal = MultiplexorSwitch | MultiplexedSignal,
};

enum class UniqueId : quint32 {};

inline UniqueId qbswap(UniqueId src) noexcept
{
    return UniqueId{qbswap_helper(qToUnderlying(src))};
}

} // namespace QtCanBus


#ifndef QT_NO_DEBUG_STREAM

class QDebug;

namespace QtCanBus {
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, DataSource source);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, DataFormat format);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, MultiplexState state);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, UniqueId uid);
} // namespace QtCanBus

#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE

#endif // QCANCOMMONDEFINITIONS_H
