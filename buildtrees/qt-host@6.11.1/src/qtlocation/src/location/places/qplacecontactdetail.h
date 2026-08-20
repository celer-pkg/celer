// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTACTDETAIL_H
#define QPLACECONTACTDETAIL_H

#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QPlaceContactDetailPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceContactDetailPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceContactDetail
{
    Q_GADGET
    QML_VALUE_TYPE(contactDetail)
    QML_STRUCTURED_VALUE
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(QString value READ value WRITE setValue)

public:
    static const QString Phone;
    static const QString Email;
    static const QString Website;
    static const QString Fax;

    QPlaceContactDetail();
    QPlaceContactDetail(const QPlaceContactDetail &other) noexcept;
    QPlaceContactDetail(QPlaceContactDetail &&other) noexcept = default;
    ~QPlaceContactDetail();

    QPlaceContactDetail &operator=(const QPlaceContactDetail &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceContactDetail)

    void swap(QPlaceContactDetail &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QPlaceContactDetail &lhs, const QPlaceContactDetail &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceContactDetail &lhs, const QPlaceContactDetail &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString label() const;
    void setLabel(const QString &label);

    QString value() const;
    void setValue(const QString &value);

    void clear();

private:
    QSharedDataPointer<QPlaceContactDetailPrivate> d_ptr;

    bool isEqual(const QPlaceContactDetail &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceContactDetail)

#endif
