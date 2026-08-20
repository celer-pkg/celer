// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebchannelabstracttransport.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebChannelAbstractTransport

    \inmodule QtWebChannel
    \brief Communication channel between the C++ QWebChannel server and a HTML/JS client.
    \since 5.4

    Users of the QWebChannel must implement this interface and connect instances of it
    to the QWebChannel server for every client that should be connected to the QWebChannel.
    The \l{Qt WebChannel Standalone Example} shows how this can be done
    using \l{Qt WebSockets}.

    \note The JSON message protocol is considered internal and might change over time.

    \sa {Qt WebChannel Standalone Example}
*/

/*!
    \fn QWebChannelAbstractTransport::messageReceived(const QJsonObject &message, QWebChannelAbstractTransport *transport)

    This signal must be emitted when a new JSON \a message was received from the remote client. The
    \a transport argument should be set to this transport object.
*/

/*!
    \fn QWebChannelAbstractTransport::sendMessage(const QJsonObject &message)

    Sends a JSON \a message to the remote client. An implementation would serialize the message and
    transmit it to the remote JavaScript client.
*/

/*!
    Constructs a transport object with the given \a parent.
*/
QWebChannelAbstractTransport::QWebChannelAbstractTransport(QObject *parent)
: QObject(parent)
{

}

/*!
    Destroys the transport object.
*/
QWebChannelAbstractTransport::~QWebChannelAbstractTransport()
{

}

QT_END_NAMESPACE
