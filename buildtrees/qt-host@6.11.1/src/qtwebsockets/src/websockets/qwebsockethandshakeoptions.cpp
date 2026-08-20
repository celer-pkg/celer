// Copyright (C) 2022 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebsockethandshakeoptions_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebSocketHandshakeOptions

    \inmodule QtWebSockets
    \since 6.4
    \brief Collects options for the WebSocket handshake.

    QWebSocketHandshakeOptions collects options that are passed along to the
    WebSocket handshake, such as WebSocket subprotocols and WebSocket
    Extensions.

    At the moment, only WebSocket subprotocols are supported.

    \sa QWebSocket::open()
*/

/*!
    \brief Constructs an empty QWebSocketHandshakeOptions object.
*/
QWebSocketHandshakeOptions::QWebSocketHandshakeOptions()
    : d(new QWebSocketHandshakeOptionsPrivate)
{
}

/*!
    \brief Constructs a QWebSocketHandshakeOptions that is a copy of \a other.
*/
QWebSocketHandshakeOptions::QWebSocketHandshakeOptions(const QWebSocketHandshakeOptions &other)
    : d(other.d)
{
}

/*!
    \fn  QWebSocketHandshakeOptions::QWebSocketHandshakeOptions(QWebSocketHandshakeOptions &&other) noexcept
    \brief Constructs a QWebSocketHandshakeOptions that is moved from \a other.
*/

/*!
    \brief Destroys this object.
*/
QWebSocketHandshakeOptions::~QWebSocketHandshakeOptions()
{
}

/*!
    \fn QWebSocketHandshakeOptions &QWebSocketHandshakeOptions::operator=(QWebSocketHandshakeOptions &&other) noexcept
    \brief Moves \a other to this object.
*/

/*!
    \brief Assigns \a other to this object.
*/
QWebSocketHandshakeOptions &QWebSocketHandshakeOptions::operator=(
        const QWebSocketHandshakeOptions &other)
{
    d = other.d;
    return *this;
}

/*!
    \fn void swap(QWebSocketHandshakeOptions &other) noexcept
    \brief Swaps this object with \a other.
*/

/*!
    \brief Returns the list of WebSocket subprotocols to send along with the
           websocket handshake.
*/
QStringList QWebSocketHandshakeOptions::subprotocols() const
{
    return d->subprotocols;
}

/*!
    \brief Sets the list of WebSocket subprotocols \a protocols to send along
           with the websocket handshake.

    WebSocket subprotocol names may only consist of those US-ASCII characters
    that are in the unreserved group. Invalid protocol names will not be
    included in the handshake.
*/
void QWebSocketHandshakeOptions::setSubprotocols(const QStringList &protocols)
{
    d->subprotocols = protocols;
}

bool QWebSocketHandshakeOptions::equals(const QWebSocketHandshakeOptions &other) const
{
    return *d == *other.d;
}

/*!
    //! friend
    \fn QWebSocketHandshakeOptions::operator==(const QWebSocketHandshakeOptions &lhs, const QWebSocketHandshakeOptions &rhs)
    \fn QWebSocketHandshakeOptions::operator!=(const QWebSocketHandshakeOptions &lhs, const QWebSocketHandshakeOptions &rhs)
    \brief Compares \a lhs for equality with \a rhs.
*/

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QWebSocketHandshakeOptionsPrivate)

QT_END_NAMESPACE
