// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacecontent.h"
#include "qplacecontent_p.h"

QT_USE_NAMESPACE

inline QPlaceContentPrivate *QPlaceContent::d_func()
{
    return d_ptr.data();
}

inline const QPlaceContentPrivate *QPlaceContent::d_func() const
{
    return d_ptr.constData();
}

bool QPlaceContentPrivate::compare(const QPlaceContentPrivate *other) const
{
    return data == other->data;
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QPlaceContentPrivate)

/*!
    \class QPlaceContent
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceContent class holds content about places.

    A QPlaceContent holds rich content such as images, reviews, or editorials, as well
    as attributes about the content such as the user or supplier of the content. Content
    objects might hold multiple data, e.g. an item holding a review typically includes
    the user that wrote the review. Use type() to inspect the type of content a
    QPlaceContent object represents, and dataTags() to see which data is held. Use value()
    to get the individual data as a QVariant.

    \b {Note:} Some providers may \e {require} that the attribution string be displayed
    to the user whenever a piece of content is viewed.

    The rich content of a place is typically made available as paginated items.

    At present the QPlaceContent class is not extensible by 3rd parties.

    Note:  The Places API considers content objects to be 'retrieve-only' objects.
    Submission of content to a provider is not a supported use case.
*/

/*!
    \typedef QPlaceContent::Collection
    Synonym for QMap<int, QPlaceContent>.  The key of the map is an \c int representing the
    index of the content.  The value is the content object itself.

    The \c {Collection} is intended to be a container where content items, that have been retrieved
    as pages, can be stored.  This enables a developer to skip pages, for example indexes 0-9 may be
    stored in the collection,  if the user skips to indexes 80-99, these can be stored in the
    collection as well.
*/

/*!
    \enum QPlaceContent::Type
    Defines the type of content.
    \value NoType
        The content object is default constructed, any other content type may be assigned
        to this content object
    \value ImageType
        The content object is an image
    \value ReviewType
        The content object is a review
    \value EditorialType
        The content object is an editorial
    \value CustomType
        The content object is of a custom type
*/

/*!
    \enum QPlaceContent::DataTag

    Defines the value entry of the content object

    \value ContentSupplier
        The supplier who contributed this content
    \value ContentUser
        The user who contributed this content
    \value ContentAttribution
        Returns a rich text attribution string
        \note Some providers may require that the attribution
        of a particular content item always be displayed
        when the content item is shown.
    \value ImageId
        The image's identifier
    \value ImageUrl
        The image's url
    \value ImageMimeType
        The image's MIME type
    \value EditorialTitle
        The title of the editorial
    \value EditorialText
        A textual description of the place. Depending upon the provider, the
        text could be either rich (HTML based) text or plain text.
    \value EditorialLanguage
        The language of the editorial. Typically this would be a language code
        in the 2 letter ISO 639-1 format.
    \value ReviewId
        The identifier of the review
    \value ReviewDateTime
        The date and time that the review was submitted
    \value ReviewTitle
        The title of the review
    \value ReviewText
        The text of the review. Depending on the provider, the text could be
        rich (HTML based) or plain text.
    \value ReviewLanguage
        The language of the review. Typically this would be a language code
        in the 2 letter ISO 639-1 format.
    \value ReviewRating
        This review's rating of the place
    \value CustomDataTag
*/

/*!
    Constructs an content object for \a type.
*/
QPlaceContent::QPlaceContent(Type type)
    : d_ptr(new QPlaceContentPrivate(type))
{}

/*!
    Constructs a new copy of \a other.
*/
QPlaceContent::QPlaceContent(const QPlaceContent &other) noexcept = default;

/*!
    Assigns the \a other content object to this and returns a reference
    to this content object.
*/
QPlaceContent &QPlaceContent::operator=(const QPlaceContent &other) noexcept = default;

/*!
    Destroys the content object.
*/
QPlaceContent::~QPlaceContent() = default;

/*!
    \internal
*/
void QPlaceContent::detach()
{
    d_ptr.detach();
}

/*!
    Returns the content type.
*/
QPlaceContent::Type QPlaceContent::type() const
{
    if (!d_ptr)
        return NoType;
    return d_ptr->type();
}

/*!
    Returns true if this content object is equivalent to \a other,
    otherwise returns false.
*/
bool QPlaceContent::operator==(const QPlaceContent &other) const
{
    // An invalid content object is only equal to another invalid content object
    if (!d_ptr)
        return !other.d_ptr;

    if (type() != other.type())
        return false;

    return d_ptr->compare(other.d_ptr.constData());
}

/*!
    Returns true if this content object is not equivalent to \a other,
    otherwise returns false.
*/
bool QPlaceContent::operator!=(const QPlaceContent &other) const
{
    return !(*this == other);
}

/*!
    Returns the list of data tags for which values are stored in this
    content objects.
*/
QList<QPlaceContent::DataTag> QPlaceContent::dataTags() const
{
    Q_D(const QPlaceContent);
    return d->data.keys();
}

/*!
    Returns the value stored for the data \a tag, or an invalid QVariant
    if there is no data for that tag.
*/
QVariant QPlaceContent::value(QPlaceContent::DataTag tag) const
{
    Q_D(const QPlaceContent);
    return d->data.value(tag);
}

/*!
    Sets the value stored for the data \a tag to \a value.
*/
void QPlaceContent::setValue(QPlaceContent::DataTag tag, const QVariant &value)
{
    detach();
    Q_D(QPlaceContent);
    d->data[tag] = value;
}
