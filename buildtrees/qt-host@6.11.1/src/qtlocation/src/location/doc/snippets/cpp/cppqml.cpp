// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtLocation/QPlaceCategory>
#include <QtLocation/QPlaceContactDetail>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceAttribute>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceUser>
#include <QtLocation/QPlaceRatings>
#include <QtLocation/QPlaceSupplier>

void cppQmlInterface(QObject *qmlObject)
{
    //! [Category get]
    QPlaceCategory category = qmlObject->property("category").value<QPlaceCategory>();
    //! [Category get]

    //! [Category set]
    qmlObject->setProperty("category", QVariant::fromValue(category));
    //! [Category set]

    //! [ContactDetail get]
    QPlaceContactDetail contactDetail = qmlObject->property("contactDetail").value<QPlaceContactDetail>();
    //! [ContactDetail get]

    //! [ContactDetail set]
    qmlObject->setProperty("contactDetail", QVariant::fromValue(contactDetail));
    //! [ContactDetail set]

    //! [Place get]
    QPlace place = qmlObject->property("place").value<QPlace>();
    //! [Place get]

    //! [Place set]
    qmlObject->setProperty("place", QVariant::fromValue(place));
    //! [Place set]

    //! [PlaceAttribute get]
    QPlaceAttribute placeAttribute = qmlObject->property("attribute").value<QPlaceAttribute>();
    //! [PlaceAttribute get]

    //! [PlaceAttribute set]
    qmlObject->setProperty("attribute", QVariant::fromValue(placeAttribute));
    //! [PlaceAttribute set]

    //! [Icon get]
    QPlaceIcon placeIcon = qmlObject->property("icon").value<QPlaceIcon>();
    //! [Icon get]

    //! [Icon set]
    qmlObject->setProperty("icon", QVariant::fromValue(placeIcon));
    //! [Icon set]

    //! [User get]
    QPlaceUser placeUser = qmlObject->property("user").value<QPlaceUser>();
    //! [User get]

    //! [User set]
    qmlObject->setProperty("user", QVariant::fromValue(placeUser));
    //! [User set]

    //! [Ratings get]
    QPlaceRatings placeRatings = qmlObject->property("ratings").value<QPlaceRatings>();
    //! [Ratings get]

    //! [Ratings set]
    qmlObject->setProperty("ratings", QVariant::fromValue(placeRatings));
    //! [Ratings set]

    //! [Supplier get]
    QPlaceSupplier placeSupplier = qmlObject->property("supplier").value<QPlaceSupplier>();
    //! [Supplier get]

    //! [Supplier set]
    qmlObject->setProperty("supplier", QVariant::fromValue(placeSupplier));
    //! [Supplier set]
}

