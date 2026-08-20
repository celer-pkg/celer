// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceuser.h"
#include "qplaceuser_p.h"

QT_USE_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QPlaceUserPrivate)

bool QPlaceUserPrivate::operator==(const QPlaceUserPrivate &other) const
{
    return userId == other.userId && name == other.name;
}

/*!
    \class QPlaceUser
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceUser class represents an individual user.
*/

/*!
    \qmlvaluetype user
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data
    \since QtLocation 5.5

    \brief The user type identifies a user who contributed a particular \l Place content item.

    Each \l Place content item has an associated user who contributed the content.  This type
    provides information about that user.

    \section1 Example

    The following example shows how to display information about the user who
    submitted an editorial:

    \snippet declarative/places.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/places.qml EditorialModel

    \sa ImageModel, ReviewModel, EditorialModel
*/

/*!
    Constructs a new user object.
*/
QPlaceUser::QPlaceUser()
    : d(new QPlaceUserPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QPlaceUser::QPlaceUser(const QPlaceUser &other) noexcept = default;

/*!
    Destroys the user object.
*/
QPlaceUser::~QPlaceUser() = default;

/*!
    Assigns \a other to this user and returns a reference to this user.
*/
QPlaceUser &QPlaceUser::operator=(const QPlaceUser &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

/*!
    \fn bool QPlaceUser::operator==(const QPlaceUser &lhs, const QPlaceUser &rhs) noexcept

    Returns true if \a lhs is equal to \a rhs, otherwise returns false.
*/

/*!
    \fn bool QPlaceUser::operator!=(const QPlaceUser &lhs, const QPlaceUser &rhs) noexcept

    Returns true if \a lhs is not equal to \a rhs, otherwise returns false.
*/

bool QPlaceUser::isEqual(const QPlaceUser &other) const noexcept
{
    return (*d) == *(other.d);
}

/*!
    \qmlproperty string QtLocation::user::userId

    This property holds the unique identifier of the user.
*/

/*!
    \property QPlaceUser::userId
    \brief the identifier of the user.
*/
QString QPlaceUser::userId() const
{
    return d->userId;
}

void QPlaceUser::setUserId(const QString &identifier)
{
    d->userId = identifier;
}

/*!
    \qmlproperty string QtLocation::user::name

    This property holds the name of a user.
*/

/*!
    \property QPlaceUser::name
    \brief the name of the user.
*/
QString QPlaceUser::name() const
{
    return d->name;
}

void QPlaceUser::setName(const QString &name)
{
    d->name = name;
}

#include "moc_qplaceuser.cpp"
