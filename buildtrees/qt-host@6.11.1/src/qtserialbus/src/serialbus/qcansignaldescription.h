// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANSIGNALDESCRIPTION_H
#define QCANSIGNALDESCRIPTION_H

#include <QtCore/QDebug>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QVariant>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanSignalDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanSignalDescriptionPrivate, Q_SERIALBUS_EXPORT)

class QCanSignalDescription
{
public:
    struct MultiplexValueRange {
        QVariant minimum;
        QVariant maximum;
    private:
        friend bool operator==(const MultiplexValueRange &lhs,
                               const MultiplexValueRange &rhs) noexcept
        {
            return lhs.minimum == rhs.minimum && lhs.maximum == rhs.maximum;
        }
        friend bool operator!=(const MultiplexValueRange &lhs,
                               const MultiplexValueRange &rhs) noexcept
        {
            return !(lhs == rhs);
        }
        friend size_t qHash(const MultiplexValueRange &, size_t) noexcept = delete;
#ifndef QT_NO_DEBUG_STREAM
        friend QDebug operator<<(QDebug dbg, const MultiplexValueRange &range)
        {
            return debugStreaming(dbg, range);
        }
        Q_SERIALBUS_EXPORT
        static QDebug debugStreaming(QDebug dbg, const MultiplexValueRange &range);
#endif // QT_NO_DEBUG_STREAM
    };
    using MultiplexValues = QList<MultiplexValueRange>;
    using MultiplexSignalValues = QHash<QString, MultiplexValues>;


    Q_SERIALBUS_EXPORT QCanSignalDescription();
    Q_SERIALBUS_EXPORT QCanSignalDescription(const QCanSignalDescription &other);
    QCanSignalDescription(QCanSignalDescription &&other) noexcept = default;
    ~QCanSignalDescription() = default;

    Q_SERIALBUS_EXPORT QCanSignalDescription &operator=(const QCanSignalDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanSignalDescription)

    void swap(QCanSignalDescription &other) noexcept { d.swap(other.d); }

    Q_SERIALBUS_EXPORT bool isValid() const;

    Q_SERIALBUS_EXPORT QString name() const;
    Q_SERIALBUS_EXPORT void setName(const QString &name);

    Q_SERIALBUS_EXPORT QString physicalUnit() const;
    Q_SERIALBUS_EXPORT void setPhysicalUnit(const QString &unit);

    Q_SERIALBUS_EXPORT QString receiver() const;
    Q_SERIALBUS_EXPORT void setReceiver(const QString &receiver);

    Q_SERIALBUS_EXPORT QString comment() const;
    Q_SERIALBUS_EXPORT void setComment(const QString &text);

    Q_SERIALBUS_EXPORT QtCanBus::DataSource dataSource() const;
    Q_SERIALBUS_EXPORT void setDataSource(QtCanBus::DataSource source);

    Q_SERIALBUS_EXPORT QSysInfo::Endian dataEndian() const;
    Q_SERIALBUS_EXPORT void setDataEndian(QSysInfo::Endian endian);

    Q_SERIALBUS_EXPORT QtCanBus::DataFormat dataFormat() const;
    Q_SERIALBUS_EXPORT void setDataFormat(QtCanBus::DataFormat format);

    Q_SERIALBUS_EXPORT quint16 startBit() const;
    Q_SERIALBUS_EXPORT void setStartBit(quint16 bit);

    Q_SERIALBUS_EXPORT quint16 bitLength() const;
    Q_SERIALBUS_EXPORT void setBitLength(quint16 length);

    Q_SERIALBUS_EXPORT double factor() const;
    Q_SERIALBUS_EXPORT void setFactor(double factor);

    Q_SERIALBUS_EXPORT double offset() const;
    Q_SERIALBUS_EXPORT void setOffset(double offset);

    Q_SERIALBUS_EXPORT double scaling() const;
    Q_SERIALBUS_EXPORT void setScaling(double scaling);

    Q_SERIALBUS_EXPORT double minimum() const;
    Q_SERIALBUS_EXPORT double maximum() const;
    Q_SERIALBUS_EXPORT void setRange(double minimum, double maximum);

    Q_SERIALBUS_EXPORT QtCanBus::MultiplexState multiplexState() const;
    Q_SERIALBUS_EXPORT void setMultiplexState(QtCanBus::MultiplexState state);

    Q_SERIALBUS_EXPORT MultiplexSignalValues multiplexSignals() const;
    Q_SERIALBUS_EXPORT void clearMultiplexSignals();
    Q_SERIALBUS_EXPORT void setMultiplexSignals(const MultiplexSignalValues &multiplexorSignals);
    Q_SERIALBUS_EXPORT void addMultiplexSignal(const QString &name,
                                               const MultiplexValues &ranges);
    Q_SERIALBUS_EXPORT void addMultiplexSignal(const QString &name, const QVariant &value);

private:
    QExplicitlySharedDataPointer<QCanSignalDescriptionPrivate> d;
    friend class QCanSignalDescriptionPrivate;

    friend void qHash(const QCanSignalDescription &desc, size_t seed) noexcept = delete;
    friend void operator==(const QCanSignalDescription &lhs,
                           const QCanSignalDescription &rhs) noexcept = delete;
    friend void operator!=(const QCanSignalDescription &lhs,
                           const QCanSignalDescription &rhs) noexcept = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug dbg, const QCanSignalDescription &sig)
    {
        return debugStreaming(dbg, sig);
    }
    Q_SERIALBUS_EXPORT static QDebug debugStreaming(QDebug dbg, const QCanSignalDescription &sig);
#endif // QT_NO_DEBUG_STREAM
};

Q_DECLARE_SHARED(QCanSignalDescription)

QT_END_NAMESPACE

#endif // QCANSIGNALDESCRIPTION_H
