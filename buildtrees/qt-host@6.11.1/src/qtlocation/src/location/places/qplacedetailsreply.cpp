// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacedetailsreply.h"
#include "qplacereply_p.h"

#include <QtLocation/QPlace>

QT_BEGIN_NAMESPACE
class QPlaceDetailsReplyPrivate : public QPlaceReplyPrivate
{
public:
    QPlace result;
};

QT_END_NAMESPACE

QT_USE_NAMESPACE

/*!
    \class QPlaceDetailsReply
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-replies
    \since 5.6

    \brief The QPlaceDetailsReply class manages a place details fetch operation started by an
    instance of QPlaceManager.

    See \l {QML Places API#Fetching Place Details}{Fetching Place Details} for an example on how to use a details reply.
    \sa QPlaceManager
*/

/*!
    Constructs a details reply with a given \a parent.
*/
QPlaceDetailsReply::QPlaceDetailsReply(QObject *parent)
    : QPlaceReply(new QPlaceDetailsReplyPrivate, parent)
{
}

/*!
    Destroys the details reply.
*/
QPlaceDetailsReply::~QPlaceDetailsReply()
{
}

/*!
    Returns the type of reply.
*/
QPlaceReply::Type QPlaceDetailsReply::type() const
{
    return QPlaceReply::DetailsReply;
}

 /*!
    Returns the place that was fetched.
*/
QPlace QPlaceDetailsReply::place() const
{
    Q_D(const QPlaceDetailsReply);
    return d->result;
}

/*!
    Sets the fetched \a place of the reply.
*/
void QPlaceDetailsReply::setPlace(const QPlace &place)
{
    Q_D(QPlaceDetailsReply);
    d->result = place;
}
