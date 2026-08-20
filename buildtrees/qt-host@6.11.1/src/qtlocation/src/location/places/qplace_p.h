// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACE_P_H
#define QPLACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QSharedData>
#include <QUrl>
#include <QtPositioning/qgeoaddress.h>
#include <QtPositioning/qgeocoordinate.h>
#include <QtPositioning/qgeolocation.h>
#include <QtPositioning/qgeorectangle.h>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/qplace.h>
#include <QtLocation/qplaceattribute.h>
#include <QtLocation/qplacecategory.h>
#include <QtLocation/qplacecontactdetail.h>
#include <QtLocation/qplaceratings.h>
#include <QtLocation/qplacesupplier.h>
#include <QtLocation/QPlaceIcon>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QPlacePrivate : public QSharedData
{
public:
    QPlacePrivate();
    QPlacePrivate(const QPlacePrivate &other);
    virtual ~QPlacePrivate();
    virtual QPlacePrivate *clone() = 0;

    bool operator==(const QPlacePrivate &other) const;

    virtual bool isEmpty() const;
    virtual QList<QPlaceCategory> categories() const = 0;
    virtual void setCategories(const QList<QPlaceCategory> &categories) = 0;
    virtual QGeoLocation location() const = 0;
    virtual void setLocation(const QGeoLocation &location) = 0;
    virtual QPlaceRatings ratings() const = 0;
    virtual void setRatings(const QPlaceRatings &ratings) = 0;
    virtual QPlaceSupplier supplier() const = 0;
    virtual void setSupplier(const QPlaceSupplier &supplier) = 0;
    virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;
    virtual QString placeId() const = 0;
    virtual void setPlaceId(const QString &placeIdentifier) = 0;
    virtual QString attribution() const = 0;
    virtual void setAttribution(const QString &attribution) = 0;
    virtual QLocation::Visibility visibility() const = 0;
    virtual void setVisibility(QLocation::Visibility visibility) = 0;
    virtual QPlaceIcon icon() const = 0;
    virtual void setIcon(const QPlaceIcon &icon) = 0;
    virtual bool detailsFetched() const = 0;
    virtual void setDetailsFetched(bool fetched) = 0;

    virtual QMap<QString, QPlaceAttribute> extendedAttributes() const = 0;
    virtual QMap<QString, QPlaceAttribute> &extendedAttributes() = 0;
    virtual QMap<QString, QList<QPlaceContactDetail> > contacts() const = 0;
    virtual QMap<QString, QList<QPlaceContactDetail> > &contacts() = 0;
    virtual QPlaceAttribute extendedAttribute(const QString &attributeType) const;


    // The place content, that has to be manually retrieved from the place manager and manually added to the place.
    // Currently, place content types can be:
    //    ImageType,
    //    ReviewType,
    //    EditorialType,
    //    CustomType = 0x0100
    QMap<QPlaceContent::Type, QPlaceContent::Collection> m_contentCollections;
    QMap<QPlaceContent::Type, int> m_contentCounts;
};


class Q_LOCATION_EXPORT QPlacePrivateDefault : public QPlacePrivate
{
public:
    QPlacePrivateDefault();
    QPlacePrivateDefault(const QPlacePrivateDefault &other);
    virtual ~QPlacePrivateDefault();
    QPlacePrivate *clone() override;

    QList<QPlaceCategory> categories() const override;
    void setCategories(const QList<QPlaceCategory> &categories) override;
    QGeoLocation location() const override;
    void setLocation(const QGeoLocation &location) override;
    QPlaceRatings ratings() const override;
    void setRatings(const QPlaceRatings &ratings) override;
    QPlaceSupplier supplier() const override;
    void setSupplier(const QPlaceSupplier &supplier) override;
    QString name() const override;
    void setName(const QString &name) override;
    QString placeId() const override;
    void setPlaceId(const QString &placeIdentifier) override;
    QString attribution() const override;
    void setAttribution(const QString &attribution) override;
    QLocation::Visibility visibility() const override;
    void setVisibility(QLocation::Visibility visibility) override;
    QPlaceIcon icon() const override;
    void setIcon(const QPlaceIcon &icon) override;
    bool detailsFetched() const override;
    void setDetailsFetched(bool fetched) override;

    QMap<QString, QPlaceAttribute> extendedAttributes() const override;
    QMap<QString, QPlaceAttribute> &extendedAttributes() override;
    QMap<QString, QList<QPlaceContactDetail> > contacts() const override;
    QMap<QString, QList<QPlaceContactDetail> > &contacts() override;


    // data members

    QList<QPlaceCategory> m_categories;
    QGeoLocation m_location;
    QPlaceRatings m_ratings;
    QPlaceSupplier m_supplier;
    QString m_name;
    QString m_placeId;
    QString m_attribution;

    QMap<QString, QPlaceAttribute> m_extendedAttributes;
    QMap<QString, QList<QPlaceContactDetail> > m_contacts;

    QLocation::Visibility m_visibility;
    QPlaceIcon m_icon;
    bool m_detailsFetched;
};

QT_END_NAMESPACE

#endif

