// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEUSER_H
#define QPLACEUSER_H

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QPlaceUserPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceUserPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceUser
{
    Q_GADGET
    QML_VALUE_TYPE(user)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(QString userId READ userId WRITE setUserId)
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    QPlaceUser();
    QPlaceUser(const QPlaceUser &other) noexcept;
    QPlaceUser(QPlaceUser &&other) noexcept = default;
    ~QPlaceUser();

    QPlaceUser &operator=(const QPlaceUser &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceUser)

    void swap(QPlaceUser &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QPlaceUser &lhs, const QPlaceUser &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceUser &lhs, const QPlaceUser &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString userId() const;
    void setUserId(const QString &identifier);

    QString name() const;
    void setName(const QString &name);

private:
    QSharedDataPointer<QPlaceUserPrivate> d;

    bool isEqual(const QPlaceUser &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceUser)

#endif
