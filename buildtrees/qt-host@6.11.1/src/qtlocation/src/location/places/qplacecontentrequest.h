// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTENTREQUEST_H
#define QPLACECONTENTREQUEST_H

#include <QtCore/QSharedDataPointer>

#include <QtLocation/QPlaceContent>

QT_BEGIN_NAMESPACE

class QPlaceContentRequestPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceContentRequestPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceContentRequest
{
public:
    QPlaceContentRequest();
    QPlaceContentRequest(const QPlaceContentRequest &other) noexcept;
    QPlaceContentRequest(QPlaceContentRequest &&other) noexcept = default;
    ~QPlaceContentRequest();

    QPlaceContentRequest &operator=(const QPlaceContentRequest &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceContentRequest)

    void swap(QPlaceContentRequest &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QPlaceContentRequest &lhs,
                                  const QPlaceContentRequest &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceContentRequest &lhs,
                                  const QPlaceContentRequest &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QPlaceContent::Type contentType() const;
    void setContentType(QPlaceContent::Type type);

    QString placeId() const;
    void setPlaceId(const QString &identifier);

    QVariant contentContext() const;
    void setContentContext(const QVariant &context);

    int limit() const;
    void setLimit(int limit);

    void clear();

private:
    QSharedDataPointer<QPlaceContentRequestPrivate> d_ptr;
    bool isEqual(const QPlaceContentRequest &other) const noexcept;

    inline QPlaceContentRequestPrivate *d_func();
    inline const QPlaceContentRequestPrivate *d_func() const;
};

QT_END_NAMESPACE

#endif
