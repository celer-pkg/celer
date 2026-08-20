// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractoauthreplyhandler.h"
#include "qabstractoauthreplyhandler_p.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcReplyHandler, "qt.networkauth.replyhandler")

/*!
    \class QAbstractOAuthReplyHandler
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief Handles replies to OAuth authentication requests.
    \since 5.8

    The QAbstractOAuthReplyHandler class handles the answers
    to all OAuth authentication requests.
    This class is designed as a base whose subclasses implement
    custom behavior in the callback() and networkReplyFinished()
    methods.
*/

/*!
    \fn QString QAbstractOAuthReplyHandler::callback() const

    Returns an absolute URI that the server will redirect the
    resource owner back to when the Resource Owner Authorization step
    is completed.  If the client is unable to receive callbacks or a
    callback URI has been established via other means, the parameter
    value \b must be set to "oob" (all lower-case), to indicate an
    out-of-band configuration.

    Derived classes should implement this function to provide the
    expected callback type.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::networkReplyFinished(QNetworkReply *reply)

    After the server determines whether the request is valid this
    function will be called. Reimplement it to get the data received
    from the server wrapped in \a reply. \a reply will be automatically
    deleted using deleteLater(), it thus must not be stored beyond the
    scope of this function.

*/

/*!
    \fn void QAbstractOAuthReplyHandler::callbackReceived(const QVariantMap &values)

    This signal is emitted when the reply from the server is
    received, with \a values containing the token credentials
    and any additional information the server may have returned.
    When this signal is emitted, the authorization process
    is complete.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::tokensReceived(const QVariantMap &tokens)

    This signal is emitted when new \a tokens are received from the
    server.
*/

/*!

    \fn void QAbstractOAuthReplyHandler::tokenRequestErrorOccurred(QAbstractOAuth::Error error,
                                                           const QString& errorString)

    This signal is emitted when a token request or refresh \a error has
    occurred. The \a errorString may provide further details on the error.

    \sa QAbstractOAuth::requestFailed()
    \since 6.6
*/

/*!
    \fn void QAbstractOAuthReplyHandler::replyDataReceived(const QByteArray &data)

    This signal is emitted when an HTTP request finishes and the
    data is available. \a data contains the response before parsing.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::callbackDataReceived(const QByteArray &data)

    This signal is emitted when a callback request is received:
    \a data contains the information before parsing.
*/

/*!
    Constructs a reply handler as a child of \a parent.
*/
QAbstractOAuthReplyHandler::QAbstractOAuthReplyHandler(QObject *parent)
    : QObject(parent)
{}

/*!
    Destroys the reply handler.
*/
QAbstractOAuthReplyHandler::~QAbstractOAuthReplyHandler()
{}

/*! \internal */
QAbstractOAuthReplyHandler::QAbstractOAuthReplyHandler(QObjectPrivate &d, QObject *parent)
    : QObject(d, parent)
{}

QT_END_NAMESPACE
