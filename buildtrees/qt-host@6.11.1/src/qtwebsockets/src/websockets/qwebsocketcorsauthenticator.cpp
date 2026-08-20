// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

/*!
    \class QWebSocketCorsAuthenticator

    \inmodule QtWebSockets
    \since 5.3
    \brief The QWebSocketCorsAuthenticator class provides an authenticator object for
    Cross Origin Requests (CORS).

    The QWebSocketCorsAuthenticator class is used in the
    \l{QWebSocketServer::}{originAuthenticationRequired()} signal.
    The class provides a way to pass back the required information to the QWebSocketServer.
    It provides applications with fine-grained control over which origin URLs are allowed
    and which aren't.
    By default, every origin is accepted.
    To get fine-grained control, an application connects the
    \l{QWebSocketServer::}{originAuthenticationRequired()} signal to a slot.
    When the origin (QWebSocketCorsAuthenticator::origin()) is accepted,
    it calls QWebSocketCorsAuthenticator::setAllowed(true)

    \note Checking on the origin does not make much sense when the server is accessed
    via a non-browser client, as that client can set whatever origin header it likes.
    In case of a browser client, the server SHOULD check the validity of the origin.
    \sa {WebSocket Security Considerations}

    \sa QWebSocketServer
*/

#include "qwebsocketcorsauthenticator.h"
#include "qwebsocketcorsauthenticator_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebSocketCorsAuthenticatorPrivate
    \inmodule QtWebSockets
    \internal
*/

/*!
  \internal
 */
QWebSocketCorsAuthenticatorPrivate::QWebSocketCorsAuthenticatorPrivate(const QString &origin,
                                                                       bool allowed) :
    m_origin(origin),
    m_isAllowed(allowed)
{}

/*!
  Destroys the object.
 */
QWebSocketCorsAuthenticatorPrivate::~QWebSocketCorsAuthenticatorPrivate()
{}

/*!
  Constructs a new QCorsAuthencator object with the given \a origin.
  \note By default, allowed() returns true. This means that per default every origin is accepted.
 */
QWebSocketCorsAuthenticator::QWebSocketCorsAuthenticator(const QString &origin) :
    d_ptr(new QWebSocketCorsAuthenticatorPrivate(origin, true))
{
}

/*!
  Destroys the object.
 */
QWebSocketCorsAuthenticator::~QWebSocketCorsAuthenticator()
{
}

/*!
  Constructs a copy of \a other.
 */
QWebSocketCorsAuthenticator::QWebSocketCorsAuthenticator(const QWebSocketCorsAuthenticator &other) :
    d_ptr(new QWebSocketCorsAuthenticatorPrivate(other.d_ptr->m_origin, other.d_ptr->m_isAllowed))
{
}

/*!
  Assigns \a other to this authenticator object.
 */
QWebSocketCorsAuthenticator &
QWebSocketCorsAuthenticator::operator =(const QWebSocketCorsAuthenticator &other)
{
    Q_D(QWebSocketCorsAuthenticator);
    if (this != &other) {
        d->m_origin = other.d_ptr->m_origin;
        d->m_isAllowed = other.d_ptr->m_isAllowed;
    }
    return *this;
}

/*!
  Move-constructs a QWebSocketCorsAuthenticator, making it point at the same
  object \a other was pointing to.
 */
QWebSocketCorsAuthenticator::QWebSocketCorsAuthenticator(QWebSocketCorsAuthenticator &&other) noexcept :
    d_ptr(other.d_ptr.release())
{}

/*!
  Move-assigns \a other to this instance.
 */
QWebSocketCorsAuthenticator &
QWebSocketCorsAuthenticator::operator =(QWebSocketCorsAuthenticator &&other) noexcept
{
    qSwap(d_ptr, other.d_ptr);
    return *this;
}

/*!
  Swaps \a other with this authenticator.

  This operation is very fast and never fails.
 */
void QWebSocketCorsAuthenticator::swap(QWebSocketCorsAuthenticator &other) noexcept
{
    if (&other != this)
        qSwap(d_ptr, other.d_ptr);
}

/*!
  Returns the origin this autenticator is handling about.
 */
QString QWebSocketCorsAuthenticator::origin() const
{
    Q_D(const QWebSocketCorsAuthenticator);
    return d->m_origin;
}

/*!
  Allows or disallows the origin. Setting \a allowed to true, will accept the connection request
  for the given origin.

  Setting \a allowed to false, will reject the connection request.

  \note By default, all origins are accepted.
 */
void QWebSocketCorsAuthenticator::setAllowed(bool allowed)
{
    Q_D(QWebSocketCorsAuthenticator);
    d->m_isAllowed = allowed;
}

/*!
  Returns true if the origin is allowed, otherwise returns false.

  \note By default, all origins are accepted.
 */
bool QWebSocketCorsAuthenticator::allowed() const
{
    Q_D(const QWebSocketCorsAuthenticator);
    return d->m_isAllowed;
}

QT_END_NAMESPACE
