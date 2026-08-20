// Copyright (C) 2019 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "libsocketcan.h"

#include <QtCore/qloggingcategory.h>

#if QT_CONFIG(library)
#   include <QtCore/qlibrary.h>
#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_SOCKETCAN)

#define GENERATE_SYMBOL(returnType, symbolName, ...) \
    typedef returnType (*fp_##symbolName)(__VA_ARGS__); \
    static fp_##symbolName symbolName = nullptr;

#define RESOLVE_SYMBOL(symbolName) \
    symbolName = reinterpret_cast<fp_##symbolName>(library->resolve(#symbolName)); \
    if (!symbolName) \
        return false;

struct can_bittiming {
    quint32 bitrate = 0;      /* Bit-rate in bits/second */
    quint32 sample_point = 0; /* Sample point in one-tenth of a percent */
    quint32 tq = 0;           /* Time quanta (TQ) in nanoseconds */
    quint32 prop_seg = 0;     /* Propagation segment in TQs */
    quint32 phase_seg1 = 0;   /* Phase buffer segment 1 in TQs */
    quint32 phase_seg2 = 0;   /* Phase buffer segment 2 in TQs */
    quint32 sjw = 0;          /* Synchronization jump width in TQs */
    quint32 brp = 0;          /* Bit-rate prescaler */
};

enum can_state {
    CAN_STATE_ERROR_ACTIVE = 0, /* RX/TX error count < 96 */
    CAN_STATE_ERROR_WARNING,    /* RX/TX error count < 128 */
    CAN_STATE_ERROR_PASSIVE,    /* RX/TX error count < 256 */
    CAN_STATE_BUS_OFF,          /* RX/TX error count >= 256 */
    CAN_STATE_STOPPED,          /* Device is stopped */
    CAN_STATE_SLEEPING,         /* Device is sleeping */
    CAN_STATE_MAX
};

GENERATE_SYMBOL(int, can_do_restart, const char * /* name */)
GENERATE_SYMBOL(int, can_do_stop, const char * /* name */)
GENERATE_SYMBOL(int, can_do_start, const char * /* name */)
GENERATE_SYMBOL(int, can_set_bitrate, const char * /* name */, quint32 /* bitrate */)
GENERATE_SYMBOL(int, can_get_bittiming, const char * /* name */, struct can_bittiming * /* bt */)
GENERATE_SYMBOL(int, can_get_state, const char * /* name */, int * /* state */)

LibSocketCan::LibSocketCan(QString *errorString)
{
#if QT_CONFIG(library)
    auto resolveSymbols = [](QLibrary *library) {
        const QString libName = QStringLiteral("socketcan");
        if (!library->isLoaded()) {
            library->setFileName(libName);
            if (!library->load()) {
                library->setFileNameAndVersion(libName, 2);
                if (!library->load())
                    return false;
            }
        }

        RESOLVE_SYMBOL(can_do_start);
        RESOLVE_SYMBOL(can_do_stop);
        RESOLVE_SYMBOL(can_do_restart);
        RESOLVE_SYMBOL(can_set_bitrate);
        RESOLVE_SYMBOL(can_get_bittiming);
        RESOLVE_SYMBOL(can_get_state);

        return true;
    };

    QLibrary lib;
    if (Q_UNLIKELY(!resolveSymbols(&lib))) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "%ls", qUtf16Printable(lib.errorString()));
        if (errorString)
            *errorString = lib.errorString();
    }
#else
    const QString error =
            QObject::tr("Cannot load library libsocketcan as Qt was built without QLibrary.");
    qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "%ls", qUtf16Printable(error));
    if (errorString)
        *errorString = error;
#endif
}

/*!
    Brings the CAN \a interface up.

    \internal
    \note Requires appropriate permissions.
*/
bool LibSocketCan::start(const QString &interface)
{
    if (!::can_do_start) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_do_start() is not available.");
        return false;
    }

    return ::can_do_start(interface.toLatin1().constData()) == 0;
}

/*!
    Brings the CAN \a interface down.

    \internal
    \note Requires appropriate permissions.
*/
bool LibSocketCan::stop(const QString &interface)
{
    if (!::can_do_stop) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_do_stop() is not available.");
        return false;
    }

    return ::can_do_stop(interface.toLatin1().constData()) == 0;
}

/*!
    Performs a CAN controller reset on the CAN \a interface.

    \internal
    \note Reset can only be triggerd if the controller is in bus off
    and the auto restart not turned on.
    \note Requires appropriate permissions.
 */
bool LibSocketCan::restart(const QString &interface)
{
    if (!::can_do_restart) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_do_restart() is not available.");
        return false;
    }

    return ::can_do_restart(interface.toLatin1().constData()) == 0;
}

/*!
    Returns the configured bitrate for \a interface.
    \internal
*/
quint32 LibSocketCan::bitrate(const QString &interface) const
{
    if (!::can_get_bittiming) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_get_bittiming() is not available.");
        return 0;
    }

    struct can_bittiming bt;
    if (::can_get_bittiming(interface.toLatin1().constData(), &bt) == 0)
        return bt.bitrate;

    return 0;
}

/*!
    Sets the bitrate for the CAN \a interface.

    \internal
    \note Requires appropriate permissions.
 */
bool LibSocketCan::setBitrate(const QString &interface, quint32 bitrate)
{
    if (!::can_set_bitrate) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_set_bitrate() is not available.");
        return false;
    }

    return ::can_set_bitrate(interface.toLatin1().constData(), bitrate) == 0;
}

bool LibSocketCan::hasBusStatus() const
{
    return ::can_get_state != nullptr;
}

QCanBusDevice::CanBusStatus LibSocketCan::busStatus(const QString &interface) const
{
    if (!::can_get_state) {
        qCWarning(QT_CANBUS_PLUGINS_SOCKETCAN, "Function can_get_state() is not available.");
        return QCanBusDevice::CanBusStatus::Unknown;
    }

    int status = 0;
    int result = ::can_get_state(interface.toLatin1().constData(), &status);

    if (result < 0)
        return QCanBusDevice::CanBusStatus::Unknown;

    switch (status) {
    case CAN_STATE_ERROR_ACTIVE:
        return QCanBusDevice::CanBusStatus::Good;
    case CAN_STATE_ERROR_WARNING:
        return QCanBusDevice::CanBusStatus::Warning;
    case CAN_STATE_ERROR_PASSIVE:
        return QCanBusDevice::CanBusStatus::Error;
    case CAN_STATE_BUS_OFF:
        return QCanBusDevice::CanBusStatus::BusOff;
    default:
        // Device is stopped or sleeping, so status is unknown
        return QCanBusDevice::CanBusStatus::Unknown;
    }
}

QT_END_NAMESPACE
