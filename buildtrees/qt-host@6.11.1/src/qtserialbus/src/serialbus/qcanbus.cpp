// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanbus.h"
#include "qcanbusfactory.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qglobalstatic.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qpluginloader.h>

#include <private/qfactoryloader_p.h>

#define QCanBusFactory_iid "org.qt-project.Qt.QCanBusFactory"

QT_BEGIN_NAMESPACE

class QCanBusPrivate
{
public:
    QCanBusPrivate() { }
    QCanBusPrivate(int index, const QCborMap &meta) : meta(meta), index(index) {}

    QCborMap meta;
    QObject *factory = nullptr;
    int index = -1;
};

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qFactoryLoader,
    (QCanBusFactory_iid, QLatin1String("/canbus")))

typedef QMap<QString, QCanBusPrivate> QCanBusPluginStore;
Q_GLOBAL_STATIC(QCanBusPluginStore, qCanBusPlugins)

static QCanBus *globalInstance = nullptr;

static void loadPlugins()
{
    const QList<QPluginParsedMetaData> meta = qFactoryLoader()->metaData();
    for (int i = 0; i < meta.size(); i++) {
        const QCborMap obj = meta.at(i).value(QtPluginMetaDataKeys::MetaData).toMap();
        if (obj.isEmpty())
            continue;

        qCanBusPlugins()->insert(obj.value(QLatin1String("Key")).toString(), {i, obj});
    }
}

/*!
    \class QCanBus
    \inmodule QtSerialBus
    \since 5.8

    \brief The QCanBus class handles registration and creation of bus plugins.

    QCanBus loads Qt CAN Bus plugins at runtime. The ownership of serial bus plugins is
    transferred to the loader.
*/

/*!
    Returns a pointer to the QCanBus class. The object is loaded if necessary. QCanBus
    uses the singleton design pattern.
*/
QCanBus *QCanBus::instance()
{
    if (!globalInstance)
        globalInstance = new QCanBus();
    return globalInstance;
}

/*!
    Returns a list of identifiers for all loaded plugins.
*/
QStringList QCanBus::plugins() const
{
    return qCanBusPlugins()->keys();
}

static void setErrorMessage(QString *result, const QString &message)
{
    if (!result)
        return;

    *result = message;
}

static QObject *canBusFactory(const QString &plugin, QString *errorMessage)
{
    if (Q_UNLIKELY(!qCanBusPlugins()->contains(plugin))) {
        setErrorMessage(errorMessage, QCanBus::tr("No such plugin: '%1'").arg(plugin));
        return nullptr;
    }

    QCanBusPrivate d = qCanBusPlugins()->value(plugin);
    if (!d.factory) {
        d.factory = qFactoryLoader->instance(d.index);

        if (d.factory)
            qCanBusPlugins()->insert(plugin, d);
    }

    if (Q_UNLIKELY(!d.factory))
        setErrorMessage(errorMessage, QCanBus::tr("No factory for plugin: '%1'").arg(plugin));

    return d.factory;
}

/*!
    \since 5.9

    Returns the available interfaces for \a plugin. In case of failure, the optional
    parameter \a errorMessage returns a textual error description.

    \note Some plugins might not or only partially support this function.

    For example, the following call returns a list of all available SocketCAN
    interfaces (which can be used for \l createDevice()):

    \code
        QString errorString;
        const QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
            QStringLiteral("socketcan"), &errorString);
        if (!errorString.isEmpty())
            qDebug() << errorString;
    \endcode

    \sa createDevice()
*/
QList<QCanBusDeviceInfo> QCanBus::availableDevices(const QString &plugin, QString *errorMessage) const
{
    const QObject *obj = canBusFactory(plugin, errorMessage);
    if (Q_UNLIKELY(!obj))
        return QList<QCanBusDeviceInfo>();

    const QCanBusFactory *factory = qobject_cast<const QCanBusFactory *>(obj);
    if (Q_UNLIKELY(!factory)) {
        setErrorMessage(errorMessage,
                        tr("The plugin '%1' does not provide this function.").arg(plugin));
        return QList<QCanBusDeviceInfo>();
    }

    QString errorString;
    QList<QCanBusDeviceInfo> result = factory->availableDevices(&errorString);

    setErrorMessage(errorMessage, errorString);
    return result;
}

/*!
    \since 6.8
    Returns the available devices for all plugins, i.e. all available CAN interfaces.

    In case of failure, the optional parameter \a errorMessage returns a textual
    error description.

    \note Some plugins might not or only partially support this function.

    \sa createDevice()
*/
QList<QCanBusDeviceInfo> QCanBus::availableDevices(QString *errorMessage) const
{
    const QStringList allPlugins = plugins();
    QList<QCanBusDeviceInfo> result;

    for (const QString &plugin : allPlugins)
        result.append(availableDevices(plugin, errorMessage));

    return result;
}

/*!
    Creates a CAN bus device. \a plugin is the name of the plugin as returned by the \l plugins()
    method. \a interfaceName is the CAN bus interface name. In case of failure, the optional
    parameter \a errorMessage returns a textual error description.

    Ownership of the returned plugin is transferred to the caller.
    Returns \c nullptr if no suitable device can be found.

    For example, the following call would connect to the SocketCAN interface vcan0:

    \code
        QString errorString;
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("socketcan"), QStringLiteral("vcan0"), &errorString);
        if (!device)
            qDebug() << errorString;
        else
            device->connectDevice();
    \endcode

    \note The \a interfaceName is plugin-dependent. See the corresponding plugin documentation
    for more information: \l {CAN Bus Plugins}. To get a list of available interfaces,
    \l availableDevices() can be used.

    \sa availableDevices()
*/
QCanBusDevice *QCanBus::createDevice(const QString &plugin, const QString &interfaceName,
                                     QString *errorMessage) const
{
    const QObject *obj = canBusFactory(plugin, errorMessage);
    if (Q_UNLIKELY(!obj))
        return nullptr;

    const QCanBusFactory *factory = qobject_cast<const QCanBusFactory *>(obj);
    if (Q_LIKELY(factory))
        return factory->createDevice(interfaceName, errorMessage);

    setErrorMessage(errorMessage,
                    tr("The plugin '%1' does not provide this function.").arg(plugin));
    return nullptr;
}

QCanBus::QCanBus(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

QT_END_NAMESPACE
