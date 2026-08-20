// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUSFRAME_H
#define QCANBUSFRAME_H

#include <QtCore/qmetatype.h>
#include <QtCore/qobject.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QDataStream;

class Q_SERIALBUS_EXPORT QCanBusFrame
{
    Q_GADGET

public:
    using FrameId = quint32;

    class TimeStamp {
    public:
        constexpr TimeStamp(qint64 s = 0, qint64 usec = 0) noexcept
            : secs(s), usecs(usec) {}

        constexpr static TimeStamp fromMicroSeconds(qint64 usec) noexcept
        { return TimeStamp(usec / 1000000, usec % 1000000); }

        constexpr qint64 seconds() const noexcept { return secs; }
        constexpr qint64 microSeconds() const noexcept { return usecs; }

    private:
        qint64 secs;
        qint64 usecs;
    };

    enum FrameType {
        UnknownFrame        = 0x0,
        DataFrame           = 0x1,
        ErrorFrame          = 0x2,
        RemoteRequestFrame  = 0x3,
        InvalidFrame        = 0x4
    };
    Q_ENUM(FrameType)

    explicit QCanBusFrame(FrameType type = DataFrame) noexcept :
        isExtendedFrame(0x0),
        version(Qt_5_10),
        isFlexibleDataRate(0x0),
        isBitrateSwitch(0x0),
        isErrorStateIndicator(0x0),
        isLocalEcho(0x0),
        reserved0(0x0)
    {
        Q_UNUSED(reserved0);
        ::memset(reserved, 0, sizeof(reserved));
        setFrameId(0x0);
        setFrameType(type);
    }

    enum FrameError {
        NoError                     = 0,
        TransmissionTimeoutError    = (1 << 0),
        LostArbitrationError        = (1 << 1),
        ControllerError             = (1 << 2),
        ProtocolViolationError      = (1 << 3),
        TransceiverError            = (1 << 4),
        MissingAcknowledgmentError  = (1 << 5),
        BusOffError                 = (1 << 6),
        BusError                    = (1 << 7),
        ControllerRestartError      = (1 << 8),
        UnknownError                = (1 << 9),
        AnyError                    = 0x1FFFFFFFU
        //only 29 bits usable
    };
    Q_DECLARE_FLAGS(FrameErrors, FrameError)
    Q_ENUM(FrameError)
    Q_FLAG(FrameErrors)

    explicit QCanBusFrame(QCanBusFrame::FrameId identifier, const QByteArray &data) :
        format(DataFrame),
        isExtendedFrame(0x0),
        version(Qt_5_10),
        isFlexibleDataRate(data.size() > 8 ? 0x1 : 0x0),
        isBitrateSwitch(0x0),
        isErrorStateIndicator(0x0),
        isLocalEcho(0x0),
        reserved0(0x0),
        load(data)
    {
        ::memset(reserved, 0, sizeof(reserved));
        setFrameId(identifier);
    }

    bool isValid() const noexcept
    {
        if (format == InvalidFrame)
            return false;

        // long id used, but extended flag not set
        if (!isExtendedFrame && (canId & 0x1FFFF800U))
            return false;

        if (!isValidFrameId)
            return false;

        // maximum permitted payload size in CAN or CAN FD
        const qsizetype length = load.size();
        if (isFlexibleDataRate) {
            if (format == RemoteRequestFrame)
                return false;

            return length <= 8 || length == 12 || length == 16 || length == 20
                    || length == 24 || length == 32 || length == 48 || length == 64;
        }

        return length <= 8;
    }

    constexpr FrameType frameType() const noexcept
    {
        switch (format) {
        case 0x1: return DataFrame;
        case 0x2: return ErrorFrame;
        case 0x3: return RemoteRequestFrame;
        case 0x4: return InvalidFrame;
        // no default to trigger warning
        }

        return UnknownFrame;
    }

    constexpr void setFrameType(FrameType newFormat) noexcept
    {
        switch (newFormat) {
        case DataFrame:
            format = 0x1; return;
        case ErrorFrame:
            format = 0x2; return;
        case RemoteRequestFrame:
            format = 0x3; return;
        case UnknownFrame:
            format = 0x0; return;
        case InvalidFrame:
            format = 0x4; return;
        }
    }

    constexpr bool hasExtendedFrameFormat() const noexcept { return (isExtendedFrame & 0x1); }
    constexpr void setExtendedFrameFormat(bool isExtended) noexcept
    {
        isExtendedFrame = (isExtended & 0x1);
    }

    constexpr QCanBusFrame::FrameId frameId() const noexcept
    {
        if (Q_UNLIKELY(format == ErrorFrame))
            return 0;
        return (canId & 0x1FFFFFFFU);
    }
    constexpr void setFrameId(QCanBusFrame::FrameId newFrameId)
    {
        if (Q_LIKELY(newFrameId < 0x20000000U)) {
            isValidFrameId = true;
            canId = newFrameId;
            setExtendedFrameFormat(isExtendedFrame || (newFrameId & 0x1FFFF800U));
        } else {
            isValidFrameId = false;
            canId = 0;
        }
    }

    void setPayload(const QByteArray &data)
    {
        load = data;
        if (data.size() > 8)
            isFlexibleDataRate = 0x1;
    }
    constexpr void setTimeStamp(TimeStamp ts) noexcept { stamp = ts; }

    QByteArray payload() const { return load; }
    constexpr TimeStamp timeStamp() const noexcept { return stamp; }

    constexpr FrameErrors error() const noexcept
    {
        if (format != ErrorFrame)
            return NoError;

        return FrameErrors(canId & 0x1FFFFFFFU);
    }
    constexpr void setError(FrameErrors e)
    {
        if (format != ErrorFrame)
            return;
        canId = (e & AnyError).toInt();
    }

    QString toString() const;

    constexpr bool hasFlexibleDataRateFormat() const noexcept { return (isFlexibleDataRate & 0x1); }
    constexpr void setFlexibleDataRateFormat(bool isFlexibleData) noexcept
    {
        isFlexibleDataRate = (isFlexibleData & 0x1);
        if (!isFlexibleData) {
            isBitrateSwitch = 0x0;
            isErrorStateIndicator = 0x0;
        }
    }

    constexpr bool hasBitrateSwitch() const noexcept { return (isBitrateSwitch & 0x1); }
    constexpr void setBitrateSwitch(bool bitrateSwitch) noexcept
    {
        isBitrateSwitch = (bitrateSwitch & 0x1);
        if (bitrateSwitch)
            isFlexibleDataRate = 0x1;
    }

    constexpr bool hasErrorStateIndicator() const noexcept { return (isErrorStateIndicator & 0x1); }
    constexpr void setErrorStateIndicator(bool errorStateIndicator) noexcept
    {
        isErrorStateIndicator = (errorStateIndicator & 0x1);
        if (errorStateIndicator)
            isFlexibleDataRate = 0x1;
    }
    constexpr bool hasLocalEcho() const noexcept { return (isLocalEcho & 0x1); }
    constexpr void setLocalEcho(bool localEcho) noexcept
    {
        isLocalEcho = (localEcho & 0x1);
    }

#ifndef QT_NO_DATASTREAM
    friend Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &, const QCanBusFrame &);
    friend Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &, QCanBusFrame &);
#endif

private:
    enum Version {
        Qt_5_8 = 0x0,
        Qt_5_9 = 0x1,
        Qt_5_10 = 0x2
    };

    quint32 canId:29; // acts as container for error codes too
    quint8 format:3; // max of 8 frame types

    quint8 isExtendedFrame:1;
    quint8 version:5;
    quint8 isValidFrameId:1;
    quint8 isFlexibleDataRate:1;

    quint8 isBitrateSwitch:1;
    quint8 isErrorStateIndicator:1;
    quint8 isLocalEcho:1;
    quint8 reserved0:5;

    // reserved for future use
    quint8 reserved[2];

    QByteArray load;
    TimeStamp stamp;
};

Q_DECLARE_TYPEINFO(QCanBusFrame, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::FrameError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::FrameType, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::TimeStamp, Q_PRIMITIVE_TYPE);

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanBusFrame::FrameErrors)

#ifndef QT_NO_DATASTREAM
Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &, const QCanBusFrame &);
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &, QCanBusFrame &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCanBusFrame::FrameType)
Q_DECLARE_METATYPE(QCanBusFrame::FrameErrors)

#endif // QCANBUSFRAME_H
