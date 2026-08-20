// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacesearchsuggestionreply.h"
#include "qplacereply_p.h"

QT_BEGIN_NAMESPACE

class QPlaceSearchSuggestionReplyPrivate : public QPlaceReplyPrivate
{
public:
    QStringList suggestions;
};

QT_END_NAMESPACE

QT_USE_NAMESPACE

/*!
    \class QPlaceSearchSuggestionReply
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-replies
    \since 5.6

    \brief The QPlaceSearchSuggestionReply class manages a search suggestion operation started by an
    instance of QPlaceManager.

    On successful completion of the operation, the reply will contain a list of search term
    suggestions.
    See \l {Search Suggestions} for an example on how to use a search suggestion reply.

    \sa QPlaceManager
*/

/*!
    Constructs a search suggestion reply with a given \a parent.
*/
QPlaceSearchSuggestionReply::QPlaceSearchSuggestionReply(QObject *parent)
    : QPlaceReply(new QPlaceSearchSuggestionReplyPrivate, parent)
{
}

/*!
    Destroys the reply.
*/
QPlaceSearchSuggestionReply::~QPlaceSearchSuggestionReply()
{
}

/*!
    Returns the search term suggestions.
*/
QStringList QPlaceSearchSuggestionReply::suggestions() const
{
    Q_D(const QPlaceSearchSuggestionReply);
    return d->suggestions;
}

/*!
   Returns type of reply.
*/
QPlaceReply::Type QPlaceSearchSuggestionReply::type() const
{
    return QPlaceReply::SearchSuggestionReply;
}

/*!
    Sets the search term \a suggestions.
*/
void QPlaceSearchSuggestionReply::setSuggestions(const QStringList &suggestions)
{
    Q_D(QPlaceSearchSuggestionReply);
    d->suggestions = suggestions;
}
