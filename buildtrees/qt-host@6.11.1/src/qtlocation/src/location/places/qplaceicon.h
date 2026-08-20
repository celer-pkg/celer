// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEICON_H
#define QPLACEICON_H

#include <QtLocation/qlocationglobal.h>

#include <QtCore/QMetaType>
#include <QtCore/QSize>
#include <QtCore/QSharedDataPointer>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QUrl;
class QPlaceManager;
class QPlaceIconPrivate;

QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceIconPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceIcon
{
    Q_GADGET
    QML_VALUE_TYPE(icon)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(QVariantMap parameters READ parameters WRITE setParameters)
    Q_PROPERTY(QPlaceManager *manager READ manager WRITE setManager)

public:
    static const QString SingleUrl;

    QPlaceIcon();
    QPlaceIcon(const QPlaceIcon &other) noexcept;
    QPlaceIcon(QPlaceIcon &&other) noexcept = default;
    ~QPlaceIcon();

    QPlaceIcon &operator=(const QPlaceIcon &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceIcon)

    void swap(QPlaceIcon &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QPlaceIcon &lhs, const QPlaceIcon &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceIcon &lhs, const QPlaceIcon &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    Q_INVOKABLE QUrl url(const QSize &size = QSize()) const;

    QPlaceManager *manager() const;
    void setManager(QPlaceManager *manager);

    QVariantMap parameters() const;
    void setParameters(const QVariantMap &parameters);

    bool isEmpty() const;

private:
    QSharedDataPointer<QPlaceIconPrivate> d;

    bool isEqual(const QPlaceIcon &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceIcon)

#endif
