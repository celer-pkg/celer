// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacecategory.h"
#include "qplacecategory_p.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QPlaceCategoryPrivate)


bool QPlaceCategoryPrivate::isEmpty() const
{
    return  categoryId.isEmpty()
            && name.isEmpty()
            && icon.isEmpty()
            && QLocation::UnspecifiedVisibility == visibility;
}

/*!
    \class QPlaceCategory
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceCategory class represents a category that a \l QPlace can be associated with.

    Categories are used to search for places based on the categories they are associated with.  The
    list/tree of available categories can be obtained from \l QPlaceManager.  The
    \l QPlaceSearchRequest::setCategories() function can be used to limit the search results to
    places with the specified categories.

    If the \l QGeoServiceProvider supports it, categories can be created and removed.  This
    functionality is available in the \l QPlaceManager class.
*/

/*!
    Constructs a category.
*/
QPlaceCategory::QPlaceCategory()
    : d(new QPlaceCategoryPrivate)
{
}

/*!
    Constructs a category which is a copy of \a other.
*/
QPlaceCategory::QPlaceCategory(const QPlaceCategory &other) noexcept = default;

/*!
    Destroys the category.
*/
QPlaceCategory::~QPlaceCategory() = default;

/*!
    Assigns \a other to this category and returns a reference to this category.
*/
QPlaceCategory &QPlaceCategory::operator=(const QPlaceCategory &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

/*!
    \fn bool QPlaceCategory::operator==(const QPlaceCategory &lhs, const QPlaceCategory &rhs) noexcept

    Returns true if \a lhs is equal to \a rhs; otherwise returns false.
*/

/*!
    \fn bool QPlaceCategory::operator!=(const QPlaceCategory &lhs, const QPlaceCategory &rhs) noexcept

    Returns true if \a lhs is not equal to \a rhs; otherwise returns false.
*/

bool QPlaceCategory::isEqual(const QPlaceCategory &other) const noexcept
{
    return d->categoryId == other.d->categoryId &&
           d->name == other.d->name &&
           (d->visibility == QLocation::UnspecifiedVisibility ||
            other.d->visibility == QLocation::UnspecifiedVisibility ||
            d->visibility == other.d->visibility) &&
           d->icon == other.d->icon;
}

/*!
    Returns the identifier of the category.  The category identifier is a string which uniquely identifies this category
    within a particular \l QPlaceManager.  The identifier is only meaningful to the QPlaceManager
    that generated it and is not transferable between managers.
*/
QString QPlaceCategory::categoryId() const
{
    return d->categoryId;
}

/*!
    Sets the \a identifier of the category.
*/
void QPlaceCategory::setCategoryId(const QString &identifier)
{
    d->categoryId = identifier;
}

/*!
    Returns the name of category.
*/
QString QPlaceCategory::name() const
{
    return d->name;
}

/*!
    Sets the \a name of the category.
*/
void QPlaceCategory::setName(const QString &name)
{
    d->name = name;
}

/*!
    Sets the \a visibility of the category.
*/
void QPlaceCategory::setVisibility(QLocation::Visibility visibility)
{
    d->visibility = visibility;
}

/*!
    Returns the visibility of the category.
*/
QLocation::Visibility QPlaceCategory::visibility() const
{
    return d->visibility;
}

/*!
    Returns the icon associated with the category.
*/
QPlaceIcon QPlaceCategory::icon() const
{
    return d->icon;
}

/*!
    Sets the \a icon of the category.
*/
void QPlaceCategory::setIcon(const QPlaceIcon &icon)
{
    d->icon = icon;
}

/*!
    Returns a boolean indicating whether the all the fields of the place category are empty or not.
*/
bool QPlaceCategory::isEmpty() const
{
    return d->isEmpty();
}

QT_END_NAMESPACE
