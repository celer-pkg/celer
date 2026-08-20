// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtNetwork/private/qnetworkinformation_p.h>

#include <QtCore/qglobal.h>
#include <QtCore/private/qobject_p.h>
#include <QtCore/qmutex.h>

#include <Network/Network.h>

QT_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(lcNetInfoSCR)
Q_LOGGING_CATEGORY(lcNetInfoSCR, "qt.network.info.applenetworkinfo");

namespace {

class ReachabilityDispatchQueue
{
public:
    ReachabilityDispatchQueue()
    {
        queue = dispatch_queue_create("qt-network-reachability-queue", nullptr);
        if (!queue)
            qCWarning(lcNetInfoSCR, "Failed to create a dispatch queue for reachability probes");
    }

    ~ReachabilityDispatchQueue()
    {
        if (queue)
            dispatch_release(queue);
    }

    dispatch_queue_t data() const
    {
        return queue;
    }

private:
    dispatch_queue_t queue = nullptr;

    Q_DISABLE_COPY_MOVE(ReachabilityDispatchQueue)
};

dispatch_queue_t qt_reachability_queue()
{
    static const ReachabilityDispatchQueue reachabilityQueue;
    return reachabilityQueue.data();
}

} // unnamed namespace

static QString backendName()
{
    return QString::fromUtf16(QNetworkInformationBackend::PluginNames
                                      [QNetworkInformationBackend::PluginNamesAppleIndex]);
}

class QAppleNetworkInformationBackend : public QNetworkInformationBackend
{
    Q_OBJECT
public:
    enum class InterfaceType {
        Unknown,
        Ethernet,
        Cellular,
        WiFi,
    };
    Q_ENUM(InterfaceType)

    QAppleNetworkInformationBackend();
    ~QAppleNetworkInformationBackend();

    QString name() const override { return backendName(); }
    QNetworkInformation::Features featuresSupported() const override
    {
        return featuresSupportedStatic();
    }

    static QNetworkInformation::Features featuresSupportedStatic()
    {
        return QNetworkInformation::Features(QNetworkInformation::Feature::Reachability
                                             | QNetworkInformation::Feature::TransportMedium);
    }

    void reachabilityChanged(bool isOnline);
    void interfaceTypeChanged(QAppleNetworkInformationBackend::InterfaceType type);

private:
    Q_DISABLE_COPY_MOVE(QAppleNetworkInformationBackend)

    bool isReachable() const;
    bool startMonitoring();
    void stopMonitoring();
    void updateState(nw_path_t state);

    nw_path_status_t status = nw_path_status_invalid;

    struct QueueCallbackData
    {
        QMutex monitorMutex;
        QAppleNetworkInformationBackend *backend = nullptr;
    } *callbackData = nullptr;

    nw_path_monitor_t monitor = nullptr;
    QAppleNetworkInformationBackend::InterfaceType interface = InterfaceType::Unknown;
};

class QAppleNetworkInformationBackendFactory : public QNetworkInformationBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QNetworkInformationBackendFactory_iid)
    Q_INTERFACES(QNetworkInformationBackendFactory)
public:
    QAppleNetworkInformationBackendFactory() = default;
    ~QAppleNetworkInformationBackendFactory() = default;
    QString name() const override { return backendName(); }
    QNetworkInformation::Features featuresSupported() const override
    {
        return QAppleNetworkInformationBackend::featuresSupportedStatic();
    }

    QNetworkInformationBackend *create(
        QNetworkInformation::Features requiredFeatures) const override
    {
        if ((requiredFeatures & featuresSupported()) != requiredFeatures)
            return nullptr;
        return new QAppleNetworkInformationBackend();
    }

private:
    Q_DISABLE_COPY_MOVE(QAppleNetworkInformationBackendFactory)
};

QAppleNetworkInformationBackend::QAppleNetworkInformationBackend()
{
    startMonitoring();
}

QAppleNetworkInformationBackend::~QAppleNetworkInformationBackend()
{
    stopMonitoring();
}

void QAppleNetworkInformationBackend::reachabilityChanged(bool isOnline)
{
    setReachability(isOnline ? QNetworkInformation::Reachability::Online
                             : QNetworkInformation::Reachability::Disconnected);
}

void QAppleNetworkInformationBackend::interfaceTypeChanged(QAppleNetworkInformationBackend::InterfaceType type)
{

    if (reachability() == QNetworkInformation::Reachability::Disconnected) {
        setTransportMedium(QNetworkInformation::TransportMedium::Unknown);
    } else {
        switch (type) {
        case QAppleNetworkInformationBackend::InterfaceType::Ethernet:
            setTransportMedium(QNetworkInformation::TransportMedium::Ethernet);
            break;
        case QAppleNetworkInformationBackend::InterfaceType::Cellular:
            setTransportMedium(QNetworkInformation::TransportMedium::Cellular);
            break;
        case QAppleNetworkInformationBackend::InterfaceType::WiFi:
            setTransportMedium(QNetworkInformation::TransportMedium::WiFi);
            break;
        case QAppleNetworkInformationBackend::InterfaceType::Unknown:
            setTransportMedium(QNetworkInformation::TransportMedium::Unknown);
            break;
        }
    }
}

bool QAppleNetworkInformationBackend::isReachable() const
{
    return status == nw_path_status_satisfied;
}

bool QAppleNetworkInformationBackend::startMonitoring()
{
    Q_ASSERT(!monitor && !callbackData);

    monitor = nw_path_monitor_create();
    if (monitor == nullptr) {
        qCWarning(lcNetInfoSCR, "Failed to create a path monitor, cannot determine current reachability.");
        return false;
    }

    auto queue = qt_reachability_queue();
    if (!queue) {
        qCWarning(lcNetInfoSCR, "Failed to create a dispatch queue to schedule a probe on");
        nw_release(monitor);
        monitor = nullptr;
        return false;
    }

    callbackData = new QueueCallbackData;
    auto *data = callbackData;
    callbackData->backend = this;

    nw_path_monitor_set_update_handler(monitor, [data](nw_path_t path){
        const QMutexLocker lock(&data->monitorMutex);
        if (data->backend)
            data->backend->updateState(path);
        // Else - we were cancelled and will delete 'data' in the callback below.
        // Presumably, this gets never called after 'cancel handler'.
    });

    nw_path_monitor_set_cancel_handler(monitor, [data]{
        delete data;
    });

    nw_path_monitor_set_queue(monitor, queue);
    nw_path_monitor_start(monitor);
    return true;
}

void QAppleNetworkInformationBackend::stopMonitoring()
{
    Q_ASSERT(callbackData && monitor);
    {
        const QMutexLocker lock(&callbackData->monitorMutex); // Release the lock _before_ cancelling.
        callbackData->backend = nullptr; // This will prevent updateState calls from the queue.
        callbackData = nullptr; // To be deleted in the cancellation callback.
    }
    nw_path_monitor_cancel(monitor);
    nw_release(monitor);
    monitor = nullptr;
}

void QAppleNetworkInformationBackend::updateState(nw_path_t state)
{
    Q_ASSERT(callbackData);

    // Lock is acquired in the callback (which is calling us).

    // NETMONTODO: for now, 'online' for us means nw_path_status_satisfied
    // is set. There are more possible flags that require more tests/some special
    // setup. So in future this part and related can change/be extended.
    const bool wasReachable = isReachable();
    const QAppleNetworkInformationBackend::InterfaceType hadInterfaceType = interface;
    const nw_path_status_t previousStatus = status;

    status = nw_path_get_status(state);
    if (wasReachable != isReachable() || previousStatus == nw_path_status_invalid)
        reachabilityChanged(isReachable());

    nw_path_enumerate_interfaces(state, ^(nw_interface_t nwInterface) {
        if (nw_path_uses_interface_type(state, nw_interface_get_type(nwInterface))) {
            const nw_interface_type_t type = nw_interface_get_type(nwInterface);

            switch (type) {
                case nw_interface_type_wifi:
                    interface = QAppleNetworkInformationBackend::InterfaceType::WiFi;
                    break;
                case nw_interface_type_cellular:
                    interface = QAppleNetworkInformationBackend::InterfaceType::Cellular;
                    break;
                case nw_interface_type_wired:
                    interface = QAppleNetworkInformationBackend::InterfaceType::Ethernet;
                    break;
                default:
                    interface = QAppleNetworkInformationBackend::InterfaceType::Unknown;
                    break;
            }

            return false;
        }

        return true;
    });

    if (hadInterfaceType != interface)
        interfaceTypeChanged(interface);
}

QT_END_NAMESPACE

#include "qapplenetworkinformationbackend.moc"
