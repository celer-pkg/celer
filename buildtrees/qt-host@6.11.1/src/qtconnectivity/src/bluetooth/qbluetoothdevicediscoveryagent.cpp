// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qbluetoothhostinfo.h"

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT)

/*!
    \class QBluetoothDeviceDiscoveryAgent
    \inmodule QtBluetooth
    \brief The QBluetoothDeviceDiscoveryAgent class discovers the Bluetooth
    devices nearby.

    \since 5.2

    To discover the nearby Bluetooth devices:
    \list
    \li create an instance of QBluetoothDeviceDiscoveryAgent,
    \li connect to either the deviceDiscovered() or finished() signals,
    \li and call start().
    \endlist

    \snippet doc_src_qtbluetooth.cpp device_discovery

    To retrieve results asynchronously, connect to the deviceDiscovered() signal. To get a list of
    all discovered devices, call discoveredDevices() after the finished() signal.

    This class can be used to discover Classic and Low Energy Bluetooth devices.
    The individual device type can be determined via the
    \l QBluetoothDeviceInfo::coreConfigurations() attribute.
    In most cases the list returned by \l discoveredDevices() contains both types
    of devices. However not every platform can detect both types of devices.
    On platforms with this limitation (for example iOS only suports Low Energy discovery),
    the discovery process will limit the search to the type which is supported.

    \note Since Android 6.0 the ability to detect devices requires ACCESS_COARSE_LOCATION.

    \note The Win32 backend currently does not support the Received Signal Strength
    Indicator (RSSI), as well as the Manufacturer Specific Data, or other data
    updates advertised by Bluetooth LE devices after discovery.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::Error

    Indicates all possible error conditions found during Bluetooth device discovery.

    \value NoError          No error has occurred.
    \value PoweredOffError  The Bluetooth adaptor is powered off, power it on before doing discovery.
    \value InputOutputError    Writing or reading from the device resulted in an error.
    \value InvalidBluetoothAdapterError The passed local adapter address does not match the physical
                                        adapter address of any local Bluetooth device.
    \value [since 5.5] UnsupportedPlatformError Device discovery is not possible or implemented
                                                on the current platform. The error is set in
                                                response to a call to \l start(). An example for
                                                such cases are iOS versions below 5.0 which do
                                                not support Bluetooth device search at all.
    \value [since 5.8] UnsupportedDiscoveryMethod   One of the requested discovery methods is not
                                                    supported by the current platform.
    \value [since 6.2] LocationServiceTurnedOffError    The location service is turned off.
                                                        Usage of Bluetooth APIs is not possible
                                                        when location service is turned off.
    \value [since 6.4] MissingPermissionsError  The operating system requests
                                                permissions which were not
                                                granted by the user.
    \value UnknownError     An unknown error has occurred.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::DiscoveryMethod

    This enum descibes the type of discovery method employed by the QBluetoothDeviceDiscoveryAgent.

    \value NoMethod             The discovery is not possible. None of the available
                                methods are supported.
    \value ClassicMethod        The discovery process searches for Bluetooth Classic
                                (BaseRate) devices.
    \value LowEnergyMethod      The discovery process searches for Bluetooth Low Energy
                                devices.

    \sa supportedDiscoveryMethods()
    \since 5.8
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::deviceDiscovered(const QBluetoothDeviceInfo &info)

    This signal is emitted when the Bluetooth device described by \a info is discovered.

    The signal is emitted as soon as the most important device information
    has been collected. However, as long as the \l finished() signal has not
    been emitted the information collection continues even for already discovered
    devices. This is particularly true for signal strength information (RSSI) and
    manufacturer data updates. If the use case requires continuous manufacturer data
    or RSSI updates it is advisable to retrieve the device information via
    \l discoveredDevices() once the discovery has finished or listen to the
    \l deviceUpdated() signal.

    If \l lowEnergyDiscoveryTimeout() is larger than 0 the signal is only ever
    emitted when at least one attribute of \a info changes. This reflects the desire to
    receive updates as more precise information becomes available. The exception to this
    behavior is the case when \l lowEnergyDiscoveryTimeout is set to \c 0. A timeout of \c 0
    expresses the desire to monitor the appearance and disappearance of Low Energy devices
    over time. Under this condition the \l deviceDiscovered() signal is emitted even if
    \a info has not changed since the last signal emission.

    \sa QBluetoothDeviceInfo::rssi(), lowEnergyDiscoveryTimeout()
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::deviceUpdated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields)

    This signal is emitted when the agent receives additional information about
    the Bluetooth device described by \a info. The \a updatedFields flags tell
    which information has been updated.

    During discovery, some information can change dynamically, such as
    \l {QBluetoothDeviceInfo::rssi()}{signal strength} and
    \l {QBluetoothDeviceInfo::manufacturerData()}{manufacturerData}.
    This signal informs you that if your application is displaying this data, it
    can be updated, rather than waiting until the discovery has finished.

    \sa QBluetoothDeviceInfo::rssi(), lowEnergyDiscoveryTimeout()
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::finished()

    This signal is emitted when Bluetooth device discovery completes.
    The signal is not going to be emitted if the device discovery finishes with an error.
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::errorOccurred(QBluetoothDeviceDiscoveryAgent::Error
   error)

    This signal is emitted when an \a error occurs during Bluetooth device discovery.
    The \a error parameter describes the error that occurred.

    \sa error(), errorString()
    \since 6.2
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::canceled()

    This signal is emitted when device discovery is aborted by a call to stop().
*/

/*!
    \fn bool QBluetoothDeviceDiscoveryAgent::isActive() const

    Returns true if the agent is currently discovering Bluetooth devices, otherwise returns false.
*/

/*!
    Constructs a new Bluetooth device discovery agent with parent \a parent.
*/
QBluetoothDeviceDiscoveryAgent::QBluetoothDeviceDiscoveryAgent(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothDeviceDiscoveryAgentPrivate(QBluetoothAddress(), this))
{
}

/*!
    Constructs a new Bluetooth device discovery agent with \a parent.

    It uses \a deviceAdapter for the device search. If \a deviceAdapter is default constructed the resulting
    QBluetoothDeviceDiscoveryAgent object will use the local default Bluetooth adapter.

    If a \a deviceAdapter is specified that is not a local adapter \l error() will be set to
    \l InvalidBluetoothAdapterError. Therefore it is recommended to test the error flag immediately after
    using this constructor.

    \sa error()
*/
QBluetoothDeviceDiscoveryAgent::QBluetoothDeviceDiscoveryAgent(
    const QBluetoothAddress &deviceAdapter, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothDeviceDiscoveryAgentPrivate(deviceAdapter, this))
{
    if (!deviceAdapter.isNull()) {
        const QList<QBluetoothHostInfo> localDevices = QBluetoothLocalDevice::allDevices();
        for (const QBluetoothHostInfo &hostInfo : localDevices) {
            if (hostInfo.address() == deviceAdapter)
                return;
        }
        d_ptr->lastError = InvalidBluetoothAdapterError;
        d_ptr->errorString = tr("Invalid Bluetooth adapter address");
    }
}

/*!
  Destructor for ~QBluetoothDeviceDiscoveryAgent()
*/
QBluetoothDeviceDiscoveryAgent::~QBluetoothDeviceDiscoveryAgent()
{
    delete d_ptr;
}

/*!
    Returns a list of all discovered Bluetooth devices.
*/
QList<QBluetoothDeviceInfo> QBluetoothDeviceDiscoveryAgent::discoveredDevices() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->discoveredDevices;
}

/*!
    Sets the maximum search time for Bluetooth Low Energy device search to
    \a timeout in milliseconds. If \a timeout is \c 0 the discovery runs
    until \l stop() is called.

    This reflects the fact that the discovery process for Bluetooth Low Energy devices
    is mostly open ended. The platform continues to look for more devices until the search is
    manually stopped. The timeout ensures that the search is aborted after \a timeout milliseconds.
    Of course, it is still possible to manually abort the discovery by calling \l stop().

    The new timeout value does not take effect until the device search is restarted.
    In addition the timeout does not affect the classic Bluetooth device search. Depending on
    the platform the classic search may add more time to the total discovery process
    beyond \a timeout.

    For a reliable Bluetooth Low Energy discovery, use at least 40000 milliseconds.

    \sa lowEnergyDiscoveryTimeout()
    \since 5.8
 */
void QBluetoothDeviceDiscoveryAgent::setLowEnergyDiscoveryTimeout(int timeout)
{
    Q_D(QBluetoothDeviceDiscoveryAgent);

    // cannot deliberately turn it off
    if (timeout < 0) {
        qCDebug(QT_BT) << "The Bluetooth Low Energy device discovery timeout cannot be negative.";
        return;
    }

    if (d->lowEnergySearchTimeout < 0) {
        qCDebug(QT_BT) << "The Bluetooth Low Energy device discovery timeout cannot be  "
                          "set on a backend which does not support this feature.";
        return;
    }

    d->lowEnergySearchTimeout = timeout;
}

/*!
    Returns a timeout in milliseconds that is applied to the Bluetooth Low Energy device search.
    A value of \c -1 implies that the platform does not support this property and the timeout for
    the device search cannot be adjusted. A return value of \c 0
    implies a never-ending search which must be manually stopped via \l stop().

    \sa setLowEnergyDiscoveryTimeout()
    \since 5.8
 */
int QBluetoothDeviceDiscoveryAgent::lowEnergyDiscoveryTimeout() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->lowEnergySearchTimeout;
}

/*!
    \fn QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()

    This function returns the discovery methods supported by the current platform.
    It can be used to limit the scope of the device discovery.

    \since 5.8
*/

/*!
    Starts Bluetooth device discovery, if it is not already started.

    The deviceDiscovered() signal is emitted as each device is discovered. The finished() signal
    is emitted once device discovery is complete. The discovery utilizes the maximum set of
    supported discovery methods on the platform.

    \sa supportedDiscoveryMethods()
*/
void QBluetoothDeviceDiscoveryAgent::start()
{
    Q_D(QBluetoothDeviceDiscoveryAgent);
    if (!isActive())
        d->start(supportedDiscoveryMethods());
}

/*!
    Starts Bluetooth device discovery, if it is not already started and the provided
    \a methods are supported.
    The discovery \a methods limit the scope of the device search.
    For example, if the target service or device is a Bluetooth Low Energy device,
    this function could be used to limit the search to Bluetooth Low Energy devices and
    thereby reduces the discovery time significantly.

    \note \a methods only determines the type of discovery and does not imply
    the filtering of the results. For example, the search may still contain classic bluetooth devices
    despite \a methods being set to \l {QBluetoothDeviceDiscoveryAgent::LowEnergyMethod}
    {LowEnergyMethod} only. This may happen due to previously cached search results
    which may be incorporated into the search results.

    \note Some platforms (e.g. Windows) may also provide cached results for
    the devices that are not currently advertising. Other platforms
    (like \c {iOS}) only provide information about currently advertising
    devices. You can store the received device UUID (or Bluetooth address)
    upon first discovery, and then use it later to establish a connection
    directly (see
    \l {QBluetoothDeviceInfo::QBluetoothDeviceInfo(const QBluetoothUuid &uuid, const QString &name, quint32 classOfDevice)}
    {QBluetoothDeviceInfo}). This way the applications can omit the
    later device discovery phases. Using the Bluetooth address requires
    that the remote device's Bluetooth address does not change.

    \since 5.8
*/
void QBluetoothDeviceDiscoveryAgent::start(DiscoveryMethods methods)
{
    if (methods == NoMethod)
        return;

    DiscoveryMethods supported =
            QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods();

    Q_D(QBluetoothDeviceDiscoveryAgent);
    if (!((supported & methods) == methods)) {
        d->lastError = UnsupportedDiscoveryMethod;
        d->errorString = QBluetoothDeviceDiscoveryAgent::tr("One or more device discovery methods "
                                                            "are not supported on this platform");
        emit errorOccurred(d->lastError);
        return;
    }

    if (!isActive())
        d->start(methods);
}

/*!
    Stops Bluetooth device discovery.  The cancel() signal is emitted once the
    device discovery is canceled.  start() maybe called before the cancel signal is
    received.  Once start() has been called the cancel signal from the prior
    discovery will be discarded.
*/
void QBluetoothDeviceDiscoveryAgent::stop()
{
    Q_D(QBluetoothDeviceDiscoveryAgent);
    if (isActive() && d->lastError != InvalidBluetoothAdapterError)
        d->stop();
}

bool QBluetoothDeviceDiscoveryAgent::isActive() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->isActive();
}

/*!
    Returns the last error.

    Any possible previous errors are cleared upon restarting the discovery.
*/
QBluetoothDeviceDiscoveryAgent::Error QBluetoothDeviceDiscoveryAgent::error() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);

    return d->lastError;
}

/*!
    Returns a human-readable description of the last error.

    \sa error(), errorOccurred()
*/
QString QBluetoothDeviceDiscoveryAgent::errorString() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->errorString;
}

QT_END_NAMESPACE

#include "moc_qbluetoothdevicediscoveryagent.cpp"
