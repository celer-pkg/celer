// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACE_H
#define QPLACE_H

#include <QtCore/QSharedDataPointer>
#include <QtLocation/QLocation>
#include <QtLocation/QPlaceContent>

QT_BEGIN_NAMESPACE

class QString;
class QGeoLocation;
class QPlaceAttribute;
class QPlaceCategory;
class QPlaceContactDetail;
class QPlaceIcon;
class QPlacePrivate;
class QPlaceRatings;

QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlacePrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlace
{
public:
    QPlace();
    QPlace(const QPlace &other) noexcept;
    QPlace(QPlace &&other) noexcept = default;
    ~QPlace();

    QPlace &operator=(const QPlace &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlace)

    void swap(QPlace &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend bool operator==(const QPlace &lhs, const QPlace &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend bool operator!=(const QPlace &lhs, const QPlace &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    QList<QPlaceCategory> categories() const;
    void setCategory(const QPlaceCategory &category);
    void setCategories(const QList<QPlaceCategory> &categories);
    QGeoLocation location() const;
    void setLocation(const QGeoLocation &location);
    QPlaceRatings ratings() const;
    void setRatings(const QPlaceRatings &ratings);
    QPlaceSupplier supplier() const;
    void setSupplier(const QPlaceSupplier &supplier);

    QString attribution() const;
    void setAttribution(const QString &attribution);

    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);

    QPlaceContent::Collection content(QPlaceContent::Type type) const;
    void setContent(QPlaceContent::Type type, const QPlaceContent::Collection &content);
    void insertContent(QPlaceContent::Type type, const QPlaceContent::Collection &content);

    int totalContentCount(QPlaceContent::Type type) const;
    void setTotalContentCount(QPlaceContent::Type type, int total);

    QString name() const;
    void setName(const QString &name);
    QString placeId() const;
    void setPlaceId(const QString &identifier);

    QString primaryPhone() const;
    QString primaryFax() const;
    QString primaryEmail() const;
    QUrl primaryWebsite() const;

    bool detailsFetched() const;
    void setDetailsFetched(bool fetched);

    QStringList extendedAttributeTypes() const;
    QPlaceAttribute extendedAttribute(const QString &attributeType) const;
    void setExtendedAttribute(const QString &attributeType, const QPlaceAttribute &attribute);
    void removeExtendedAttribute(const QString &attributeType);

    QStringList contactTypes() const;
    QList<QPlaceContactDetail> contactDetails(const QString &contactType) const;
    void setContactDetails(const QString &contactType, QList<QPlaceContactDetail> details);
    void appendContactDetail(const QString &contactType, const QPlaceContactDetail &detail);
    void removeContactDetails(const QString &contactType);

    QLocation::Visibility visibility() const;
    void setVisibility(QLocation::Visibility visibility);

    bool isEmpty() const;

protected:
    QPlace(const QSharedDataPointer<QPlacePrivate> &dd);

private:
    QSharedDataPointer<QPlacePrivate> d_ptr;

    bool isEqual(const QPlace &other) const noexcept;

    inline QPlacePrivate *d_func();
    inline const QPlacePrivate *d_func() const;
    friend class QDeclarativePlace;
};

Q_DECLARE_TYPEINFO(QPlace, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlace)

#endif
