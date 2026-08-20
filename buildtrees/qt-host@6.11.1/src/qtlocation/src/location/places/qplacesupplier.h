// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESUPPLIER_H
#define QPLACESUPPLIER_H

#include <QtLocation/qlocationglobal.h>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QUrl;
class QPlaceIcon;
class QPlaceSupplierPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceSupplierPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceSupplier
{
    Q_GADGET
    QML_VALUE_TYPE(supplier)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString supplierId READ supplierId WRITE setSupplierId)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QPlaceIcon icon READ icon WRITE setIcon)

public:
    QPlaceSupplier();
    QPlaceSupplier(const QPlaceSupplier &other) noexcept;
    QPlaceSupplier(QPlaceSupplier &&other) noexcept = default;
    ~QPlaceSupplier();

    QPlaceSupplier &operator=(const QPlaceSupplier &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceSupplier)

    void swap(QPlaceSupplier &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QPlaceSupplier &lhs,
                                  const QPlaceSupplier &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceSupplier &lhs,
                                  const QPlaceSupplier &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString name() const;
    void setName(const QString &data);

    QString supplierId() const;
    void setSupplierId(const QString &identifier);

    QUrl url() const;
    void setUrl(const QUrl &data);

    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);

    bool isEmpty() const;

private:
    QSharedDataPointer<QPlaceSupplierPrivate> d;

    bool isEqual(const QPlaceSupplier &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceSupplier)

#endif // QPLACESUPPLIER_H
