// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativecontactdetails_p.h"

#include <QPlaceContactDetail>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ContactDetails
    \nativetype QDeclarativeContactDetails
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data
    \since QtLocation 5.5

    \brief The ContactDetails type holds contact details for a \l Place.

    The ContactDetails type is a map of \l contactDetail objects.
    To access contact details in the map use the \l keys() method to get the list of keys stored in
    the map and then use the \c {[]} operator to access the \l contactDetail items.

    The following keys are defined in the API.  \l Plugin implementations are free to define
    additional keys.

    \list
        \li phone
        \li fax
        \li email
        \li website
    \endlist

    ContactDetails instances are only ever used in the context of \l {Place}{Places}.  It is not possible
    to create a ContactDetails instance directly or re-assign ContactDetails instances to \l {Place}{Places}.
    Modification of ContactDetails can only be accomplished via Javascript.

    \section1 Examples

    The following example shows how to access all \l {contactDetail}{contact details}
    and print them to the console:

    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/places.qml ContactDetails read

    The returned list of contact details is an \l {QObjectList-based model}{object list} and so can be used directly as a data model.  For example, the
    following demonstrates how to display a list of contact phone numbers in a list view:

    \snippet declarative/places.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/places.qml ContactDetails phoneList

    The following example demonstrates how to assign a single phone number to a place in JavaScript:
    \snippet declarative/places.qml  ContactDetails write single

    The following demonstrates how to assign multiple phone numbers to a place in JavaScript:
    \snippet declarative/places.qml  ContactDetails write multiple
*/

/*!
    \qmlmethod variant ContactDetails::keys()

    Returns an array of contact detail keys currently stored in the map.
*/
QDeclarativeContactDetails::QDeclarativeContactDetails(QObject *parent)
    : QQmlPropertyMap(this, parent)
{
}

QVariant QDeclarativeContactDetails::updateValue(const QString &, const QVariant &input)
{
    if (input.metaType() == QMetaType::fromType<QPlaceContactDetail>()) {
        QVariantList varList;
        varList.append(input);
        return varList;
    }

    return input;
}

QT_END_NAMESPACE
