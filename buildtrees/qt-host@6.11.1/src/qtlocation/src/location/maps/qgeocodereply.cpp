// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocodereply.h"
#include "qgeocodereply_p.h"

#include <QtCore/QVariant>
#include <QtPositioning/QGeoLocation>

QT_BEGIN_NAMESPACE
/*!
    \class QGeoCodeReply
    \inmodule QtLocation
    \ingroup QtLocation-geocoding
    \since 5.6

    \brief The QGeoCodeReply class manages an operation started by an
    instance of QGeoCodingManager.

    Instances of QGeoCodeReply manage the state and results of these
    operations.

    The isFinished(), error() and errorString() methods provide information
    on whether the operation has completed and if it completed successfully.

    The finished() and errorOccurred(QGeoCodeReply::Error,QString)
    signals can be used to monitor the progress of the operation.

    It is possible that a newly created QGeoCodeReply may be in a finished
    state, most commonly because an error has occurred. Since such an instance
    will never emit the finished() or
    errorOccurred(QGeoCodeReply::Error,QString) signals, it is
    important to check the result of isFinished() before making the connections
    to the signals. The documentation for QGeoCodingManager demonstrates how
    this might be carried out.

    If the operation completes successfully the results will be able to be
    accessed with locations().
*/

/*!
    \enum QGeoCodeReply::Error

    Describes an error which prevented the completion of the operation.

    \value NoError
        No error has occurred.
    \value EngineNotSetError
        The geocoding manager that was used did not have a QGeoCodingManagerEngine instance associated with it.
    \value CommunicationError
        An error occurred while communicating with the service provider.
    \value ParseError
        The response from the service provider was in an unrecognizable format.
    \value UnsupportedOptionError
        The requested operation or one of the options for the operation are not
        supported by the service provider.
    \value CombinationError
        An error occurred while results where being combined from multiple sources.
    \value UnknownError
        An error occurred which does not fit into any of the other categories.
*/

/*!
    Constructs a geocode reply with the specified \a parent.
*/
QGeoCodeReply::QGeoCodeReply(QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoCodeReplyPrivate()) {}

/*!
    Constructs a geocode reply with a given \a error and \a errorString and the specified \a parent.
*/
QGeoCodeReply::QGeoCodeReply(Error error, const QString &errorString, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoCodeReplyPrivate(error, errorString))
{}

/*!
    Destroys this reply object.
*/
QGeoCodeReply::~QGeoCodeReply()
{
    delete d_ptr;
}

/*!
    Sets whether or not this reply has finished to \a finished.

    If \a finished is true, this will cause the finished() signal to be
    emitted.

    If the operation completed successfully, QGeoCodeReply::setLocations()
    should be called before this function. If an error occurred,
    QGeoCodeReply::setError() should be used instead.
*/
void QGeoCodeReply::setFinished(bool finished)
{
    d_ptr->isFinished = finished;
    if (d_ptr->isFinished)
        emit this->finished();
}

/*!
    Return true if the operation completed successfully or encountered an
    error which cause the operation to come to a halt.
*/
bool QGeoCodeReply::isFinished() const
{
    return d_ptr->isFinished;
}

/*!
    Sets the error state of this reply to \a error and the textual
    representation of the error to \a errorString.

    This will also cause errorOccurred() and finished() signals to be
    emitted, in that order.
*/
void QGeoCodeReply::setError(QGeoCodeReply::Error error, const QString &errorString)
{
    d_ptr->error = error;
    d_ptr->errorString = errorString;
    emit this->errorOccurred(error, errorString);
    setFinished(true);
}

/*!
    Returns the error state of this reply.

    If the result is QGeoCodeReply::NoError then no error has occurred.
*/
QGeoCodeReply::Error QGeoCodeReply::error() const
{
    return d_ptr->error;
}

/*!
    Returns the textual representation of the error state of this reply.

    If no error has occurred this will return an empty string.  It is possible
    that an error occurred which has no associated textual representation, in
    which case this will also return an empty string.

    To determine whether an error has occurred, check to see if
    QGeoCodeReply::error() is equal to QGeoCodeReply::NoError.
*/
QString QGeoCodeReply::errorString() const
{
    return d_ptr->errorString;
}

/*!
    Sets the viewport which contains the results to \a viewport.
*/
void QGeoCodeReply::setViewport(const QGeoShape &viewport)
{
    d_ptr->viewport = viewport;
}

/*!
    Returns the viewport which contains the results.

    This function will return \nullptr if no viewport bias
    was specified in the QGeoCodingManager function which created this reply.
*/
QGeoShape QGeoCodeReply::viewport() const
{
    return d_ptr->viewport;
}

/*!
    Returns a list of locations.

    The locations are the results of the operation corresponding to the
    QGeoCodingManager function which created this reply.
*/
QList<QGeoLocation> QGeoCodeReply::locations() const
{
    return d_ptr->locations;
}

/*!
    Adds \a location to the list of locations in this reply.
*/
void QGeoCodeReply::addLocation(const QGeoLocation &location)
{
    d_ptr->locations.append(location);
}

/*!
    Sets the list of \a locations in the reply.
*/
void QGeoCodeReply::setLocations(const QList<QGeoLocation> &locations)
{
    d_ptr->locations = locations;
}

/*!
    \fn void QGeoCodeReply::aborted()
    \since 5.9

    This signal is emitted when the operation has been cancelled.

    \sa abort()
*/

/*!
    Cancels the operation immediately.

    This will do nothing if the reply is finished.

    \sa aborted()
*/
void QGeoCodeReply::abort()
{
    if (!isFinished())
        setFinished(true);
    emit aborted();
}

/*!
    Returns the limit on the number of responses from each data source.

    If no limit was set this function will return -1.

    This may be more than locations().length() if the number of responses
    was less than the number requested.
*/
qsizetype QGeoCodeReply::limit() const
{
    return d_ptr->limit;
}

/*!
    Returns the offset into the entire result set at which to start
    fetching results.
*/
qsizetype QGeoCodeReply::offset() const
{
    return d_ptr->offset;
}

/*!
    Sets the limit on the number of responses from each data source to \a limit.

    If \a limit is -1 then all available responses will be returned.
*/
void QGeoCodeReply::setLimit(qsizetype limit)
{
    d_ptr->limit = limit;
}

/*!
    Sets the offset in the entire result set at which to start
    fetching result to \a offset.
*/
void QGeoCodeReply::setOffset(qsizetype offset)
{
    d_ptr->offset = offset;
}

/*!
    \fn void QGeoCodeReply::finished()

    This signal is emitted when this reply has finished processing.

    If error() equals QGeoCodeReply::NoError then the processing
    finished successfully.

    This signal and QGeoCodingManager::finished() will be
    emitted at the same time.

    \note Do not delete this reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/
/*!
    \fn void QGeoCodeReply::errorOccurred(QGeoCodeReply::Error error, const QString &errorString)

    This signal is emitted when an error has been detected in the processing of
    this reply. The finished() signal will probably follow.

    The error will be described by the error code \a error. If \a errorString is
    not empty it will contain a textual description of the error.

    This signal and QGeoCodingManager::errorOccurred() will be emitted at the same time.

    \note Do not delete this reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/

/*******************************************************************************
*******************************************************************************/

QGeoCodeReplyPrivate::QGeoCodeReplyPrivate() = default;

QGeoCodeReplyPrivate::QGeoCodeReplyPrivate(QGeoCodeReply::Error error, const QString &errorString)
    : error(error), errorString(errorString), isFinished(true)
{}

const QGeoCodeReplyPrivate *QGeoCodeReplyPrivate::get(const QGeoCodeReply &reply)
{
    return reply.d_ptr;
}

QGeoCodeReplyPrivate *QGeoCodeReplyPrivate::get(QGeoCodeReply &reply)
{
    return reply.d_ptr;
}

QT_END_NAMESPACE
