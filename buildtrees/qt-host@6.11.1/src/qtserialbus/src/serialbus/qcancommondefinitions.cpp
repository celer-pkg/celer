// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcancommondefinitions.h"

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/QDebug>
#endif // QT_NO_DEBUG_STREAM


QT_BEGIN_NAMESPACE

/*!
    \namespace QtCanBus
    \inmodule QtSerialBus
    \since 6.5
    \brief The QtCanBus namespace provides some commons enums that are used in
    the CAN bus handling part of the QtSerialPort module.
*/

/*!
    \enum QtCanBus::DataSource

    This enum represents the placement of the data within the CAN frame.

    \value Payload The data will be extracted from the payload.
    \value FrameId The data will be extracted from the frame ID.
*/

/*!
    \enum QtCanBus::DataFormat

    This enum represents the possible data formats. The format defines how the
    value will be extracted from its source.

    \value SignedInteger The signal value is a signed integer.
    \value UnsignedInteger The signal value is an unsigned integer.
    \value Float The signal value is float.
    \value Double The signal value is double.
    \value AsciiString The signal value is an ASCII string.
*/

/*!
    \enum QtCanBus::MultiplexState

    This enum represents the possible multiplex states of a signal.

    \value None The signal is not used in multiplexing.
    \value MultiplexorSwitch The signal is used as a multiplexor switch, which
                             means that other signals depend on the values of
                             this signal.
    \value MultiplexedSignal The signal is multiplexed by some switch, and
                             therefore its value can only be extracted when the
                             switch has a specific value.
    \value SwitchAndSignal The multiplexor switch of the signal must have the
                           value that enables us to use this signal. When used,
                           the signal also acts as a multiplexor switch for
                           other multiplexed signals.
*/

/*!
    \enum QtCanBus::UniqueId

    Represents a CAN unique identifier. The underlying type is quint32.

    An enum is used to avoid implicit conversions to or from unsigned int.
*/

#ifndef QT_NO_DEBUG_STREAM
QDebug QtCanBus::operator<<(QDebug dbg, DataSource source)
{
    QDebugStateSaver saver(dbg);
    switch (source) {
    case DataSource::Payload:
        dbg << "Payload";
        break;
    case DataSource::FrameId:
        dbg << "FrameId";
        break;
    }
    return dbg;
}

QDebug QtCanBus::operator<<(QDebug dbg, DataFormat format)
{
    QDebugStateSaver saver(dbg);
    switch (format) {
    case DataFormat::UnsignedInteger:
        dbg << "UnsignedInteger";
        break;
    case DataFormat::SignedInteger:
        dbg << "SignedInteger";
        break;
    case DataFormat::Float:
        dbg << "Float";
        break;
    case DataFormat::Double:
        dbg << "Double";
        break;
    case DataFormat::AsciiString:
        dbg << "ASCII";
        break;
    }
    return dbg;
}

QDebug QtCanBus::operator<<(QDebug dbg, MultiplexState state)
{
    QDebugStateSaver saver(dbg);
    switch (state) {
    case MultiplexState::None:
        dbg << "None";
        break;
    case MultiplexState::MultiplexorSwitch:
        dbg << "MultiplexorSwitch";
        break;
    case MultiplexState::MultiplexedSignal:
        dbg << "MultiplexedSignal";
        break;
    case MultiplexState::SwitchAndSignal:
        dbg << "SwitchAndSignal";
        break;
    }
    return dbg;
}

QDebug QtCanBus::operator<<(QDebug dbg, UniqueId uid)
{
    dbg << qToUnderlying(uid);
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE
