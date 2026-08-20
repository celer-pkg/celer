// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEATTRIBUTE_H
#define QPLACEATTRIBUTE_H

#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QPlaceAttributePrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceAttributePrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceAttribute
{
    Q_GADGET
    QML_VALUE_TYPE(placeAttribute)
    QML_STRUCTURED_VALUE
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    static const QString OpeningHours;
    static const QString Payment;
    static const QString Provider;

    QPlaceAttribute();
    QPlaceAttribute(const QPlaceAttribute &other) noexcept;
    QPlaceAttribute(QPlaceAttribute &&other) noexcept = default;
    ~QPlaceAttribute();

    QPlaceAttribute &operator=(const QPlaceAttribute &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceAttribute)

    void swap(QPlaceAttribute &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QPlaceAttribute &lhs, const QPlaceAttribute &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceAttribute &lhs, const QPlaceAttribute &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString label() const;
    void setLabel(const QString &label);

    QString text() const;
    void setText(const QString &text);

    bool isEmpty() const;

private:
    QSharedDataPointer<QPlaceAttributePrivate> d_ptr;
    bool isEqual(const QPlaceAttribute &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceAttribute)

#endif
