// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANUNIQUEIDDESCRIPTION_H
#define QCANUNIQUEIDDESCRIPTION_H

#include <QtCore/QExplicitlySharedDataPointer>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanUniqueIdDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanUniqueIdDescriptionPrivate, Q_SERIALBUS_EXPORT)

class QCanUniqueIdDescription
{
public:
    Q_SERIALBUS_EXPORT QCanUniqueIdDescription();
    Q_SERIALBUS_EXPORT QCanUniqueIdDescription(const QCanUniqueIdDescription &other);
    QCanUniqueIdDescription(QCanUniqueIdDescription &&other) noexcept = default;
    ~QCanUniqueIdDescription() = default;

    Q_SERIALBUS_EXPORT QCanUniqueIdDescription &operator=(const QCanUniqueIdDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanUniqueIdDescription)

    void swap(QCanUniqueIdDescription &other) noexcept { d.swap(other.d); }

    Q_SERIALBUS_EXPORT bool isValid() const;

    Q_SERIALBUS_EXPORT QtCanBus::DataSource source() const;
    Q_SERIALBUS_EXPORT void setSource(QtCanBus::DataSource source);

    Q_SERIALBUS_EXPORT quint16 startBit() const;
    Q_SERIALBUS_EXPORT void setStartBit(quint16 bit);

    Q_SERIALBUS_EXPORT quint8 bitLength() const;
    Q_SERIALBUS_EXPORT void setBitLength(quint8 length);

    Q_SERIALBUS_EXPORT QSysInfo::Endian endian() const;
    Q_SERIALBUS_EXPORT void setEndian(QSysInfo::Endian endian);

private:
    QExplicitlySharedDataPointer<QCanUniqueIdDescriptionPrivate> d;
    friend class QCanUniqueIdDescriptionPrivate;

    friend void qHash(const QCanUniqueIdDescription &desc, size_t seed) noexcept = delete;
    friend void operator==(const QCanUniqueIdDescription &lhs,
                           const QCanUniqueIdDescription &rhs) noexcept = delete;
    friend void operator!=(const QCanUniqueIdDescription &lhs,
                           const QCanUniqueIdDescription &rhs) noexcept = delete;
};

QT_END_NAMESPACE

#endif // QCANUNIQUEIDDESCRIPTION_H
