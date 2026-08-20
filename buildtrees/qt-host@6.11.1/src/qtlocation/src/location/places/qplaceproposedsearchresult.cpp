// Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceproposedsearchresult.h"
#include "qplaceproposedsearchresult_p.h"

QT_BEGIN_NAMESPACE

bool QPlaceProposedSearchResultPrivate::compare(const QPlaceSearchResultPrivate *other) const
{
    const QPlaceProposedSearchResultPrivate *od = static_cast<const QPlaceProposedSearchResultPrivate *>(other);
    return QPlaceSearchResultPrivate::compare(other) && searchRequest == od->searchRequest;
}

/*!
    \class QPlaceProposedSearchResult
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since QtLocation 5.2

    \brief The QPlaceProposedSearchResult class represents a search result containing a proposed search.

    \sa QPlaceSearchResult
*/

/*!
    Constructs a new proposed search result.
*/
QPlaceProposedSearchResult::QPlaceProposedSearchResult()
:   QPlaceSearchResult(new QPlaceProposedSearchResultPrivate)
{
}

/*!
    \fn QPlaceProposedSearchResult::QPlaceProposedSearchResult(const QPlaceSearchRequest &other)

    Contructs a copy of \a other if possible, otherwise constructs a default proposed search
    result.
*/
Q_IMPLEMENT_SEARCHRESULT_COPY_CTOR(QPlaceProposedSearchResult)

Q_IMPLEMENT_SEARCHRESULT_D_FUNC(QPlaceProposedSearchResult)

/*!
    Destroys the proposed search result.
*/
QPlaceProposedSearchResult::~QPlaceProposedSearchResult()
{
}

/*!
    Returns a place search request that can be used to perform an additional proposed search.
*/
QPlaceSearchRequest QPlaceProposedSearchResult::searchRequest() const
{
    Q_D(const QPlaceProposedSearchResult);
    return d->searchRequest;
}

/*!
    Sets the proposed search request to \a request.
*/
void QPlaceProposedSearchResult::setSearchRequest(const QPlaceSearchRequest &request)
{
    Q_D(QPlaceProposedSearchResult);
    d->searchRequest = request;
}

QT_END_NAMESPACE
