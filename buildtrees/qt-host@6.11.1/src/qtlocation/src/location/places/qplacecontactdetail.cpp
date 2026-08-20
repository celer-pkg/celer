// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacecontactdetail_p.h"
#include "qplacecontactdetail.h"

QT_USE_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QPlaceContactDetailPrivate)

bool QPlaceContactDetailPrivate::operator== (const QPlaceContactDetailPrivate &other) const
{
    return label == other.label
            && value == other.value;
}

/*!
\class QPlaceContactDetail
\brief The QPlaceContactDetail class represents a contact detail such as a phone number or website url.
\inmodule QtLocation

\ingroup QtLocation-places
\ingroup QtLocation-places-data

The detail consists of a label and value.  The label is a localized string that can be presented
to the end user that describes that detail value which is the actual phone number, email address and so on.

\section2 Contact Types

The QPlaceContactDetail class defines some constant strings which characterize standard \e {contact types}.
\list
    \li QPlaceContactDetail::Phone
    \li QPlaceContactDetail::Email
    \li QPlaceContactDetail::Website
    \li QPlaceContactDetail::Fax
\endlist

These types are used to access and modify contact details in QPlace via:
\list
    \li QPlace::contactDetails()
    \li QPlace::setContactDetails()
    \li QPlace::appendContactDetail()
    \li QPlace::contactTypes()
\endlist

The \e {contact type} is intended to be a string type so that providers are able to introduce new contact
types if necessary.
*/

/*!
    \qmlvaluetype contactDetail
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data
    \since QtLocation 5.5

    \brief The contactDetail type holds a contact detail such as a phone number or a website
           address.

    The contactDetail provides a single detail on how one could contact a \l Place.  The
    contactDetail consists of a \l {contactDetail::}{label}, which is a localized string
    describing the contact method, and a \l {contactDetail::}{value} representing the actual
    contact detail.

    \section1 Examples

    The following example demonstrates how to assign a single phone number to a place in JavaScript:
    \snippet declarative/places.qml  ContactDetails write single

    The following demonstrates how to assign multiple phone numbers to a place in JavaScript:
    \snippet declarative/places.qml  ContactDetails write multiple

    Note, due to limitations of the QQmlPropertyMap, it is not possible
    to declaratively specify the contact details in QML, it can only be accomplished
    via JavaScript.
*/

/*!
   \variable QPlaceContactDetail::Phone
   The constant to specify phone contact details
*/
const QString QPlaceContactDetail::Phone(QLatin1String("phone"));

/*!
   \variable QPlaceContactDetail::Email
   The constant to specify email contact details.
*/
const QString QPlaceContactDetail::Email(QLatin1String("email"));

/*!
   \variable QPlaceContactDetail::Website
   The constant used to specify website contact details.
*/
const QString QPlaceContactDetail::Website(QLatin1String("website"));

/*!
   \variable QPlaceContactDetail::Fax
   The constant used to specify fax contact details.
*/
const QString QPlaceContactDetail::Fax(QLatin1String("fax"));

/*!
    Constructs a contact detail.
*/
QPlaceContactDetail::QPlaceContactDetail()
    : d_ptr(new QPlaceContactDetailPrivate)
{
}

/*!
    Destroys the contact detail.
*/
QPlaceContactDetail::~QPlaceContactDetail() = default;

/*!
    Creates a copy of \a other.
*/
QPlaceContactDetail::QPlaceContactDetail(const QPlaceContactDetail &other) noexcept = default;

/*!
    Assigns \a other to this contact detail and returns a reference to this
    contact detail.
*/
QPlaceContactDetail &QPlaceContactDetail::operator=(const QPlaceContactDetail &other) noexcept
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QPlaceContactDetail::operator==(const QPlaceContactDetail &lhs, const QPlaceContactDetail &rhs) noexcept

    Returns true if the contact detail \a lhs is equal to \a rhs,
    otherwise returns false.
*/
bool QPlaceContactDetail::isEqual(const QPlaceContactDetail &other) const noexcept
{
    if (d_ptr == other.d_ptr)
        return true;
    return ( *(d_ptr.constData()) == *(other.d_ptr.constData()));
}

/*!
    \fn bool QPlaceContactDetail::operator!=(const QPlaceContactDetail &lhs, const QPlaceContactDetail &rhs) noexcept

    Returns true if the contact detail \a lhs is not equal to \a rhs,
    otherwise returns false.
*/

/*!
    \qmlproperty string QtLocation::contactDetail::label

    This property holds a label describing the contact detail.

    The label can potentially be localized. The language is dependent on the entity that sets it,
    typically this is the \l {Plugin}.  The \l {Plugin::locales} property defines
    what language is used.
*/

/*!
    \property QPlaceContactDetail::label
    \brief a label describing the contact detail.

    The label can potentially be localized. The language is dependent on the entity that sets it,
    typically this is the manager from which the places are sourced.
    The QPlaceManager::locales() field defines what language is used.
*/
QString QPlaceContactDetail::label() const
{
    return d_ptr->label;
}

void QPlaceContactDetail::setLabel(const QString &label)
{
    d_ptr->label = label;
}

/*!
    \qmlproperty string QtLocation::contactDetail::value

    This property holds the value of the contact detail which may be a phone number, an email
    address, a website url and so on.
*/

/*!
    \property QPlaceContactDetail::value
    \brief the value of the contact detail.
*/
QString QPlaceContactDetail::value() const
{
    return d_ptr->value;
}

void QPlaceContactDetail::setValue(const QString &value)
{
    d_ptr->value = value;
}

/*!
    Clears the contact detail.
*/
void QPlaceContactDetail::clear()
{
    d_ptr->label.clear();
    d_ptr->value.clear();
}

#include "moc_qplacecontactdetail.cpp"
