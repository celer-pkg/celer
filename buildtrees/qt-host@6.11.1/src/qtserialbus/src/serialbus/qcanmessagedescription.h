// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANMESSAGEDESCRIPTION_H
#define QCANMESSAGEDESCRIPTION_H

#include <QtCore/QDebug>
#include <QtCore/QExplicitlySharedDataPointer>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanSignalDescription;

class QCanMessageDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanMessageDescriptionPrivate, Q_SERIALBUS_EXPORT)

class QCanMessageDescription
{
public:
    Q_SERIALBUS_EXPORT QCanMessageDescription();
    Q_SERIALBUS_EXPORT QCanMessageDescription(const QCanMessageDescription &other);
    QCanMessageDescription(QCanMessageDescription &&other) noexcept = default;
    ~QCanMessageDescription() = default;

    Q_SERIALBUS_EXPORT QCanMessageDescription &operator=(const QCanMessageDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanMessageDescription)

    void swap(QCanMessageDescription &other) noexcept { d.swap(other.d); }

    Q_SERIALBUS_EXPORT bool isValid() const;

    Q_SERIALBUS_EXPORT QtCanBus::UniqueId uniqueId() const;
    Q_SERIALBUS_EXPORT void setUniqueId(QtCanBus::UniqueId id);

    Q_SERIALBUS_EXPORT QString name() const;
    Q_SERIALBUS_EXPORT void setName(const QString &name);

    Q_SERIALBUS_EXPORT quint8 size() const;
    Q_SERIALBUS_EXPORT void setSize(quint8 size);

    Q_SERIALBUS_EXPORT QString transmitter() const;
    Q_SERIALBUS_EXPORT void setTransmitter(const QString &transmitter);

    Q_SERIALBUS_EXPORT QString comment() const;
    Q_SERIALBUS_EXPORT void setComment(const QString &text);

    Q_SERIALBUS_EXPORT QList<QCanSignalDescription> signalDescriptions() const;
    Q_SERIALBUS_EXPORT QCanSignalDescription signalDescriptionForName(const QString &name) const;
    Q_SERIALBUS_EXPORT void clearSignalDescriptions();
    Q_SERIALBUS_EXPORT void addSignalDescription(const QCanSignalDescription &description);
    Q_SERIALBUS_EXPORT
    void setSignalDescriptions(const QList<QCanSignalDescription> &descriptions);

private:
    QExplicitlySharedDataPointer<QCanMessageDescriptionPrivate> d;
    friend class QCanMessageDescriptionPrivate;

    friend void qHash(const QCanMessageDescription &desc, size_t seed) noexcept = delete;
    friend void operator==(const QCanMessageDescription &lhs,
                           const QCanMessageDescription &rhs) noexcept = delete;
    friend void operator!=(const QCanMessageDescription &lhs,
                           const QCanMessageDescription &rhs) noexcept = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug dbg, const QCanMessageDescription &msg)
    {
        return debugStreaming(dbg, msg);
    }
    Q_SERIALBUS_EXPORT static QDebug debugStreaming(QDebug dbg, const QCanMessageDescription &msg);
#endif // QT_NO_DEBUG_STREAM
};

Q_DECLARE_SHARED(QCanMessageDescription)

QT_END_NAMESPACE

#endif // QCANMESSAGEDESCRIPTION_H
