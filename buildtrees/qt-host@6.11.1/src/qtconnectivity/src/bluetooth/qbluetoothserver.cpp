// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserver.h"
#include "qbluetoothserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothserviceinfo.h"

QT_BEGIN_NAMESPACE

/*!
    \class QBluetoothServer
    \inmodule QtBluetooth
    \brief The QBluetoothServer class uses the RFCOMM or L2cap protocol to communicate with
    a Bluetooth device.

    \since 5.2

    QBluetoothServer is used to implement Bluetooth services over RFCOMM or L2cap.

    Start listening for incoming connections with listen(). Wait till the newConnection() signal
    is emitted when a new connection is established, and call nextPendingConnection() to get a QBluetoothSocket
    for the new connection.

    To enable other devices to find your service, create a QBluetoothServiceInfo with the
    applicable attributes for your service and register it using QBluetoothServiceInfo::registerService().
    Call serverPort() to get the channel number that is being used.

    If the \l QBluetoothServiceInfo::Protocol is not supported by a platform, \l listen() will return \c false.
    Android and WinRT only support RFCOMM for example.

    On iOS, this class cannot be used because the platform does not expose
    an API which may permit access to QBluetoothServer related features.

    \sa QBluetoothServiceInfo, QBluetoothSocket
*/

/*!
    \fn void QBluetoothServer::newConnection()

    This signal is emitted when a new connection is available.

    The connected slot should call nextPendingConnection() to get a QBluetoothSocket object to
    send and receive data over the connection.

    \sa nextPendingConnection(), hasPendingConnections()
*/

/*!
    \fn void QBluetoothServer::errorOccurred(QBluetoothServer::Error error)

    This signal is emitted when an \a error occurs.

    \sa error(), QBluetoothServer::Error
    \since 6.2
*/

/*!
    \fn void QBluetoothServer::close()

    Closes and resets the listening socket. Any already established \l QBluetoothSocket
    continues to operate and must be separately \l {QBluetoothSocket::close()}{closed}.
*/

/*!
    \enum QBluetoothServer::Error

    This enum describes Bluetooth server error types.

    \value NoError                  No error.
    \value UnknownError             An unknown error occurred.
    \value PoweredOffError          The Bluetooth adapter is powered off.
    \value InputOutputError         An input output error occurred.
    \value ServiceAlreadyRegisteredError  The service or port was already registered
    \value UnsupportedProtocolError The \l {QBluetoothServiceInfo::Protocol}{Protocol} is not
                                    supported on this platform.
    \value [since 6.4] MissingPermissionsError  The operating system requests
                                                permissions which were not
                                                granted by the user.
*/

/*!
    \fn bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)

    Start listening for incoming connections to \a address on \a port. \a address
    must be a local Bluetooth adapter address and \a port must be larger than zero
    and not be taken already by another Bluetooth server object. It is recommended
    to avoid setting a port number to enable the system to automatically choose
    a port.

    Returns \c true if the operation succeeded and the server is listening for
    incoming connections, otherwise returns \c false.

    If the server object is already listening for incoming connections this function
    always returns \c false. \l close() should be called before calling this function.

    \sa isListening(), newConnection()
*/

/*!
    \fn void QBluetoothServer::setMaxPendingConnections(int numConnections)

    Sets the maximum number of pending connections to \a numConnections. If
    the number of pending sockets exceeds this limit new sockets will be rejected.

    \sa maxPendingConnections()
*/

/*!
    \fn bool QBluetoothServer::hasPendingConnections() const
    Returns true if a connection is pending, otherwise false.
*/

/*!
    \fn QBluetoothSocket *QBluetoothServer::nextPendingConnection()

    Returns a pointer to the QBluetoothSocket for the next pending connection. It is the callers
    responsibility to delete the pointer.
*/

/*!
    \fn QBluetoothAddress QBluetoothServer::serverAddress() const

    Returns the server address.
*/

/*!
    \fn quint16 QBluetoothServer::serverPort() const

    Returns the server port number.
*/

/*!
    Constructs a bluetooth server with \a parent and \a serverType.
*/
QBluetoothServer::QBluetoothServer(QBluetoothServiceInfo::Protocol serverType, QObject *parent)
    : QObject(parent), d_ptr(new QBluetoothServerPrivate(serverType, this))
{
}

/*!
    Destroys the bluetooth server.
*/
QBluetoothServer::~QBluetoothServer()
{
    delete d_ptr;
}

/*!
    \fn QBluetoothServiceInfo QBluetoothServer::listen(const QBluetoothUuid &uuid, const QString &serviceName)

    Convenience function for registering an SPP service with \a uuid and \a serviceName.
    Because this function already registers the service, the QBluetoothServiceInfo object
    which is returned can not be changed any more. To shutdown the server later on it is
    required to call \l QBluetoothServiceInfo::unregisterService() and \l close() on this
    server object.

    Returns a registered QBluetoothServiceInfo instance if successful otherwise an
    invalid QBluetoothServiceInfo. This function always assumes that the default Bluetooth adapter
    should be used.

    If the server object is already listening for incoming connections this function
    returns an invalid \l QBluetoothServiceInfo.

    For an RFCOMM server this function is equivalent to following code snippet.

    \snippet qbluetoothserver.cpp listen
    \snippet qbluetoothserver.cpp listen2
    \snippet qbluetoothserver.cpp listen3

    \sa isListening(), newConnection(), listen()
*/
QBluetoothServiceInfo QBluetoothServer::listen(const QBluetoothUuid &uuid, const QString &serviceName)
{
    Q_D(const QBluetoothServer);
    if (!listen())
        return QBluetoothServiceInfo();
//! [listen]
    QBluetoothServiceInfo serviceInfo;
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, serviceName);
    QBluetoothServiceInfo::Sequence browseSequence;
    browseSequence << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             browseSequence);

    QBluetoothServiceInfo::Sequence profileSequence;
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    classId << QVariant::fromValue(quint16(0x100));
    profileSequence.append(QVariant::fromValue(classId));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             profileSequence);

    classId.clear();
    //Android requires custom uuid to be set as service class
    classId << QVariant::fromValue(uuid);
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo.setServiceUuid(uuid);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
    if (d->serverType == QBluetoothServiceInfo::L2capProtocol)
        protocol << QVariant::fromValue(serverPort());
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
//! [listen]
    if (d->serverType == QBluetoothServiceInfo::RfcommProtocol) {
//! [listen2]
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
             << QVariant::fromValue(quint8(serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
//! [listen2]
    }
//! [listen3]
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);
    bool result = serviceInfo.registerService();
//! [listen3]
    if (!result) {
        close(); //close the still listening socket
        return QBluetoothServiceInfo();
    }
    return serviceInfo;
}

/*!
    Returns true if the server is listening for incoming connections, otherwise false.
*/
bool QBluetoothServer::isListening() const
{
    Q_D(const QBluetoothServer);

#if defined(QT_ANDROID_BLUETOOTH) || defined(QT_WINRT_BLUETOOTH) || defined(QT_OSX_BLUETOOTH)
    return d->isListening();
#endif

    return d->socket->state() == QBluetoothSocket::SocketState::ListeningState;
}

/*!
    Returns the maximum number of pending connections.

    \sa setMaxPendingConnections()
*/
int QBluetoothServer::maxPendingConnections() const
{
    Q_D(const QBluetoothServer);

    return d->maxPendingConnections;
}

/*!
    \fn QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
    Sets the Bluetooth security flags to \a security. This function must be called
    before calling listen(). The Bluetooth link will always be encrypted when using
    Bluetooth 2.1 devices as encryption is mandatory.

    Android only supports two levels of security (secure and non-secure). If this flag
    is set to \l QBluetooth::Security::NoSecurity the server object will not employ
    any authentication or encryption. Any other security flag combination will
    trigger a secure Bluetooth connection.

    On \macos, security flags are not supported and will be ignored.
*/

/*!
    \fn QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
    Returns the Bluetooth security flags.
*/

/*!
    \fn QBluetoothSocket::ServerType QBluetoothServer::serverType() const
    Returns the type of the QBluetoothServer.
*/
QBluetoothServiceInfo::Protocol QBluetoothServer::serverType() const
{
    Q_D(const QBluetoothServer);
    return d->serverType;
}

/*!
    \fn QBluetoothServer::Error QBluetoothServer::error() const
    Returns the last error of the QBluetoothServer.
*/
QBluetoothServer::Error QBluetoothServer::error() const
{
    Q_D(const QBluetoothServer);
    return d->m_lastError;
}

QT_END_NAMESPACE

#include "moc_qbluetoothserver.cpp"
