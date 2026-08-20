// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtHttpServer/qhttpserverwebsocketupgraderesponse.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \class QHttpServerWebSocketUpgradeResponse
    \since 6.8
    \inmodule QtHttpServer
    \brief Response to return when verifying WebSocket upgrades on HTTP server.

    Use this class to return when determining whether a socket upgrade should
    succeed. If type() is \l Accept upgrade the socket, if type() is \l Deny
    send an error with the given denyStatus() and denyMessage(), and if type()
    is \l PassToNext proceed to the next registered handler.
    If all handlers return \l PassToNext or none exist,
    QAbstractHttpServer::missingHandler() is executed.

    \sa QAbstractHttpServer::addWebSocketUpgradeVerifier(),
    QAbstractHttpServer::missingHandler()
*/

/*!
    \enum QHttpServerWebSocketUpgradeResponse::ResponseType

    Response types

    \value Accept       Accept the WebSocket upgrade request.
    \value Deny         Deny the WebSocket upgrade request.
    \value PassToNext   Pass the Websocket upgrade decision to the next verifier if any.
    \sa QAbstractHttpServer::addWebSocketUpgradeVerifier(), type()
*/

/*!
    \internal
*/
QHttpServerWebSocketUpgradeResponse::QHttpServerWebSocketUpgradeResponse(ResponseType type)
    : responseType(type), errorMessage("Forbidden"_ba), reserved(nullptr)
{
}

/*!
    \internal
*/
QHttpServerWebSocketUpgradeResponse::QHttpServerWebSocketUpgradeResponse(ResponseType type,
                                                                         int status,
                                                                         QByteArray message)
    : responseType(type), errorStatus(status), errorMessage(message), reserved(nullptr)
{
}

/*\fn QHttpServerWebSocketUpgradeResponse::QHttpServerWebSocketUpgradeResponse(QHttpServerWebSocketUpgradeResponse &&other) noexcept

    Move-constructs an instance of a QHttpServerWebSocketUpgradeResponse object from \a other.
*/

/*!
    Copy-constructs an instance of a QHttpServerWebSocketUpgradeResponse object from \a other.
*/
QHttpServerWebSocketUpgradeResponse::QHttpServerWebSocketUpgradeResponse(
        const QHttpServerWebSocketUpgradeResponse &other)
    : responseType(other.responseType),
      errorStatus(other.errorStatus),
      errorMessage(other.errorMessage),
      reserved(nullptr)
{
}

/*!
    Destroys a QHttpServerWebSocketUpgradeResponse object.
*/
QHttpServerWebSocketUpgradeResponse::~QHttpServerWebSocketUpgradeResponse() noexcept = default;

/*!
    \fn QHttpServerWebSocketUpgradeResponse &QHttpServerWebSocketUpgradeResponse::operator=(QHttpServerWebSocketUpgradeResponse &&other) noexcept

    Move-assigns the values of \a other to this object.
*/

/*!
    Copy-assigns the values of \a other to this object.
*/
QHttpServerWebSocketUpgradeResponse &
QHttpServerWebSocketUpgradeResponse::operator=(const QHttpServerWebSocketUpgradeResponse &other)
{
    responseType = other.responseType;
    errorStatus = other.errorStatus;
    errorMessage = other.errorMessage;
    reserved = nullptr;
    return *this;
}

/*!
    \fn void QHttpServerWebSocketUpgradeResponse::swap(QHttpServerWebSocketUpgradeResponse &other) noexcept
    Swaps the contents of this with \a other
*/

/*!
    Creates an instance of QHttpServerWebSocketUpgradeResponse with type()
    \l Accept.

    \sa ResponseType, type()
*/
QHttpServerWebSocketUpgradeResponse QHttpServerWebSocketUpgradeResponse::accept()
{
    return QHttpServerWebSocketUpgradeResponse::ResponseType::Accept;
}

/*!
    Creates an instance of QHttpServerWebSocketUpgradeResponse with
    \l type() \l Deny, \l denyStatus() \c 403 and the \l denyMessage()
    \c "Forbidden".

    \sa ResponseType, type(), denyStatus(), denyMessage()
*/
QHttpServerWebSocketUpgradeResponse QHttpServerWebSocketUpgradeResponse::deny()
{
    return QHttpServerWebSocketUpgradeResponse::ResponseType::Deny;
}

/*!
    Creates an instance of QHttpServerWebSocketUpgradeResponse with type()
    \l Deny, denyStatus() \a status and denyMessage() \a message.

    \sa ResponseType, type(), denyStatus(), denyMessage()
*/
QHttpServerWebSocketUpgradeResponse QHttpServerWebSocketUpgradeResponse::deny(int status,
                                                                        QByteArray message)
{
    return { QHttpServerWebSocketUpgradeResponse::ResponseType::Deny, status, message };
}

/*!
    Creates an instance of QHttpServerWebSocketUpgradeResponse with type() \l PassToNext.

    \sa ResponseType, type()
*/
QHttpServerWebSocketUpgradeResponse QHttpServerWebSocketUpgradeResponse::passToNext()
{
    return QHttpServerWebSocketUpgradeResponse::ResponseType::PassToNext;
}

/*!
    \fn QHttpServerWebSocketUpgradeResponse::ResponseType QHttpServerWebSocketUpgradeResponse::type() const

    Returns the type of response.

    \sa ResponseType
*/

/*!
    \fn int QHttpServerWebSocketUpgradeResponse::denyStatus() const

    Returns the HTTP status code to return if type() is \l Deny.
*/

/*!
    \fn const QByteArray &QHttpServerWebSocketUpgradeResponse::denyMessage() const &

    Returns the error message to return if type() is \l Deny.
*/

/*!
    \fn QByteArray QHttpServerWebSocketUpgradeResponse::denyMessage() &&

    Returns the error message to return if type() is \l Deny.
*/

QT_END_NAMESPACE
