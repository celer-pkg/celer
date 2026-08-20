// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapqudpconnection_p.h"
#include <private/qcoapnamespace_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qnetworkdatagram.h>
#include <QtNetwork/private/qabstractsocket_p.h>

#if QT_CONFIG(dtls)
#include <QtNetwork/QDtls>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslKey>
#endif

QT_BEGIN_NAMESPACE

/*!
    \internal

    \class QCoapQUdpConnection
    \inmodule QtCoap

    \brief The QCoapQUdpConnection class handles the transfer of frames to
    and from a server.

    \reentrant

    The QCoapQUdpConnection class is used by the QCoapClient class to send
    requests to a server. It has a socket listening for UDP messages,
    that is used to send the CoAP frames.

    When a reply is available, the QCoapQUdpConnection object emits a readyRead()
    signal.

    \sa QCoapClient
*/

/*!
    Constructs a new QCoapQUdpConnection for the given \a securityMode and
    sets \a parent as the parent object.

    \note Since QtCoap::RawPublicKey is not supported yet, the connection
    will fall back to the QtCoap::NoSecurity in the QtCoap::RawPublicKey mode.
    That is, the connection won't be secure in this mode.
*/
QCoapQUdpConnection::QCoapQUdpConnection(QtCoap::SecurityMode securityMode, QObject *parent) :
    QCoapQUdpConnection(*new QCoapQUdpConnectionPrivate(securityMode), parent)
{
}

/*!
    \internal

    Constructs a new QCoapQUdpConnection as a child of \a parent, with \a dd as
    its \c d_ptr. This constructor must be used when internally subclassing
    the QCoapQUdpConnection class.
*/
QCoapQUdpConnection::QCoapQUdpConnection(QCoapQUdpConnectionPrivate &dd, QObject *parent) :
    QCoapConnection(dd, parent)
{
    Q_D(QCoapQUdpConnection);

    createSocket();

    if (isSecure()) {
#if QT_CONFIG(dtls)
        connect(this, &QCoapConnection::securityConfigurationChanged, this,
                [this]() {
                       Q_D(QCoapQUdpConnection);
                       d->setSecurityConfiguration(securityConfiguration());
                });

        auto configuration = QSslConfiguration::defaultDtlsConfiguration();

        switch (d->securityMode) {
        case QtCoap::SecurityMode::RawPublicKey:
            qCWarning(lcCoapConnection, "RawPublicKey security is not supported yet,"
                                        "disabling security");
            d->securityMode = QtCoap::SecurityMode::NoSecurity;
            break;
        case QtCoap::SecurityMode::PreSharedKey:
            d->dtls = new QDtls(QSslSocket::SslClientMode, this);
            configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
            d->dtls->setDtlsConfiguration(configuration);

            connect(d->dtls.data(), &QDtls::pskRequired, this, &QCoapQUdpConnection::pskRequired);
            connect(d->dtls.data(), &QDtls::handshakeTimeout,
                    this, &QCoapQUdpConnection::handshakeTimeout);
            break;
        case QtCoap::SecurityMode::Certificate:
            d->dtls = new QDtls(QSslSocket::SslClientMode, this);
            configuration.setPeerVerifyMode(QSslSocket::VerifyPeer);
            d->dtls->setDtlsConfiguration(configuration);

            connect(d->dtls.data(), &QDtls::handshakeTimeout,
                    this, &QCoapQUdpConnection::handshakeTimeout);
            break;
        default:
            break;
        }
#else
        qCWarning(lcCoapConnection, "DTLS is disabled, falling back to QtCoap::NoSecurity mode.");
        d->securityMode = QtCoap::SecurityMode::NoSecurity;
#endif
    }
}

/*!
    \internal

    Creates the socket used by the connection and sets it in connection's private
    class.
*/
void QCoapQUdpConnection::createSocket()
{
    Q_D(QCoapQUdpConnection);

    d->udpSocket = new QUdpSocket(this);

    connect(d->udpSocket.data(), &QUdpSocket::readyRead, this, [this]() {
        Q_D(QCoapQUdpConnection);
        d->socketReadyRead();
    });
    connect(d->udpSocket.data(), &QUdpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError socketError) {
                    qCWarning(lcCoapConnection) << "CoAP UDP socket error" << socketError
                                                << socket()->errorString();
                    emit error(socketError);
            });
}

QCoapQUdpConnectionPrivate::QCoapQUdpConnectionPrivate(QtCoap::SecurityMode security)
    : QCoapConnectionPrivate(security)
#if QT_CONFIG(dtls)
    , dtls(nullptr)
#endif
    , udpSocket(nullptr)
{
}

QCoapQUdpConnectionPrivate::~QCoapQUdpConnectionPrivate()
{
#if QT_CONFIG(dtls)
    if (dtls && dtls->isConnectionEncrypted()) {
        Q_ASSERT(udpSocket);
        dtls->shutdown(udpSocket);
    }
#endif
}

/*!
    \internal

    Binds the socket to a random port and returns \c true if it succeeds.
*/
bool QCoapQUdpConnectionPrivate::bind()
{
#if QT_CONFIG(networkinterface)
    // Using the QAbstractSocketPrivate private API to bind to a specific
    // network interface.
    auto priv = static_cast<QAbstractSocketPrivate *>(QObjectPrivate::get(socket()));
    QNetworkInterface *iface = udpNetworkInterface.isValid() ? &udpNetworkInterface : nullptr;
    return priv->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress, iface);
#else
    return socket()->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress);
#endif
}

/*!
    \internal

    Binds the socket if it is not already done and emits the bound() signal when
    the socket is ready.
*/
void QCoapQUdpConnectionPrivate::bindSocket()
{
    Q_Q(QCoapQUdpConnection);

    if (state != QCoapQUdpConnection::ConnectionState::Bound && bind())
        emit q->bound();
}

/*!
    \internal

    Prepares the socket for data transmission to the given \a host and
    \a port by binding the socket. In case of a secure connection also
    starts the handshake with the server.
    Emits the bound() signal when the transport is ready.
*/
void QCoapQUdpConnection::bind(const QString &host, quint16 port)
{
    Q_D(QCoapQUdpConnection);

    if (!isSecure()) {
        d->bindSocket();
#if QT_CONFIG(dtls)
    } else {
        Q_ASSERT(d->dtls);
        if (d->dtls->isConnectionEncrypted()) {
            emit bound();
        } else if (socket()->state() == QAbstractSocket::UnconnectedState) {
            d->bind();
            d->dtls->setPeer(QHostAddress(host), port);
            if (!d->dtls->doHandshake(d->socket()))
                qCWarning(lcCoapConnection) << "Handshake error: " << d->dtls->dtlsErrorString();
        }
#else
        Q_UNUSED(host);
        Q_UNUSED(port);
#endif
    }
}

/*!
    \internal

    Sends the given \a data frame to the \a host at the \a port.
*/
void QCoapQUdpConnection::writeData(const QByteArray &data, const QString &host, quint16 port)
{
    Q_D(QCoapQUdpConnection);
    d->writeToSocket(data, host, port);
}

/*!
    \internal

    \brief Close the UDP socket

    In the case of a secure connection, this also interrupts any ongoing
    hand-shake and shuts down the DTLS connection.
*/
void QCoapQUdpConnection::close()
{
    Q_D(QCoapQUdpConnection);

#if QT_CONFIG(dtls)
    if (isSecure()) {
        if (d->dtls->handshakeState() == QDtls::HandshakeInProgress)
            d->dtls->abortHandshake(d->socket());

        if (d->dtls->isConnectionEncrypted())
            d->dtls->shutdown(d->socket());
    }
#endif
    d->socket()->close();
}

/*!
    \internal

    Sets the QUdpSocket socket \a option to \a value.
*/
void QCoapQUdpConnection::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
    Q_D(QCoapQUdpConnection);
    d->socket()->setSocketOption(option, value);
}

/*!
    \internal

    Sends the given \a data frame to the \a host at the \a port.
*/
void QCoapQUdpConnectionPrivate::writeToSocket(const QByteArray &data, const QString &host, quint16 port)
{
#if QT_CONFIG(dtls)
    Q_Q(QCoapQUdpConnection);
#endif
    if (!socket()->isWritable()) {
        bool opened = socket()->open(socket()->openMode() | QIODevice::WriteOnly);
        if (!opened) {
            qCWarning(lcCoapConnection, "Failed to open the UDP socket with write permission");
            return;
        }
    }

    QHostAddress hostAddress(host);
    if (hostAddress.isNull()) {
        qCWarning(lcCoapConnection) << "Invalid host IP address" << host
                                    << "- only IPv4/IPv6 destination addresses are supported.";
        return;
    }

    const qint64 bytesWritten =
#if QT_CONFIG(dtls)
            q->isSecure() ? (Q_ASSERT(dtls), dtls->writeDatagramEncrypted(socket(), data)) :
#endif
            socket()->writeDatagram(data, hostAddress, port);

    if (bytesWritten < 0)
        qCWarning(lcCoapConnection) << "Failed to write datagram:" << socket()->errorString();
}

/*!
    \internal

    This slot reads all data stored in the socket and emits a readyRead()
    signal for each received datagram.
*/
void QCoapQUdpConnectionPrivate::socketReadyRead()
{
    Q_Q(QCoapQUdpConnection);

    if (!socket()->isReadable()) {
        bool opened = socket()->open(socket()->openMode() | QIODevice::ReadOnly);
        if (!opened) {
            qCWarning(lcCoapConnection, "Failed to open the UDP socket with read permission");
            return;
        }
    }

    while (socket()->hasPendingDatagrams()) {
        if (!q->isSecure()) {
            const auto &datagram = socket()->receiveDatagram();
            if (datagramMatchesNetworkInterface(datagram))
                emit q->readyRead(datagram.data(), datagram.senderAddress());
#if QT_CONFIG(dtls)
        } else {
            handleEncryptedDatagram();
#endif
        }
    }
}

/*!
    \internal

    Returns the socket.
*/
QUdpSocket *QCoapQUdpConnection::socket() const
{
    Q_D(const QCoapQUdpConnection);
    return d->udpSocket;
}

/*!
    \internal

    Sets the DTLS configuration.
*/
void QCoapQUdpConnectionPrivate::setSecurityConfiguration(
        const QCoapSecurityConfiguration &configuration)
{
#if QT_CONFIG(dtls)
    Q_ASSERT(dtls);
    auto dtlsConfig = dtls->dtlsConfiguration();

    if (!configuration.defaultCipherString().isEmpty()) {
        dtlsConfig.setBackendConfigurationOption("CipherString",
                                                 configuration.defaultCipherString());
    }

    if (!configuration.caCertificates().isEmpty())
        dtlsConfig.setCaCertificates(configuration.caCertificates().toList());

    if (!configuration.localCertificateChain().isEmpty())
        dtlsConfig.setLocalCertificateChain(configuration.localCertificateChain().toList());

    if (!configuration.privateKey().isNull()) {
        if (configuration.privateKey().algorithm() != QSsl::Opaque) {
            QSslKey privateKey(configuration.privateKey().key(),
                               configuration.privateKey().algorithm(),
                               configuration.privateKey().encodingFormat(),
                               QSsl::PrivateKey,
                               configuration.privateKey().passPhrase());
            dtlsConfig.setPrivateKey(privateKey);
        } else if (configuration.privateKey().handle()) {
            QSslKey opaqueKey(configuration.privateKey().handle());
            dtlsConfig.setPrivateKey(opaqueKey);
        } else {
            qCWarning(lcCoapConnection, "Failed to set private key, the provided key is invalid");
        }
    }

    dtls->setDtlsConfiguration(dtlsConfig);
#else
    Q_UNUSED(configuration);
#endif
}

/*!
    \internal

    Checks if the datagram was received on a specified network interface
*/
bool QCoapQUdpConnectionPrivate::datagramMatchesNetworkInterface(
        const QNetworkDatagram &datagram) const
{
#if QT_CONFIG(networkinterface)
    return !udpNetworkInterface.isValid()
            || uint(udpNetworkInterface.index()) == datagram.interfaceIndex();
#else
    Q_UNUSED(datagram);
    return true;
#endif
}

#if QT_CONFIG(dtls)
/*!
    \internal

    This slot is invoked when PSK authentication is required. It is used
    for setting the identity and pre shared key in order for the TLS handshake
    to complete.
*/
void QCoapQUdpConnection::pskRequired(QSslPreSharedKeyAuthenticator *authenticator)
{
    Q_ASSERT(authenticator);
    authenticator->setIdentity(securityConfiguration().preSharedKeyIdentity());
    authenticator->setPreSharedKey(securityConfiguration().preSharedKey());
}

/*!
    \internal

    This slot handles handshake timeouts.
*/
void QCoapQUdpConnection::handshakeTimeout()
{
    Q_D(QCoapQUdpConnection);

    qCWarning(lcCoapConnection, "Handshake timeout, trying to re-transmit");
    if (d->dtls->handshakeState() == QDtls::HandshakeInProgress &&
            !d->dtls->handleTimeout(d->udpSocket)) {
        qCWarning(lcCoapConnection) << "Failed to re-transmit" << d->dtls->dtlsErrorString();
    }
}

/*!
    \internal

    Returns a decrypted datagram received from a UDP socket or a std::nullopt
    if the datagram was received on an incorrect network interface.
*/
std::optional<QNetworkDatagram> QCoapQUdpConnectionPrivate::receiveDatagramDecrypted() const
{
    auto datagram = socket()->receiveDatagram();
    if (!datagramMatchesNetworkInterface(datagram))
        return std::nullopt;

    const QByteArray plainText = dtls->decryptDatagram(socket(), datagram.data());
    datagram.setData(plainText);
    return datagram;
}

/*!
    \internal

    If the connection is encrypted, emits the readyRead() signal for
    the pending datagram. Otherwise starts the DTLS handshake and
    if it is completed, starts to send the pending request.
*/
void QCoapQUdpConnectionPrivate::handleEncryptedDatagram()
{
    Q_Q(QCoapQUdpConnection);

    Q_ASSERT(dtls);

    if (dtls->isConnectionEncrypted()) {
        const auto datagram = receiveDatagramDecrypted();
        if (datagram)
            emit q->readyRead(datagram->data(), datagram->senderAddress());
    } else {
        const auto datagram = socket()->receiveDatagram();
        if (!datagramMatchesNetworkInterface(datagram))
            return;

        if (!dtls->doHandshake(socket(), datagram.data())) {
            qCWarning(lcCoapConnection) << "Handshake error: " << dtls->dtlsErrorString();
            return;
        }

        // The handshake is successfully done, emit the bound() signal to inform
        // listeners that the socket is ready to start transferring data.
        if (dtls->isConnectionEncrypted())
            emit q->bound();
    }
}

#endif // dtls

QT_END_NAMESPACE
