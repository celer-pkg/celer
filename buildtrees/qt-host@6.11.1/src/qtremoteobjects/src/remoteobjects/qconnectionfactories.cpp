// Copyright (C) 2017-2015 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser-and-backend-provider

#include "qconnectionfactories_p.h"
#include "qremoteobjectpacket_p.h"

// BEGIN: Backends
#if defined(Q_OS_QNX)
#include "qconnection_qnx_backend_p.h"
#endif
#include "qconnection_local_backend_p.h"
#include "qconnection_tcpip_backend_p.h"
// END: Backends

QT_BEGIN_NAMESPACE

using namespace QtRemoteObjects;

class QtROFactoryLoader
{
public:
    QtROClientFactory clientFactory;
    QtROServerFactory serverFactory;
};

Q_GLOBAL_STATIC(QtROFactoryLoader, loader)

inline bool fromDataStream(QDataStream &in, QRemoteObjectPacketTypeEnum &type, QString &name)
{
    quint16 _type;
    in >> _type;
    type = Invalid;
    switch (_type) {
    case Handshake: type = Handshake; break;
    case InitPacket: type = InitPacket; break;
    case InitDynamicPacket: type = InitDynamicPacket; break;
    case AddObject: type = AddObject; break;
    case RemoveObject: type = RemoveObject; break;
    case InvokePacket: type = InvokePacket; break;
    case InvokeReplyPacket: type = InvokeReplyPacket; break;
    case PropertyChangePacket: type = PropertyChangePacket; break;
    case ObjectList: type = ObjectList; break;
    case Ping: type = Ping; break;
    case Pong: type = Pong; break;
    default:
        qCWarning(QT_REMOTEOBJECT_IO) << "Invalid packet received" << _type;
    }
    if (type == Invalid)
        return false;
    if (type == ObjectList)
        return true;
    in >> name;
    qCDebug(QT_REMOTEOBJECT_IO) << "Packet received of type" << type << "for object" << name;
    return true;
}

/*!
    All communication between nodes happens through some form of QIODevice with
    an associated QDataStream to handle marshalling of Qt types. QtROIoDeviceBase
    is an abstract base class that provides a consistent interface to QtRO, yet
    can be extended to support different types of QIODevice.
 */
QtROIoDeviceBase::QtROIoDeviceBase(QObject *parent) : QObject(*new QtROIoDeviceBasePrivate, parent) { }

QtROIoDeviceBase::QtROIoDeviceBase(QtROIoDeviceBasePrivate &dptr, QObject *parent) : QObject(dptr, parent) { }

QtROIoDeviceBase::~QtROIoDeviceBase()
{
}

bool QtROIoDeviceBase::read(QRemoteObjectPacketTypeEnum &type, QString &name)
{
    Q_D(QtROIoDeviceBase);
    qCDebug(QT_REMOTEOBJECT_IO) << deviceType() << "read()" << d->m_curReadSize << bytesAvailable();

    if (d->m_curReadSize == 0) {
        if (bytesAvailable() < static_cast<int>(sizeof(quint32)))
            return false;

        d->m_dataStream >> d->m_curReadSize;
    }

    qCDebug(QT_REMOTEOBJECT_IO) << deviceType() << "read()-looking for map" << d->m_curReadSize
                                << bytesAvailable();

    if (bytesAvailable() < d->m_curReadSize)
        return false;

    d->m_curReadSize = 0;
    return fromDataStream(d->m_dataStream, type, name);
}

void QtROIoDeviceBase::write(const QByteArray &data)
{
    Q_D(QtROIoDeviceBase);
    if (connection()->isOpen() && !d->m_isClosing)
        connection()->write(data);
}

void QtROIoDeviceBase::write(const QByteArray &data, qint64 size)
{
    Q_D(QtROIoDeviceBase);
    if (connection()->isOpen() && !d->m_isClosing)
        connection()->write(data.data(), size);
}

bool QtROIoDeviceBase::isOpen() const
{
    return !isClosing();
}

void QtROIoDeviceBase::close()
{
    Q_D(QtROIoDeviceBase);
    d->m_isClosing = true;
    doClose();
}

qint64 QtROIoDeviceBase::bytesAvailable() const
{
    return connection()->bytesAvailable();
}

void QtROIoDeviceBase::initializeDataStream()
{
    Q_D(QtROIoDeviceBase);
    d->m_dataStream.setDevice(connection());
    d->m_dataStream.resetStatus();
}

bool QtROIoDeviceBase::isClosing() const
{
    Q_D(const QtROIoDeviceBase);
    return d->m_isClosing;
}

void QtROIoDeviceBase::addSource(const QString &name)
{
    Q_D(QtROIoDeviceBase);
    d->m_remoteObjects.insert(name);
}

void QtROIoDeviceBase::removeSource(const QString &name)
{
    Q_D(QtROIoDeviceBase);
    d->m_remoteObjects.remove(name);
}

QSet<QString> QtROIoDeviceBase::remoteObjects() const
{
    Q_D(const QtROIoDeviceBase);
    return d->m_remoteObjects;
}

QtROClientIoDevice::QtROClientIoDevice(QObject *parent) : QtROIoDeviceBase(*new QtROClientIoDevicePrivate, parent)
{
}

QtROClientIoDevice::~QtROClientIoDevice()
{
    if (!isClosing())
        close();
}

void QtROClientIoDevice::disconnectFromServer()
{
    doDisconnectFromServer();
    emit shouldReconnect(this);
}

QUrl QtROClientIoDevice::url() const
{
    Q_D(const QtROClientIoDevice);
    return d->m_url;
}

QString QtROClientIoDevice::deviceType() const
{
    return QStringLiteral("QtROClientIoDevice");
}

void QtROClientIoDevice::setUrl(const QUrl &url)
{
    Q_D(QtROClientIoDevice);
    d->m_url = url;
}

/*!
    The Qt servers create QIODevice derived classes from handleConnection. The
    problem is that they behave differently, so this class adds some
    consistency.
 */
QtROServerIoDevice::QtROServerIoDevice(QObject *parent) : QtROIoDeviceBase(parent)
{
}

QString QtROServerIoDevice::deviceType() const
{
    return QStringLiteral("QtROServerIoDevice");
}

QConnectionAbstractServer::QConnectionAbstractServer(QObject *parent)
    : QObject(parent)
{
}

QConnectionAbstractServer::~QConnectionAbstractServer()
{
}

QtROServerIoDevice *QConnectionAbstractServer::nextPendingConnection()
{
    QtROServerIoDevice *iodevice = configureNewConnection();
    iodevice->initializeDataStream();
    return iodevice;
}

QtROExternalIoDevice::QtROExternalIoDevice(QIODevice *device, QObject *parent)
    : QtROIoDeviceBase(*new QtROExternalIoDevicePrivate(device), parent)
{
    Q_D(QtROExternalIoDevice);
    initializeDataStream();
    connect(device, &QIODevice::aboutToClose, this, [d]() { d->m_isClosing = true; });
    connect(device, &QIODevice::readyRead, this, &QtROExternalIoDevice::readyRead);
    auto meta = device->metaObject();
    if (-1 != meta->indexOfSignal("disconnected()"))
        connect(device, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

QIODevice *QtROExternalIoDevice::connection() const
{
    Q_D(const QtROExternalIoDevice);
    return d->m_device;
}

bool QtROExternalIoDevice::isOpen() const
{
    Q_D(const QtROExternalIoDevice);
    if (!d->m_device)
        return false;
    return d->m_device->isOpen() && QtROIoDeviceBase::isOpen();
}

void QtROExternalIoDevice::doClose()
{
    Q_D(QtROExternalIoDevice);
    if (isOpen())
        d->m_device->close();
}

QString QtROExternalIoDevice::deviceType() const
{
    return QStringLiteral("QtROExternalIoDevice");
}

/*!
    \class QtROServerFactory
    \inmodule QtRemoteObjects
    \brief A class that holds information about server backends available on the Qt Remote Objects network.
*/
QtROServerFactory::QtROServerFactory()
{
#ifdef Q_OS_QNX
    registerType<QnxServerImpl>(QStringLiteral("qnx"));
#endif
#ifdef Q_OS_LINUX
    registerType<AbstractLocalServerImpl>(QStringLiteral("localabstract"));
#endif
    registerType<LocalServerImpl>(QStringLiteral("local"));
    registerType<TcpServerImpl>(QStringLiteral("tcp"));
}

QtROServerFactory *QtROServerFactory::instance()
{
    return &loader->serverFactory;
}

/*!
    \class QtROClientFactory
    \inmodule QtRemoteObjects
    \brief A class that holds information about client backends available on the Qt Remote Objects network.
*/
QtROClientFactory::QtROClientFactory()
{
#ifdef Q_OS_QNX
    registerType<QnxClientIo>(QStringLiteral("qnx"));
#endif
#ifdef Q_OS_LINUX
    registerType<AbstractLocalClientIo>(QStringLiteral("localabstract"));
#endif
    registerType<LocalClientIo>(QStringLiteral("local"));
    registerType<TcpClientIo>(QStringLiteral("tcp"));
}

QtROClientFactory *QtROClientFactory::instance()
{
    return &loader->clientFactory;
}

/*!
    \fn template <typename T> void qRegisterRemoteObjectsClient(const QString &id)
    \relates QtROClientFactory

    Registers the Remote Objects client \a id for the type \c{T}.

    If you need a custom transport protocol for Qt Remote Objects, you need to
    register the client & server implementation here.

    \note This function requires that \c{T} is a fully defined type at the point
    where the function is called.

    This example registers the class \c{CustomClientIo} as \c{"myprotocol"}:

    \code
        qRegisterRemoteObjectsClient<CustomClientIo>(QStringLiteral("myprotocol"));
    \endcode

    With this in place, you can now instantiate nodes using this new custom protocol:

    \code
        QRemoteObjectNode client(QUrl(QStringLiteral("myprotocol:registry")));
    \endcode

    \sa {qRegisterRemoteObjectsServer}
*/

/*!
    \fn template <typename T> void qRegisterRemoteObjectsServer(const QString &id)
    \relates QtROServerFactory

    Registers the Remote Objects server \a id for the type \c{T}.

    If you need a custom transport protocol for Qt Remote Objects, you need to
    register the client & server implementation here.

    \note This function requires that \c{T} is a fully defined type at the point
    where the function is called.

    This example registers the class \c{CustomServerImpl} as \c{"myprotocol"}:

    \code
        qRegisterRemoteObjectsServer<CustomServerImpl>(QStringLiteral("myprotocol"));
    \endcode

    With this in place, you can now instantiate nodes using this new custom protocol:

    \code
        QRemoteObjectNode client(QUrl(QStringLiteral("myprotocol:registry")));
    \endcode

    \sa {qRegisterRemoteObjectsClient}
*/

QtROIoDeviceBasePrivate::QtROIoDeviceBasePrivate() : QObjectPrivate()
{
    m_dataStream.setVersion(dataStreamVersion);
    m_dataStream.setByteOrder(QDataStream::LittleEndian);
}

QT_END_NAMESPACE
