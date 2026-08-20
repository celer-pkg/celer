// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothhostinfo.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"

#include "qbluetoothdevicediscoveryagent.h"

QT_BEGIN_NAMESPACE

/*!
    \class QBluetoothServiceDiscoveryAgentPrivate
    \inmodule QtBluetooth
    \internal
*/

/*!
    \class QBluetoothServiceDiscoveryAgent
    \inmodule QtBluetooth
    \brief The QBluetoothServiceDiscoveryAgent class enables you to query for
    Bluetooth services.

    \since 5.2

    The discovery process relies on the Bluetooth Service Discovery Process (SDP).
    The following steps are required to query the services provided by all contactable
    Bluetooth devices:

    \list
    \li create an instance of QBluetoothServiceDiscoveryAgent,
    \li connect to either the serviceDiscovered() or finished() signals,
    \li and call start().
    \endlist

    \snippet doc_src_qtbluetooth.cpp service_discovery

    By default a minimal service discovery is performed. In this mode, the returned \l QBluetoothServiceInfo
    objects are guaranteed to contain only device and service UUID information. Depending
    on platform and device capabilities, other service information may also be available.
    The minimal service discovery mode relies on cached SDP data of the platform. Therefore it
    is possible that this discovery does not find a device although it is physically available.
    In such cases a full discovery must be performed to force an update of the platform cache.
    However for most use cases a minimal discovery is adequate as it is much quicker and other
    classes which require up-to-date information such as QBluetoothSocket::connectToService()
    will perform additional discovery if required.  If the full service information is required,
    pass \l FullDiscovery as the discoveryMode parameter to start().

    This class may internally utilize \l QBluetoothDeviceDiscoveryAgent to find unknown devices.

    The service discovery may find Bluetooth Low Energy services too if the target device
    is a combination of a classic and Low Energy device. Those devices are required to advertise
    their Low Energy services via SDP. If the target device only supports Bluetooth Low
    Energy services, it is likely to not advertise them via SDP. The \l QLowEnergyController class
    should be utilized to perform the service discovery on Low Energy devices.

    On iOS, this class cannot be used because the platform does not expose
    an API which may permit access to QBluetoothServiceDiscoveryAgent related features.

    \sa QBluetoothDeviceDiscoveryAgent, QLowEnergyController
*/

/*!
    \enum QBluetoothServiceDiscoveryAgent::Error

    This enum describes errors that can occur during service discovery.

    \value NoError          No error has occurred.
    \value PoweredOffError  The Bluetooth adaptor is powered off, power it on before doing discovery.
    \value InputOutputError    Writing or reading from the device resulted in an error.
    \value [since 5.3] InvalidBluetoothAdapterError The passed local adapter address does not
                                                    match the physical adapter address of any
                                                    local Bluetooth device.
    \value [since 6.4] MissingPermissionsError  The operating system requests
                                                permissions which were not
                                                granted by the user.
    \value UnknownError     An unknown error has occurred.
*/

/*!
    \enum QBluetoothServiceDiscoveryAgent::DiscoveryMode

    This enum describes the service discovery mode.

    \value MinimalDiscovery     Performs a minimal service discovery. The QBluetoothServiceInfo
    objects returned may be incomplete and are only guaranteed to contain device and service UUID information.
    Since a minimal discovery relies on cached SDP data it may not find a physically existing
    device until a \c FullDiscovery is performed.
    \value FullDiscovery        Performs a full service discovery.
*/

/*!
    \fn QBluetoothServiceDiscoveryAgent::serviceDiscovered(const QBluetoothServiceInfo &info)

    This signal is emitted when the Bluetooth service described by \a info is discovered.

    \note The passed \l QBluetoothServiceInfo parameter may contain a Bluetooth Low Energy
    service if the target device advertises the service via SDP. This is required from device
    which support both, classic Bluetooth (BaseRate) and Low Energy services.

    \sa QBluetoothDeviceInfo::coreConfigurations()
*/

/*!
    \fn QBluetoothServiceDiscoveryAgent::finished()

    This signal is emitted when the Bluetooth service discovery completes.

    Unlike the \l QBluetoothDeviceDiscoveryAgent::finished() signal this
    signal will even be emitted when an error occurred during the service discovery. Therefore
    it is recommended to check the \l errorOccurred() signal to evaluate the success of the
    service discovery discovery.
*/

/*!
    \fn void QBluetoothServiceDiscoveryAgent::errorOccurred(QBluetoothServiceDiscoveryAgent::Error
   error)

    This signal is emitted when an \a error occurs. The \a error parameter describes the error that
    occurred.

    \since 6.2
*/

/*!
    Constructs a new QBluetoothServiceDiscoveryAgent with \a parent. The search is performed via the
    local default Bluetooth adapter.
*/
QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(QObject *parent)
  : QObject(parent),
    d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(this, QBluetoothAddress()))
{
}

/*!
    Constructs a new QBluetoothServiceDiscoveryAgent for \a deviceAdapter and with \a parent.

    It uses \a deviceAdapter for the service search. If \a deviceAdapter is default constructed
    the resulting QBluetoothServiceDiscoveryAgent object will use the local default Bluetooth adapter.

    If a \a deviceAdapter is specified that is not a local adapter \l error() will be set to
    \l InvalidBluetoothAdapterError. Therefore it is recommended to test the error flag immediately after
    using this constructor.

    \note On WinRT the passed adapter address will be ignored.

    \note On Android passing any \a deviceAdapter address is meaningless as Android 6.0 or later does not publish
    the local Bluetooth address anymore. Subsequently, the passed adapter address can never be matched
    against the local adapter address. Therefore the subsequent call to \l start() will always trigger
    \l InvalidBluetoothAdapterError.

    \sa error()
*/
QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(const QBluetoothAddress &deviceAdapter, QObject *parent)
  : QObject(parent),
    d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(this, deviceAdapter))
{
    if (!deviceAdapter.isNull()) {
        const QList<QBluetoothHostInfo> localDevices = QBluetoothLocalDevice::allDevices();
        for (const QBluetoothHostInfo &hostInfo : localDevices) {
            if (hostInfo.address() == deviceAdapter)
                return;
        }
        d_ptr->error = InvalidBluetoothAdapterError;
        d_ptr->errorString = tr("Invalid Bluetooth adapter address");
    }
}

/*!

  Destructor for QBluetoothServiceDiscoveryAgent

*/

QBluetoothServiceDiscoveryAgent::~QBluetoothServiceDiscoveryAgent()
{
    if (isActive()) {
        disconnect(); //don't emit any signals due to stop()
        stop();
    }

    delete d_ptr;
}

/*!
    Returns the list of all discovered services.

    This list of services accumulates newly discovered services from multiple calls
    to \l start(). Unless \l clear() is called the list cannot decrease in size. This implies
    that if a remote Bluetooth device moves out of range in between two subsequent calls
    to \l start() the list may contain stale entries.

    \note The list of services should always be cleared before the discovery mode is changed.

    \sa clear()
*/
QList<QBluetoothServiceInfo> QBluetoothServiceDiscoveryAgent::discoveredServices() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->discoveredServices;
}
/*!
    Sets the UUID filter to \a uuids.  Only services matching the UUIDs in \a uuids will be
    returned. The matching applies to the service's
    \l {QBluetoothServiceInfo::ServiceId}{ServiceId} and \l {QBluetoothServiceInfo::ServiceClassIds} {ServiceClassIds}
    attributes.

    An empty UUID list is equivalent to a list containing only QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QList<QBluetoothUuid> &uuids)
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    d->uuidFilter = uuids;
}

/*!
    This is an overloaded member function, provided for convenience.

    Sets the UUID filter to a list containing the single element \a uuid.
    The matching applies to the service's \l {QBluetoothServiceInfo::ServiceId}{ServiceId}
    and \l {QBluetoothServiceInfo::ServiceClassIds} {ServiceClassIds}
    attributes.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QBluetoothUuid &uuid)
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    d->uuidFilter.clear();
    d->uuidFilter.append(uuid);
}

/*!
    Returns the UUID filter.

    \sa setUuidFilter()
*/
QList<QBluetoothUuid> QBluetoothServiceDiscoveryAgent::uuidFilter() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->uuidFilter;
}

/*!
    Sets the remote device address to \a address. If \a address is default constructed,
    services will be discovered on all contactable Bluetooth devices. A new remote
    address can only be set while there is no service discovery in progress; otherwise
    this function returns false.

    On some platforms the service discovery might lead to pairing requests.
    Therefore it is not recommended to do service discoveries on all devices.
    This function can be used to restrict the service discovery to a particular device.

    \sa remoteAddress()
*/
bool QBluetoothServiceDiscoveryAgent::setRemoteAddress(const QBluetoothAddress &address)
{
    if (isActive())
        return false;
    if (!address.isNull())
        d_ptr->singleDevice = true;
    d_ptr->deviceAddress = address;
    return true;
}

/*!
    Returns the remote device address. If \l setRemoteAddress() is not called, the function
    will return a default constructed \l QBluetoothAddress.

    \sa setRemoteAddress()
*/
QBluetoothAddress QBluetoothServiceDiscoveryAgent::remoteAddress() const
{
    if (d_ptr->singleDevice == true)
        return d_ptr->deviceAddress;
    else
        return QBluetoothAddress();
}

namespace DarwinBluetooth {

void qt_test_iobluetooth_runloop();

}


/*!
    Starts service discovery. \a mode specifies the type of service discovery to perform.

    On some platforms, device discovery may lead to pairing requests.

    \sa DiscoveryMode
*/
void QBluetoothServiceDiscoveryAgent::start(DiscoveryMode mode)
{
    Q_D(QBluetoothServiceDiscoveryAgent);
#ifdef QT_OSX_BLUETOOTH
    // Make sure we are on the right thread/have a run loop:
    DarwinBluetooth::qt_test_iobluetooth_runloop();
#endif

    if (d->discoveryState() == QBluetoothServiceDiscoveryAgentPrivate::Inactive
            && d->error != InvalidBluetoothAdapterError) {
#if QT_CONFIG(bluez)
        // done to avoid repeated parsing for adapter address
        // on Bluez5
        d->foundHostAdapterPath.clear();
#endif
        d->setDiscoveryMode(mode);
        // Clear any possible previous errors
        d->error = QBluetoothServiceDiscoveryAgent::NoError;
        d->errorString.clear();
        if (d->deviceAddress.isNull()) {
            d->startDeviceDiscovery();
        } else {
            d->discoveredDevices << QBluetoothDeviceInfo(d->deviceAddress, QString(), 0);
            d->startServiceDiscovery();
        }
    }
}

/*!
    Stops the service discovery process. The \l canceled() signal will be emitted once
    the search has stopped.
*/
void QBluetoothServiceDiscoveryAgent::stop()
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    if (d->error == InvalidBluetoothAdapterError || !isActive())
        return;

    switch (d->discoveryState()) {
    case QBluetoothServiceDiscoveryAgentPrivate::DeviceDiscovery:
        d->stopDeviceDiscovery();
        break;
    case QBluetoothServiceDiscoveryAgentPrivate::ServiceDiscovery:
        d->stopServiceDiscovery();
        break;
    default:
        break;
    }

    d->discoveredDevices.clear();
}

/*!
    Clears the results of previous service discoveries and resets \l uuidFilter().
    This function does nothing during an ongoing service discovery (see \l isActive()).

    \sa discoveredServices()
*/
void QBluetoothServiceDiscoveryAgent::clear()
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    //don't clear the list while the search is ongoing
    if (isActive())
        return;

    d->discoveredDevices.clear();
    d->discoveredServices.clear();
    d->uuidFilter.clear();
}

/*!
    Returns \c true if the service discovery is currently active; otherwise returns \c false.
    An active discovery can be stopped by calling \l stop().
*/
bool QBluetoothServiceDiscoveryAgent::isActive() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->state != QBluetoothServiceDiscoveryAgentPrivate::Inactive;
}

/*!
    Returns the type of error that last occurred. If the service discovery is done
    for a single \l remoteAddress() it will return errors that occurred while trying to discover
    services on that device. If the \l remoteAddress() is not set and devices are
    discovered by a scan, errors during service discovery on individual
    devices are not saved and no signals are emitted. In this case, errors are
    fairly normal as some devices may not respond to discovery or
    may no longer be in range.  Such errors are suppressed.  If no services
    are returned, it can be assumed no services could be discovered.

    Any possible previous errors are cleared upon restarting the discovery.
*/
QBluetoothServiceDiscoveryAgent::Error QBluetoothServiceDiscoveryAgent::error() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->error;
}

/*!
    Returns a human-readable description of the last error that occurred during the
    service discovery.

    \sa error(), errorOccurred()
*/
QString QBluetoothServiceDiscoveryAgent::errorString() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);
    return d->errorString;
}


/*!
    \fn QBluetoothServiceDiscoveryAgent::canceled()

    This signal is triggered when the service discovery was canceled via a call to \l stop().
 */


/*!
    Starts device discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::startDeviceDiscovery()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (!deviceDiscoveryAgent) {
#if QT_CONFIG(bluez)
        deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(m_deviceAdapterAddress, q);
#else
        deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(q);
#endif
        QObject::connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                         q, [this](){
            this->_q_deviceDiscoveryFinished();
        });
        QObject::connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                         q, [this](const QBluetoothDeviceInfo &info){
            this->_q_deviceDiscovered(info);
        });
        QObject::connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, q,
                         [this](QBluetoothDeviceDiscoveryAgent::Error newError) {
                             this->_q_deviceDiscoveryError(newError);
                         });
    }

    setDiscoveryState(DeviceDiscovery);

    deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
}

/*!
    Stops device discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::stopDeviceDiscovery()
{
    // disconnect to avoid recursion during stop() - QTBUG-60131
    // we don't care about a potential signals from device discovery agent anymore
    deviceDiscoveryAgent->disconnect();

    deviceDiscoveryAgent->stop();
    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = nullptr;

    setDiscoveryState(Inactive);

    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

/*!
    Called when device discovery finishes.
*/
void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryFinished()
{
    if (deviceDiscoveryAgent->error() != QBluetoothDeviceDiscoveryAgent::NoError) {
        //Forward the device discovery error
        error = static_cast<QBluetoothServiceDiscoveryAgent::Error>(deviceDiscoveryAgent->error());
        errorString = deviceDiscoveryAgent->errorString();
        setDiscoveryState(Inactive);
        Q_Q(QBluetoothServiceDiscoveryAgent);
        emit q->errorOccurred(error);
        emit q->finished();
        return;
    }

    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = nullptr;

    startServiceDiscovery();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    // look for duplicates, and cached entries
    const auto addressEquals = [](const auto &a) {
        return [a](const auto &info) { return info.address() == a; };
    };
    erase_if(discoveredDevices, addressEquals(info.address()));
    discoveredDevices.prepend(info);
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error newError)
{
    error = static_cast<QBluetoothServiceDiscoveryAgent::Error>(newError);
    errorString = deviceDiscoveryAgent->errorString();

    // disconnect to avoid recursion during stop() - QTBUG-60131
    // we don't care about a potential signals from device discovery agent anymore
    deviceDiscoveryAgent->disconnect();

    deviceDiscoveryAgent->stop();
    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = nullptr;

    setDiscoveryState(Inactive);
    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->errorOccurred(error);
    emit q->finished();
}

/*!
    Starts service discovery for the next device.
*/
void QBluetoothServiceDiscoveryAgentPrivate::startServiceDiscovery()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (discoveredDevices.isEmpty()) {
        setDiscoveryState(Inactive);
        emit q->finished();
        return;
    }

    setDiscoveryState(ServiceDiscovery);
    start(discoveredDevices.at(0).address());
}

/*!
    Stops service discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::stopServiceDiscovery()
{
    stop();

    setDiscoveryState(Inactive);
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_serviceDiscoveryFinished()
{
    if(!discoveredDevices.isEmpty()) {
        discoveredDevices.removeFirst();
    }

    startServiceDiscovery();
}

bool QBluetoothServiceDiscoveryAgentPrivate::isDuplicatedService(
        const QBluetoothServiceInfo &serviceInfo) const
{
    //check the service is not already part of our known list
    for (const QBluetoothServiceInfo &info : discoveredServices) {
        if (info.device() == serviceInfo.device()
                && info.serviceClassUuids() == serviceInfo.serviceClassUuids()
                && info.serviceUuid() == serviceInfo.serviceUuid()
                && info.serverChannel() == serviceInfo.serverChannel()) {
            return true;
        }
    }
    return false;
}

QT_END_NAMESPACE

#include "moc_qbluetoothservicediscoveryagent.cpp"
