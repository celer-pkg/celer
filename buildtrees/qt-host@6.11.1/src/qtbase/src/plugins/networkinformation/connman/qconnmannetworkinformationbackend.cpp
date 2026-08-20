// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qconnmannetworkinformationbackend.h"

#include <QtCore/qglobal.h>
#include <QtCore/private/qobject_p.h>

#include <QtDBus/qdbusmessage.h>

QT_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(lcNetInfoConnman)
Q_LOGGING_CATEGORY(lcNetInfoConnman, "qt.network.info.connman")

using namespace Qt::StringLiterals;

namespace {
QNetworkInformation::Reachability reachabilityFromState(const QString &state)
{
    if (state == "idle"_L1)
        return QNetworkInformation::Reachability::Disconnected;
    else if (state == "ready"_L1)
        return QNetworkInformation::Reachability::Local;
    else if (state == "online"_L1)
        return QNetworkInformation::Reachability::Online;
    else
        return QNetworkInformation::Reachability::Unknown;
}

QNetworkInformation::TransportMedium transportMediumFromType(const QString &type)
{
    if (type == "ethernet"_L1)
        return QNetworkInformation::TransportMedium::Ethernet;
    else if (type == "wifi"_L1)
        return QNetworkInformation::TransportMedium::WiFi;
    else if (type == "bluetooth"_L1)
        return QNetworkInformation::TransportMedium::Bluetooth;
    else if (type == "cellular"_L1)
        return QNetworkInformation::TransportMedium::Cellular;
    else
        return QNetworkInformation::TransportMedium::Unknown;
}
} // unnamed namespace

static QString backendName()
{
    return "connman"_L1;
}

QString QConnManNetworkInformationBackend::name() const
{
    return backendName();
}

class QConnManNetworkInformationBackendFactory : public QNetworkInformationBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QNetworkInformationBackendFactory_iid)
    Q_INTERFACES(QNetworkInformationBackendFactory)
public:
    QConnManNetworkInformationBackendFactory() = default;
    ~QConnManNetworkInformationBackendFactory() = default;
    QString name() const override { return backendName(); }
    QNetworkInformation::Features featuresSupported() const override
    {
        if (!QConnManInterfaceBase::connmanAvailable())
            return {};
        return QConnManNetworkInformationBackend::featuresSupportedStatic();
    }

    QNetworkInformationBackend *
    create(QNetworkInformation::Features requiredFeatures) const override
    {
        if ((requiredFeatures & featuresSupported()) != requiredFeatures)
            return nullptr;
        if (!QConnManInterfaceBase::connmanAvailable())
            return nullptr;
        auto backend = new QConnManNetworkInformationBackend();
        if (!backend->isValid())
            delete std::exchange(backend, nullptr);
        return backend;
    }

private:
    Q_DISABLE_COPY_MOVE(QConnManNetworkInformationBackendFactory)
};

QConnManNetworkInformationBackend::QConnManNetworkInformationBackend()
{
    if (!iface.isValid())
        return;
    iface.setBackend(this);
    onStateChanged(iface.state());
    onTypeChanged(iface.type());
}

void QConnManNetworkInformationBackend::onStateChanged(const QString &state)
{
    qCInfo(lcNetInfoConnman) << "network state changed to" << state;
    QNetworkInformation::Reachability reachability = reachabilityFromState(state);
    setReachability(reachability);
}

void QConnManNetworkInformationBackend::onTypeChanged(const QString &type)
{
    qCInfo(lcNetInfoConnman) << "network type changed to" << type;
    QNetworkInformation::TransportMedium medium = transportMediumFromType(type);
    setTransportMedium(medium);
}

QT_END_NAMESPACE

#include "qconnmannetworkinformationbackend.moc"
#include "moc_qconnmannetworkinformationbackend.cpp"
