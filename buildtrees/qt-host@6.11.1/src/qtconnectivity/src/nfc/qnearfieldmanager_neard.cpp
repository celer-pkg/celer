// Copyright (C) 2016 BlackBerry Limited, Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager_neard_p.h"
#include "qnearfieldtarget_neard_p.h"

#include "adapter_interface.h"
#include "properties_interface.h"
#include "objectmanager_interface.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)
Q_LOGGING_CATEGORY(QT_NFC_NEARD, "qt.nfc.neard")

using namespace QtNfcPrivate; // for D-Bus wrappers

// TODO We need a constructor that lets us select an adapter
QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
    : QNearFieldManagerPrivate(),
      m_neardHelper(NeardHelper::instance())
{
    QDBusPendingReply<ManagedObjectList> reply = m_neardHelper->dbusObjectManager()->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_NFC_NEARD) << "Error getting managed objects";
        return;
    }

    bool found = false;
    const QList<QDBusObjectPath> paths = reply.value().keys();
    for (const QDBusObjectPath &path : paths) {
        const InterfaceList ifaceList = reply.value().value(path);
        const QStringList ifaces = ifaceList.keys();
        for (const QString &iface : ifaces) {
            if (iface == QStringLiteral("org.neard.Adapter")) {
                found = true;
                m_adapterPath = path.path();
                qCDebug(QT_NFC_NEARD) << "org.neard.Adapter found for path" << m_adapterPath;
                break;
            }
        }

        if (found)
            break;
    }

    if (!found) {
        qCWarning(QT_NFC_NEARD) << "no adapter found, neard daemon running?";
    } else {
        connect(m_neardHelper, &NeardHelper::tagFound,
                this,          &QNearFieldManagerPrivateImpl::handleTagFound);
        connect(m_neardHelper, &NeardHelper::tagRemoved,
                this,          &QNearFieldManagerPrivateImpl::handleTagRemoved);
    }
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    stopTargetDetection();
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    if (!m_neardHelper->dbusObjectManager()->isValid() || m_adapterPath.isNull()) {
        qCWarning(QT_NFC_NEARD) << "dbus object manager invalid or adapter path invalid";
        return false;
    }

    QDBusPendingReply<ManagedObjectList> reply = m_neardHelper->dbusObjectManager()->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_NFC_NEARD) << "error getting managed objects";
        return false;
    }

    const QList<QDBusObjectPath> paths = reply.value().keys();
    for (const QDBusObjectPath &path : paths) {
        if (m_adapterPath == path.path())
            return true;
    }

    return false;
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    if (m_adapterPath.isEmpty()) {
        qCWarning(QT_NFC_NEARD) << "no adapter found, neard daemon running?";
        return false;
    }

    if (!m_neardHelper->dbusObjectManager()->isValid()) {
        qCWarning(QT_NFC_NEARD) << "dbus object manager invalid or adapter path invalid";
        return false;
    }

    return accessMethod == QNearFieldTarget::NdefAccess;
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    qCDebug(QT_NFC_NEARD) << "starting target detection";
    if (!isEnabled() || accessMethod != QNearFieldTarget::NdefAccess)
        return false;

    OrgFreedesktopDBusPropertiesInterface dbusProperties(QStringLiteral("org.neard"),
                                                         m_adapterPath,
                                                         QDBusConnection::systemBus());

    if (!dbusProperties.isValid()) {
        qCWarning(QT_NFC_NEARD) << "dbus property interface invalid";
        return false;
    }

    // check if the adapter is currently polling
    QDBusPendingReply<QDBusVariant> replyPolling = dbusProperties.Get(QStringLiteral("org.neard.Adapter"),
                                                                      QStringLiteral("Polling"));
    replyPolling.waitForFinished();
    if (!replyPolling.isError()) {
        if (replyPolling.value().variant().toBool()) {
            qCDebug(QT_NFC_NEARD) << "adapter is already polling";
            return true;
        }
    } else {
        qCWarning(QT_NFC_NEARD) << "error getting 'Polling' state from property interface";
        return false;
    }

    // check if the adapter it powered
    QDBusPendingReply<QDBusVariant> replyPowered = dbusProperties.Get(QStringLiteral("org.neard.Adapter"),
                                                                      QStringLiteral("Powered"));
    replyPowered.waitForFinished();
    if (!replyPowered.isError()) {
        if (replyPowered.value().variant().toBool()) {
            qCDebug(QT_NFC_NEARD) << "adapter is already powered";
        } else {
            QDBusPendingReply<QDBusVariant> replyTryPowering = dbusProperties.Set(QStringLiteral("org.neard.Adapter"),
                                                                                  QStringLiteral("Powered"),
                                                                                  QDBusVariant(true));
            replyTryPowering.waitForFinished();
            if (!replyTryPowering.isError()) {
                qCDebug(QT_NFC_NEARD) << "powering adapter";
            }
        }
    } else {
        qCWarning(QT_NFC_NEARD) << "error getting 'Powered' state from property interface";
        return false;
    }

    // create adapter and start poll loop
    OrgNeardAdapterInterface neardAdapter(QStringLiteral("org.neard"),
                                          m_adapterPath,
                                          QDBusConnection::systemBus());

    // possible modes: "Target", "Initiator", "Dual"
    QDBusPendingReply<> replyPollLoop = neardAdapter.StartPollLoop(QStringLiteral("Initiator"));
    replyPollLoop.waitForFinished();
    if (replyPollLoop.isError()) {
        qCWarning(QT_NFC_NEARD) << "error when starting polling";
        return false;
    } else {
        qCDebug(QT_NFC_NEARD) << "successfully started polling";
    }

    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString&)
{
    qCDebug(QT_NFC_NEARD) << "stopping target detection";
    if (!isEnabled())
        return;

    OrgFreedesktopDBusPropertiesInterface dbusProperties(QStringLiteral("org.neard"),
                                                         m_adapterPath,
                                                         QDBusConnection::systemBus());

    if (!dbusProperties.isValid()) {
        qCWarning(QT_NFC_NEARD) << "dbus property interface invalid";
        return;
    }

    // check if the adapter is currently polling
    QDBusPendingReply<QDBusVariant> replyPolling = dbusProperties.Get(QStringLiteral("org.neard.Adapter"),
                                                                      QStringLiteral("Polling"));
    replyPolling.waitForFinished();
    if (!replyPolling.isError()) {
        if (replyPolling.value().variant().toBool()) {
            // create adapter and stop poll loop
            OrgNeardAdapterInterface neardAdapter(QStringLiteral("org.neard"),
                                                  m_adapterPath,
                                                  QDBusConnection::systemBus());

            QDBusPendingReply<> replyStopPolling = neardAdapter.StopPollLoop();
            replyStopPolling.waitForFinished();
            if (replyStopPolling.isError())
                qCWarning(QT_NFC_NEARD) << "error when stopping polling";
            else
                qCDebug(QT_NFC_NEARD) << "successfully stopped polling";
        } else {
            qCDebug(QT_NFC_NEARD) << "already stopped polling";
        }
    } else {
        qCWarning(QT_NFC_NEARD) << "error getting 'Polling' state from property interface";
    }
}

void QNearFieldManagerPrivateImpl::handleTagFound(const QDBusObjectPath &path)
{
    auto priv = new QNearFieldTargetPrivateImpl(this, path);
    auto nfTag = new QNearFieldTarget(priv, this);
    m_activeTags.insert(path.path(), nfTag);
    emit targetDetected(nfTag);
}

void QNearFieldManagerPrivateImpl::handleTagRemoved(const QDBusObjectPath &path)
{
    const QString adapterPath = path.path();
    if (m_activeTags.contains(adapterPath)) {
        QNearFieldTarget *nfTag = m_activeTags.value(adapterPath);
        m_activeTags.remove(adapterPath);
        emit targetLost(nfTag);
    }
}

QT_END_NAMESPACE
