// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author
// Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qqmlwebchannel.h"
#include <QtWebChannel/qwebchannelabstracttransport.h>
#include <QtWebChannel/private/qwebchannel_p.h>
#include <QtWebChannel/private/qmetaobjectpublisher_p.h>
#include <QtQml/QQmlContext>

#include "qqmlwebchannelattached_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebChannel
    \nativetype QQmlWebChannel

    \inqmlmodule QtWebChannel
    \ingroup webchannel-qml
    \brief QML interface to QWebChannel.
    \since 5.4

    The WebChannel provides a mechanism to transparently access QObject or QML objects from HTML
    clients. All properties, signals and public slots can be used from the HTML clients.

    \sa QWebChannel, {Qt WebChannel JavaScript API}{JavaScript API}
*/

/*!
  \qmlproperty list<QtObject> WebChannel::transports
  A list of transport objects, which implement QWebChannelAbstractTransport. The transports
  are used to talk to the remote clients.

  \sa connectTo(), disconnectFrom()
*/

/*!
  \qmlproperty list<QtObject> WebChannel::registeredObjects

  \brief A list of objects which should be accessible to remote clients.

  The objects must have the attached \l id property set to an identifier, under which the
  object is then known on the HTML side.

  Once registered, all signals and property changes are automatically propagated to the clients.
  Public invokable methods, including slots, are also accessible to the clients.

  If one needs to register objects which are not available when the component is created, use the
  imperative registerObjects method.

  \sa registerObjects(), id
*/

class QQmlWebChannelPrivate : public QWebChannelPrivate
{
    Q_DECLARE_PUBLIC(QQmlWebChannel)
public:
    QList<QObject *> registeredObjects;

    void _q_objectIdChanged(const QString &newId);
};

/*!
    \internal

    Update the name of the sender object, when its attached WebChannel.id property changed.
    This is required, since during startup the property is empty and only gets set later on.
*/
void QQmlWebChannelPrivate::_q_objectIdChanged(const QString &newId)
{
    Q_Q(QQmlWebChannel);
    const QQmlWebChannelAttached *const attached =
            qobject_cast<QQmlWebChannelAttached *>(q->sender());
    Q_ASSERT(attached);
    Q_ASSERT(attached->parent());
    Q_ASSERT(registeredObjects.contains(attached->parent()));

    QObject *const object = attached->parent();
    const QString &oldId = publisher->registeredObjectIds.value(object);

    if (!oldId.isEmpty()) {
        q->deregisterObject(object);
    }

    q->registerObject(newId, object);
}

QQmlWebChannel::QQmlWebChannel(QObject *parent) : QWebChannel(*(new QQmlWebChannelPrivate), parent)
{
}

QQmlWebChannel::~QQmlWebChannel() { }

/*!
    \qmlmethod void WebChannel::registerObjects(object objects)
    Registers the specified \a objects to make them accessible to HTML clients.
    \a objects should be a JavaScript Map object.
    The key of the map is used as an identifier for the object on the client side.

    Once registered, all signals and property changes are automatically propagated to the clients.
    Public invokable methods, including slots, are also accessible to the clients.

    This imperative API can be used to register objects on the fly. For static objects, the
    declarative registeredObjects property should be preferred.

    \sa registeredObjects
*/
void QQmlWebChannel::registerObjects(const QVariantMap &objects)
{
    Q_D(QQmlWebChannel);
    QMap<QString, QVariant>::const_iterator it = objects.constBegin();
    for (; it != objects.constEnd(); ++it) {
        QObject *object = it.value().value<QObject *>();
        if (!object) {
            qWarning("Invalid QObject given to register under name %s", qPrintable(it.key()));
            continue;
        }
        d->publisher->registerObject(it.key(), object);
    }
}

QQmlWebChannelAttached *QQmlWebChannel::qmlAttachedProperties(QObject *obj)
{
    return new QQmlWebChannelAttached(obj);
}

/*!
    \qmlmethod void WebChannel::connectTo(QtObject transport)

    \brief Connects to the \a transport, which represents a communication
    channel to a single client.

    The transport object must be an implementation of \l QWebChannelAbstractTransport.

    \sa transports, disconnectFrom()
*/
void QQmlWebChannel::connectTo(QObject *transport)
{
    if (QWebChannelAbstractTransport *realTransport =
                qobject_cast<QWebChannelAbstractTransport *>(transport)) {
        QWebChannel::connectTo(realTransport);
    } else {
        qWarning() << "Cannot connect to transport" << transport
                   << " - it is not a QWebChannelAbstractTransport.";
    }
}

/*!
    \qmlmethod void WebChannel::disconnectFrom(QtObject transport)

    \brief Disconnects the \a transport from this WebChannel.

    The client will not be able to communicate with the WebChannel anymore, nor will it receive any
    signals or property updates.

    \sa connectTo()
*/
void QQmlWebChannel::disconnectFrom(QObject *transport)
{
    if (QWebChannelAbstractTransport *realTransport =
                qobject_cast<QWebChannelAbstractTransport *>(transport)) {
        QWebChannel::disconnectFrom(realTransport);
    } else {
        qWarning() << "Cannot disconnect from transport" << transport
                   << " - it is not a QWebChannelAbstractTransport.";
    }
}

/*!
    \property QQmlWebChannel::registeredObjects

    This property holds the list of objects which should be accessible to remote clients.

    The objects must have the attached id property set to an identifier, under which the
    object is then known on the HTML side.

    Once registered, all signals and property changes are automatically propagated to the clients.
    Public invokable methods, including slots, are also accessible to the clients.

    If one needs to register objects which are not available when the component is created, use the
    imperative registerObjects method.

    \sa registerObjects(), id
*/
QQmlListProperty<QObject> QQmlWebChannel::registeredObjects()
{
    return QQmlListProperty<QObject>(this, nullptr, registeredObjects_append,
                                     registeredObjects_count, registeredObjects_at,
                                     registeredObjects_clear);
}

void QQmlWebChannel::registeredObjects_append(QQmlListProperty<QObject> *prop, QObject *object)
{
    if (!object) {
        qWarning() << "Cannot register null object to WebChannel";
        return;
    }
    const QQmlWebChannelAttached *const attached = qobject_cast<QQmlWebChannelAttached *>(
            qmlAttachedPropertiesObject<QQmlWebChannel>(object, false /* don't create */));
    if (!attached) {
        const QQmlContext *const context = qmlContext(object);
        if (context) {
            qWarning() << "Cannot register object" << context->nameForObject(object) << '(' << object
                       << ") without attached WebChannel.id property. Did you forget to set it?";
        } else {
            qWarning() << "Cannot register an object without WebChannel attached property.";
        }
        return;
    }
    QQmlWebChannel *channel = static_cast<QQmlWebChannel *>(prop->object);
    if (!attached->id().isEmpty()) {
        // TODO: warning in such cases?
        channel->registerObject(attached->id(), object);
    }
    channel->d_func()->registeredObjects.append(object);
    connect(attached, SIGNAL(idChanged(QString)), channel, SLOT(_q_objectIdChanged(QString)));
}

qsizetype QQmlWebChannel::registeredObjects_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QQmlWebChannel *>(prop->object)->d_func()->registeredObjects.size();
}

QObject *QQmlWebChannel::registeredObjects_at(QQmlListProperty<QObject> *prop, qsizetype index)
{
    return static_cast<QQmlWebChannel *>(prop->object)->d_func()->registeredObjects.at(index);
}

void QQmlWebChannel::registeredObjects_clear(QQmlListProperty<QObject> *prop)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel *>(prop->object);
    const auto moved = std::exchange(channel->d_func()->registeredObjects, {}); // precautionary
    for (QObject *object : moved)
        channel->deregisterObject(object);
}

/*!
    \property QQmlWebChannel::transports

    This property holds a list of transport objects, which implement QWebChannelAbstractTransport.
    The transports are used to talk to the remote clients.

    \sa connectTo(), disconnectFrom()
*/
QQmlListProperty<QObject> QQmlWebChannel::transports()
{
    return QQmlListProperty<QObject>(this, nullptr, transports_append, transports_count,
                                     transports_at, transports_clear);
}

void QQmlWebChannel::transports_append(QQmlListProperty<QObject> *prop, QObject *transport)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel *>(prop->object);
    channel->connectTo(transport);
}

qsizetype QQmlWebChannel::transports_count(QQmlListProperty<QObject> *prop)
{
    return static_cast<QQmlWebChannel *>(prop->object)->d_func()->transports.size();
}

QObject *QQmlWebChannel::transports_at(QQmlListProperty<QObject> *prop, qsizetype index)
{
    QQmlWebChannel *channel = static_cast<QQmlWebChannel *>(prop->object);
    return channel->d_func()->transports.at(index);
}

void QQmlWebChannel::transports_clear(QQmlListProperty<QObject> *prop)
{
    QWebChannel *channel = static_cast<QWebChannel *>(prop->object);
    // Need to take a copy (incl. forced detach :():
    // - disconnectFrom() modifies `transport`, and
    // - if we were to use a consume loop, disconnectFrom() wouldn't actually disconnect
    const auto copy = channel->d_func()->transports;
    for (QWebChannelAbstractTransport *transport : copy)
        channel->disconnectFrom(transport);
    Q_ASSERT(channel->d_func()->transports.isEmpty());
}

QT_END_NAMESPACE

#include "moc_qqmlwebchannel.cpp"
