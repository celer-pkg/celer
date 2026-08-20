// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECATEGORY_H
#define QPLACECATEGORY_H

#include <QtLocation/qlocationglobal.h>

#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtLocation/QLocation>

QT_BEGIN_NAMESPACE

class QPlaceIcon;

class QPlaceCategoryPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceCategoryPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceCategory
{
public:
    QPlaceCategory();
    QPlaceCategory(const QPlaceCategory &other) noexcept;
    QPlaceCategory(QPlaceCategory &&other) noexcept = default;
    ~QPlaceCategory();

    QPlaceCategory &operator=(const QPlaceCategory &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceCategory)

    void swap(QPlaceCategory &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QPlaceCategory &lhs, const QPlaceCategory &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceCategory &lhs, const QPlaceCategory &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString categoryId() const;
    void setCategoryId(const QString &identifier);

    QString name() const;
    void setName(const QString &name);

    QLocation::Visibility visibility() const;
    void setVisibility(QLocation::Visibility visibility);

    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);

    bool isEmpty() const;

private:
    QSharedDataPointer<QPlaceCategoryPrivate> d;

    bool isEqual(const QPlaceCategory &other) const noexcept;
};

Q_DECLARE_TYPEINFO(QPlaceCategory, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceCategory)

#endif // QPLACECATEGORY_H
