// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHREQUEST_H
#define QPLACESEARCHREQUEST_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QMetaType>
#include <QtLocation/QLocation>

QT_BEGIN_NAMESPACE

class QGeoShape;
class QPlaceCategory;
class QPlaceSearchRequestPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceSearchRequestPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceSearchRequest
{
public:
    enum RelevanceHint {
        UnspecifiedHint,
        DistanceHint,
        LexicalPlaceNameHint
    };

    QPlaceSearchRequest();
    QPlaceSearchRequest(const QPlaceSearchRequest &other) noexcept;
    QPlaceSearchRequest(QPlaceSearchRequest &&other) noexcept = default;
    ~QPlaceSearchRequest();

    QPlaceSearchRequest &operator=(const QPlaceSearchRequest &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceSearchRequest)

    void swap(QPlaceSearchRequest &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QPlaceSearchRequest &lhs,
                                  const QPlaceSearchRequest &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceSearchRequest &lhs,
                                  const QPlaceSearchRequest &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QString searchTerm() const;
    void setSearchTerm(const QString &term);

    QList<QPlaceCategory> categories() const;
    void setCategory(const QPlaceCategory &category);
    void setCategories(const QList<QPlaceCategory> &categories);

    QGeoShape searchArea() const;
    void setSearchArea(const QGeoShape &area);

    QString recommendationId() const;
    void setRecommendationId(const QString &recommendationId);

    QVariant searchContext() const;
    void setSearchContext(const QVariant &context);

    QLocation::VisibilityScope visibilityScope() const;
    void setVisibilityScope(QLocation::VisibilityScope visibilityScopes);

    RelevanceHint relevanceHint() const;
    void setRelevanceHint(RelevanceHint hint);

    int limit() const;
    void setLimit(int limit);

    void clear();

private:
    QSharedDataPointer<QPlaceSearchRequestPrivate> d_ptr;

    bool isEqual(const QPlaceSearchRequest &other) const noexcept;

    inline QPlaceSearchRequestPrivate *d_func();
    inline const QPlaceSearchRequestPrivate *d_func() const;

    friend class QPlaceSearchRequestPrivate;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceSearchRequest::RelevanceHint)

#endif // QPLACESEARCHQUERY_H
