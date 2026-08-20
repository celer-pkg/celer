// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEMATCHREQUEST_H
#define QPLACEMATCHREQUEST_H

#include <QtLocation/qlocationglobal.h>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QPlace;
class QPlaceSearchResult;
class QPlaceMatchRequestPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceMatchRequestPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceMatchRequest
{
public:
    static const QString AlternativeId;

    QPlaceMatchRequest();
    QPlaceMatchRequest(const QPlaceMatchRequest &other) noexcept;
    QPlaceMatchRequest(QPlaceMatchRequest &&other) noexcept = default;
    ~QPlaceMatchRequest();

    QPlaceMatchRequest &operator=(const QPlaceMatchRequest &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceMatchRequest)

    void swap(QPlaceMatchRequest &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QPlaceMatchRequest &lhs, const QPlaceMatchRequest &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceMatchRequest &lhs, const QPlaceMatchRequest &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QList<QPlace> places() const;
    void setPlaces(const QList<QPlace> &places);

    void setResults(const QList<QPlaceSearchResult> &results);

    QVariantMap parameters() const;
    void setParameters(const QVariantMap &parameters);

    void clear();

private:
    QSharedDataPointer<QPlaceMatchRequestPrivate> d_ptr;

    bool isEqual(const QPlaceMatchRequest &other) const noexcept;
    inline QPlaceMatchRequestPrivate *d_func();
    inline const QPlaceMatchRequestPrivate *d_func() const;
};

QT_END_NAMESPACE

#endif
