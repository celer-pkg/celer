// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebchannel.h"
#include "qwebchannel_p.h"
#include "qmetaobjectpublisher_p.h"
#include "qwebchannelabstracttransport.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

QT_BEGIN_NAMESPACE

/*!
    \class QWebChannel

    \inmodule QtWebChannel
    \brief Exposes QObjects to remote HTML clients.
    \since 5.4

    The QWebChannel fills the gap between C++ applications and HTML/JavaScript
    applications. By publishing a QObject derived object to a QWebChannel and
    using the \l{Qt WebChannel JavaScript API}{qwebchannel.js} on the HTML side, one can transparently access
    properties and public slots and methods of the QObject. No manual message
    passing and serialization of data is required, property updates and signal emission
    on the C++ side get automatically transmitted to the potentially remotely running HTML clients.
    On the client side, a JavaScript object will be created for any published C++ QObject. It mirrors the
    C++ object's API and thus is intuitively useable.

    QWebChannel transparently supports QFuture. When a client calls a method that returns a QFuture,
    QWebChannel will send a response with the QFuture result only after the QFuture has finished.

    Custom conversion of types to and from JSON is supported by defining converters with
    QMetaType::registerConverter() to and from QJsonValue. Note that custom converters from QJsonValue to a concrete
    type must fail if the QJsonValue does not match the expected format. Otherwise QWebChannel cannot fall back to its
    default conversion mechanisms.
    Custom converters are also available on \l{Qt WebChannel JavaScript API}{the JavaScript side}.

    The C++ QWebChannel API makes it possible to talk to any HTML client, which could run on a local
    or even remote machine. The only limitation is that the HTML client supports the JavaScript
    features used by \c{qwebchannel.js}. As such, one can interact
    with basically any modern HTML browser or standalone JavaScript runtime, such as node.js.

    There also exists a declarative \l{Qt WebChannel QML Types}{WebChannel API}.

    \sa {Qt WebChannel Standalone Example}, {Qt WebChannel JavaScript API}{JavaScript API},
        QMetaType::registerConverter()
*/

/*!
    \internal

    Remove a destroyed transport object from the list of known transports.
*/
void QWebChannelPrivate::_q_transportDestroyed(QObject *object)
{
    auto it = std::find(transports.begin(), transports.end(), object);
    if (it != transports.end()) {
        auto *transport = *it;
        transports.erase(it);
        publisher->transportRemoved(transport);
    }
}

/*!
    \internal

    Shared code to initialize the QWebChannel from both constructors.
*/
void QWebChannelPrivate::init()
{
    Q_Q(QWebChannel);
    publisher = new QMetaObjectPublisher(q);
    QObject::connect(publisher, SIGNAL(blockUpdatesChanged(bool)),
                     q, SIGNAL(blockUpdatesChanged(bool)));
}

/*!
    Constructs the QWebChannel object with the given \a parent.

    Note that a QWebChannel is only fully operational once you connect it to a
    QWebChannelAbstractTransport. The HTML clients also need to be setup appropriately
    using \l{qtwebchannel-javascript.html}{\c qwebchannel.js}.
*/
QWebChannel::QWebChannel(QObject *parent)
: QObject(*(new QWebChannelPrivate), parent)
{
    Q_D(QWebChannel);
    d->init();
}

/*!
    \internal

    Construct a QWebChannel from an ancestor class with the given \a parent.

    \sa QQmlWebChannel
*/
QWebChannel::QWebChannel(QWebChannelPrivate &dd, QObject *parent)
: QObject(dd, parent)
{
    Q_D(QWebChannel);
    d->init();
}

/*!
    Destroys the QWebChannel.
*/
QWebChannel::~QWebChannel()
{
}

/*!
    Registers a group of objects to the QWebChannel.

    The properties, signals and public invokable methods of the objects are published to the remote clients.
    There, an object with the identifier used as key in the \a objects map is then constructed.

    \note A current limitation is that objects must be registered before any client is initialized.

    \sa QWebChannel::registerObject(), QWebChannel::deregisterObject(), QWebChannel::registeredObjects()
*/
void QWebChannel::registerObjects(const QHash< QString, QObject * > &objects)
{
    Q_D(QWebChannel);
    const QHash<QString, QObject *>::const_iterator end = objects.constEnd();
    for (QHash<QString, QObject *>::const_iterator it = objects.constBegin(); it != end; ++it) {
        d->publisher->registerObject(it.key(), it.value());
    }
}

/*!
    Returns the map of registered objects that are published to remote clients.

    \sa QWebChannel::registerObjects(), QWebChannel::registerObject(), QWebChannel::deregisterObject()
*/
QHash<QString, QObject *> QWebChannel::registeredObjects() const
{
    Q_D(const QWebChannel);
    return d->publisher->registeredObjects;
}

/*!
    \qmlmethod void WebChannel::registerObject(const string id, QtObject object)

    Registers a single object to the WebChannel.

    The properties, signals and public methods of the \a object are published to the remote clients.
    There, an object with the identifier \a id is then constructed.

    A property that is \c BINDABLE but does not have a \c NOTIFY signal will have working property
    updates on the client side, but no mechanism to register a callback for the change notifications.

    \note A current limitation is that objects must be registered before any client is initialized.

    \sa registerObjects(), deregisterObject(), registeredObjects
*/

/*!
    Registers a single object to the QWebChannel.

    The properties, signals and public methods of the \a object are published to the remote clients.
    There, an object with the identifier \a id is then constructed.

    A property that is \c BINDABLE but does not have a \c NOTIFY signal will have working property
    updates on the client side, but no mechanism to register a callback for the change notifications.

    \note A current limitation is that objects must be registered before any client is initialized.

    \sa QWebChannel::registerObjects(), QWebChannel::deregisterObject(), QWebChannel::registeredObjects()
*/
void QWebChannel::registerObject(const QString &id, QObject *object)
{
    Q_D(QWebChannel);
    d->publisher->registerObject(id, object);
}

/*!
    \qmlmethod void WebChannel::deregisterObject(QtObject object)

    Deregisters the given \a object from the WebChannel.

    Remote clients will receive a \c destroyed signal for the given object.

    \sa registerObjects(), registerObject(), registeredObjects
*/

/*!
    Deregisters the given \a object from the QWebChannel.

    Remote clients will receive a \c destroyed signal for the given object.

    \sa QWebChannel::registerObjects(), QWebChannel::registerObject(), QWebChannel::registeredObjects()
*/
void QWebChannel::deregisterObject(QObject *object)
{
    Q_D(QWebChannel);
    // handling of deregistration is analogously to handling of a destroyed signal
    d->publisher->signalEmitted(object, s_destroyedSignalIndex, QVariantList() << QVariant::fromValue(object));
}

/*!
    \property QWebChannel::blockUpdates

    \brief When set to \c true, updates are blocked and remote clients will not be notified about property changes.

    The changes are recorded and sent to the clients once updates become unblocked again by setting
    this property to \c false. By default, updates are not blocked.
*/

/*!
    \qmlproperty bool WebChannel::blockUpdates

    \brief When set to \c true, updates are blocked and remote clients will not be notified about property changes.

    The changes are recorded and sent to the clients once updates become unblocked again by setting
    this property to \c false. By default, updates are not blocked.
*/
bool QWebChannel::blockUpdates() const
{
    Q_D(const QWebChannel);
    return d->publisher->blockUpdates();
}

void QWebChannel::setBlockUpdates(bool block)
{
    Q_D(QWebChannel);
    d->publisher->setBlockUpdates(block);
}

QBindable<bool> QWebChannel::bindableBlockUpdates()
{
    Q_D(QWebChannel);
    return &d->publisher->blockUpdatesStatus;
}

/*!
    \property QWebChannel::propertyUpdateInterval

    \brief The property update interval.

    This interval can be changed to a different interval in milliseconds by
    setting it to a positive value. Property updates are batched and sent out
    after the interval expires. If set to zero, the updates occurring within a
    single event loop run are batched and sent out on the next run.
    If negative, updates will be sent immediately.
    Default value is 50 milliseconds.
*/

/*!
    \qmlproperty int WebChannel::propertyUpdateInterval

    \brief The property update interval.

    This interval can be changed to a different interval in milliseconds by
    setting it to a positive value. Property updates are batched and sent out
    after the interval expires. If set to zero, the updates occurring within a
    single event loop run are batched and sent out on the next run.
    If negative, updates will be sent immediately.
    Default value is 50 milliseconds.
*/
int QWebChannel::propertyUpdateInterval() const
{
    Q_D(const QWebChannel);
    return d->publisher->propertyUpdateInterval();
}

void QWebChannel::setPropertyUpdateInterval(int ms)
{
    Q_D(QWebChannel);
    d->publisher->setPropertyUpdateInterval(ms);
}

QBindable<int> QWebChannel::bindablePropertyUpdateInterval()
{
    Q_D(QWebChannel);
    return &d->publisher->propertyUpdateIntervalTime;
}

/*!
    Connects the QWebChannel to the given \a transport object.

    The transport object then handles the communication between the C++ application and a remote
    HTML client.

    \sa QWebChannelAbstractTransport, QWebChannel::disconnectFrom()
*/
void QWebChannel::connectTo(QWebChannelAbstractTransport *transport)
{
    Q_D(QWebChannel);
    Q_ASSERT(transport);
    if (!d->transports.contains(transport)) {
        d->transports << transport;
        connect(transport, &QWebChannelAbstractTransport::messageReceived,
                d->publisher, &QMetaObjectPublisher::handleMessage,
                Qt::UniqueConnection);
        connect(transport, SIGNAL(destroyed(QObject*)),
                this, SLOT(_q_transportDestroyed(QObject*)));
    }
}

/*!
    Disconnects the QWebChannel from the \a transport object.

    \sa QWebChannel::connectTo()
*/
void QWebChannel::disconnectFrom(QWebChannelAbstractTransport *transport)
{
    Q_D(QWebChannel);
    const int idx = d->transports.indexOf(transport);
    if (idx != -1) {
        disconnect(transport, 0, this, 0);
        disconnect(transport, 0, d->publisher, 0);
        d->transports.remove(idx);
        d->publisher->transportRemoved(transport);
    }
}

QT_END_NAMESPACE

#include "moc_qwebchannel.cpp"
