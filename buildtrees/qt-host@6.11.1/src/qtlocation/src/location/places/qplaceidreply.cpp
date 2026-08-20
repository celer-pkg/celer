// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceidreply.h"
#include "qplacereply_p.h"

QT_BEGIN_NAMESPACE
class QPlaceIdReplyPrivate : public QPlaceReplyPrivate
{
public:
    QPlaceIdReplyPrivate(QPlaceIdReply::OperationType operationType)
        : operationType(operationType) {}
    ~QPlaceIdReplyPrivate() {}
    QString id;
    QPlaceIdReply::OperationType operationType;
};

QT_END_NAMESPACE

QT_USE_NAMESPACE

/*!
    \class QPlaceIdReply
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-replies
    \since 5.6

    \brief The QPlaceIdReply class manages operations which return an identifier such as
           saving and removal operations of places and categories.

    The QPlaceIdReply can be considered a multipurpose reply in that it can
    be used to save places, save categories, remove places and remove categories.
    In each case it returns an identifier of the place or category that was added, modified or removed.

    See \l {Saving a place cpp}{Saving a place} for an example of how to use an identifier reply.
    \sa QPlaceManager
*/

/*!
    \enum QPlaceIdReply::OperationType
    Defines the type of operation that was used to generate this reply.
    \value SavePlace The reply was created for a save place operation
    \value RemovePlace The reply was created for a remove place operation.
    \value SaveCategory The reply was created for a save category operation
    \value RemoveCategory The reply was created for a remove category operation.
*/

/*!
    Constructs a reply which contains the identifier of the object operated upon. The reply is for the given \a operationType and with \a parent.
*/
QPlaceIdReply::QPlaceIdReply(QPlaceIdReply::OperationType operationType, QObject *parent)
    : QPlaceReply(new QPlaceIdReplyPrivate(operationType), parent) {}

/*!
    Destroys the reply.
*/
QPlaceIdReply::~QPlaceIdReply()
{
}

/*!
    Returns the type of reply.
*/
QPlaceReply::Type QPlaceIdReply::type() const
{
    return QPlaceReply::IdReply;
}

/*!
    Returns the operation type of the reply. This means whether this
    identifier reply was for a save place operation,
    remove category operation and so on.
*/
QPlaceIdReply::OperationType QPlaceIdReply::operationType() const
{
    Q_D(const QPlaceIdReply);
    return d->operationType;
}

/*!
    Returns the relevant identifier for the operation. For example for a save place operation,
    the identifier is that of the saved place.  For a category removal operation,
    it is the identifier of the category that was removed.
*/
QString QPlaceIdReply::id() const
{
    Q_D(const QPlaceIdReply);
    return d->id;
}

/*!
    Sets the \a identifier of the reply.
*/
void QPlaceIdReply::setId(const QString &identifier)
{
    Q_D(QPlaceIdReply);
    d->id = identifier;
}
