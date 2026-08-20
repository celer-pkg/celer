// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qconnmanservice.h"
#include "qconnmannetworkinformationbackend.h"

#include <QObject>
#include <QList>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusObjectPath>

#include <utility> // for std::pair

#define CONNMAN_DBUS_SERVICE "net.connman"_L1
#define CONNMAN_DBUS_INTERFACE "net.connman.Manager"
#define CONNMAN_DBUS_INTERFACE_L1 CONNMAN_DBUS_INTERFACE ""_L1
#define CONNMAN_DBUS_PATH "/"_L1

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

using ConnmanService = QMap<QString, QVariant>;
using ConnmanServiceEntry = std::pair<QDBusObjectPath, ConnmanService>;
using ConnmanServices = QList<ConnmanServiceEntry>;

namespace {
constexpr QLatin1StringView propertyChangedKey = "PropertyChanged"_L1;
constexpr QLatin1StringView stateKey = "State"_L1;
constexpr QLatin1StringView typeKey = "Type"_L1;
} // namespace

QConnManInterfaceBase::QConnManInterfaceBase(QObject *parent)
    : QDBusAbstractInterface(CONNMAN_DBUS_SERVICE, CONNMAN_DBUS_PATH, CONNMAN_DBUS_INTERFACE,
                             QDBusConnection::systemBus(), parent)
{
}

bool QConnManInterfaceBase::connmanAvailable()
{
    return QConnManInterfaceBase().isValid();
}

QConnManInterface::QConnManInterface(QObject *parent) : QConnManInterfaceBase(parent)
{
    if (!QDBusAbstractInterface::isValid())
        return;

    qDBusRegisterMetaType<ConnmanServiceEntry>();
    qDBusRegisterMetaType<ConnmanServices>();

    QDBusReply<QVariantMap> propertiesReply = call(QDBus::Block, "GetProperties"_L1);
    if (!propertiesReply.isValid()) {
        validDBusConnection = false;
        if (propertiesReply.error().type() != QDBusError::AccessDenied) {
            qWarning("Failed to query ConnMan properties: %ls",
                     qUtf16Printable(propertiesReply.error().message()));
        }
        return;
    }
    auto map = propertiesReply.value();
    m_state = map.value(stateKey).toString();
    m_type = findServiceType();

    validDBusConnection = QDBusConnection::systemBus().connect(
            CONNMAN_DBUS_SERVICE, CONNMAN_DBUS_PATH, CONNMAN_DBUS_INTERFACE_L1, propertyChangedKey,
            this, SLOT(propertyChanged(QString,QDBusVariant)));
}

QConnManInterface::~QConnManInterface()
{
    QDBusConnection::systemBus().disconnect(CONNMAN_DBUS_SERVICE, CONNMAN_DBUS_PATH,
                                            CONNMAN_DBUS_INTERFACE_L1, propertyChangedKey, this,
                                            SLOT(propertyChanged(QString,QDBusVariant)));
}

QString QConnManInterface::state() const
{
    return m_state;
}

QString QConnManInterface::type() const
{
    return m_type;
}

void QConnManInterface::setBackend(QConnManNetworkInformationBackend *ourBackend)
{
    backend = ourBackend;
}

void QConnManInterface::propertyChanged(const QString &name, const QDBusVariant &value)
{
    if (name == stateKey) {
        m_state = value.variant().toString();
        backend->onStateChanged(m_state);

        QString type = findServiceType();
        if (type != m_type) {
            m_type = std::move(type);
            backend->onTypeChanged(m_type);
        }
    }
}

QString QConnManInterface::findServiceType()
{
    QDBusReply<ConnmanServices> servicesReply = call(QDBus::Block, "GetServices"_L1);
    if (!servicesReply.isValid()) {
        if (servicesReply.error().type() != QDBusError::AccessDenied) {
            qWarning("Failed to query ConnMan services: %ls",
                     qUtf16Printable(servicesReply.error().message()));
        }
        return QString();
    }

    // The services list is sorted by connman and the first service matching
    // the current state is the most relevant.
    for (const auto &[_, service] : servicesReply.value()) {
        if (service.value(stateKey).toString() == m_state)
            return service.value(typeKey).toString();
    }
    return QString();
}

QT_END_NAMESPACE

#include "moc_qconnmanservice.cpp"
