// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller.h"

#include "qlowenergycharacteristicdata.h"
#include "qlowenergyconnectionparameters.h"
#include "qlowenergydescriptordata.h"
#include "qlowenergyservicedata.h"

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtCore/QLoggingCategory>


#if QT_CONFIG(bluez) && !defined(QT_BLUEZ_NO_BTLE)
#include "bluez/bluez5_helper_p.h"
#include "qlowenergycontroller_bluezdbus_p.h"
#include "qlowenergycontroller_bluez_p.h"
#elif defined(QT_ANDROID_BLUETOOTH)
#include "qlowenergycontroller_android_p.h"
#include "android/androidutils_p.h"
#elif defined(QT_WINRT_BLUETOOTH)
#include "qtbluetoothglobal_p.h"
#include "qlowenergycontroller_winrt_p.h"
#elif defined(Q_OS_DARWIN)
#include "qlowenergycontroller_darwin_p.h"
#else
#include "qlowenergycontroller_dummy_p.h"
#endif

#include <algorithm>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QLowEnergyController::Error, QLowEnergyController__Error)
QT_IMPL_METATYPE_EXTERN_TAGGED(QLowEnergyController::ControllerState,
                               QLowEnergyController__ControllerState)
QT_IMPL_METATYPE_EXTERN_TAGGED(QLowEnergyController::RemoteAddressType,
                               QLowEnergyController__RemoteAddressType)
QT_IMPL_METATYPE_EXTERN_TAGGED(QLowEnergyController::Role, QLowEnergyController__Role)

Q_DECLARE_LOGGING_CATEGORY(QT_BT)
#if defined(QT_ANDROID_BLUETOOTH)
Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)
#endif

/*!
    \class QLowEnergyController
    \inmodule QtBluetooth
    \brief The QLowEnergyController class provides access to Bluetooth
    Low Energy Devices.

    \since 5.4

    QLowEnergyController acts as the entry point for Bluetooth Low Energy
    development.

    Bluetooth Low Energy defines two types of devices; the peripheral and
    the central. Each role performs a different task. The peripheral device
    provides data which is utilized by central devices. An example might be a
    humidity sensor which measures the moisture in a winter garden. A device
    such as a mobile phone might read the sensor's value and display it to the user
    in the greater context of all sensors in the same environment. In this case
    the sensor is the peripheral device and the mobile phone acts as the
    central device.

    A controller in the central role is created via the \l createCentral() factory method.
    Such an object essentially acts as a placeholder towards a remote Low Energy peripheral
    device, enabling features such as service discovery and state tracking.

    After having created a controller object in the central role, the first step is to establish
    a connection via \l connectToDevice().
    Once the connection has been established, the controller's \l state()
    changes to \l QLowEnergyController::ConnectedState and the \l connected()
    signal is emitted. It is important to mention that some platforms such as
    a BlueZ based Linux cannot maintain two connected instances of
    \l QLowEnergyController to the same remote device. In such cases the second
    call to \l connectToDevice() may fail. This limitation may disappear at some
    stage in the future. The \l disconnectFromDevice() function is used to break
    the existing connection.

    The second step after establishing the connection is to discover the services
    offered by the remote peripheral device. This process is started via
    \l discoverServices() and has finished once the \l discoveryFinished() signal
    has been emitted. The discovered services can be enumerated via
    \l services().

    The last step is to create service objects. The \l createServiceObject()
    function acts as factory for each service object and expects the service
    UUID as parameter. The calling context should take ownership of the returned
    \l QLowEnergyService instance.

    Any \l QLowEnergyService, \l QLowEnergyCharacteristic or
    \l QLowEnergyDescriptor instance which is later created from this controller's
    connection becomes invalid as soon as the controller disconnects from the
    remote Bluetooth Low Energy device.

    A controller in the peripheral role is created via the \l createPeripheral() factory method.
    Such an object acts as a peripheral device itself, enabling features such as advertising
    services and allowing clients to get notified about changes to characteristic values.

    After having created a controller object in the peripheral role, the first step is to
    populate the set of GATT services offered to client devices via calls to \l addService().
    Afterwards, one would call \l startAdvertising() to let the device broadcast some data
    and, depending on the type of advertising being done, also listen for incoming connections
    from GATT clients.

    \sa QLowEnergyService, QLowEnergyCharacteristic, QLowEnergyDescriptor
    \sa QLowEnergyAdvertisingParameters, QLowEnergyAdvertisingData
*/

/*!
    \enum QLowEnergyController::Error

    Indicates all possible error conditions found during the controller's
    existence.

    \value NoError                      No error has occurred.
    \value UnknownError                 An unknown error has occurred.
    \value UnknownRemoteDeviceError     The remote Bluetooth Low Energy device with the address passed to
                                        the constructor of this class cannot be found.
    \value NetworkError                 The attempt to read from or write to the
                                        remote device failed.
    \value InvalidBluetoothAdapterError The local Bluetooth device with the address passed to
                                        the constructor of this class cannot be found or
                                        there is no local Bluetooth device.
    \value [since 5.5] ConnectionError  The attempt to connect to the remote device failed.
    \value [since 5.7] AdvertisingError The attempt to start advertising failed.
    \value [since 5.10] RemoteHostClosedError   The remote device closed the connection.
    \value [since 5.14] AuthorizationError      The local Bluetooth device closed the connection
                                                due to insufficient authorization.
    \value [since 6.4] MissingPermissionsError  The operating system requests
                                                permissions which were not
                                                granted by the user.
    \value [since 6.5] RssiReadError An attempt to read RSSI (received signal strength indicator)
                                     of a remote device finished with error.
*/

/*!
    \enum QLowEnergyController::ControllerState

    Indicates the state of the controller object.

    \value UnconnectedState   The controller is not connected to a remote device.
    \value ConnectingState    The controller is attempting to connect to a remote device.
    \value ConnectedState     The controller is connected to a remote device.
    \value DiscoveringState   The controller is retrieving the list of services offered
                              by the remote device.
    \value DiscoveredState    The controller has discovered all services offered by the
                              remote device.
    \value ClosingState       The controller is about to be disconnected from the remote device.
    \value [since 5.7] AdvertisingState The controller is currently advertising data.
*/

/*!
    \enum QLowEnergyController::RemoteAddressType

    Indicates what type of Bluetooth address the remote device uses.

    \value PublicAddress The remote device uses a public Bluetooth address.
    \value RandomAddress A random address is a Bluetooth Low Energy security feature.
                         Peripherals using such addresses may frequently change their
                         Bluetooth address. This information is needed when trying to
                         connect to a peripheral.
 */

/*!
    \enum QLowEnergyController::Role

    Indicates the role of the controller object.

    \value CentralRole
       The controller acts as a client interacting with a remote device which is in the peripheral
       role. The controller can initiate connections, discover services and
       read and write characteristics.
    \value PeripheralRole
       The controller can be used to advertise services and handle incoming
       connections and client requests, acting as a GATT server. A remote device connected to
       the controller is in the central role.

   \sa QLowEnergyController::createCentral()
   \sa QLowEnergyController::createPeripheral()
   \since 5.7
   \note The peripheral role is not supported on Windows. In addition on Linux, handling the
         "Signed Write" ATT command on the server side requires BlueZ 5 and kernel version 3.7
         or newer.
 */


/*!
    \fn void QLowEnergyController::connected()

    This signal is emitted when the controller successfully connects to the remote
    Low Energy device (if the controller is in the \l CentralRole) or if a remote Low Energy
    device connected to the controller (if the controller is in the \l PeripheralRole).
    On iOS, macOS, and Android this signal is not reliable if the controller is in the
    \l PeripheralRole. On iOS and macOS the controller only guesses that some central
    connected to our peripheral as soon as this central tries to write/read a
    characteristic/descriptor. On Android the controller monitors all connected GATT
    devices. On Linux BlueZ DBus peripheral backend the remote is considered connected
    when it first reads/writes a characteristic or a descriptor.
*/

/*!
    \fn void QLowEnergyController::mtuChanged(int mtu)

    This signal is emitted as a result of a successful MTU change. \a mtu
    represents the new value.

    \note If the controller is in the \l PeripheralRole, the MTU value is negotiated
    for each client/central device individually. Therefore this signal can be emitted
    several times in a row for one or several devices.

    \sa mtu()
*/

/*!
    \fn void QLowEnergyController::rssiRead(qint16 rssi)

    This signal is emitted after successful read of RSSI (received
    signal strength indicator) for a connected remote device.
    \a rssi parameter represents the new value.

    \sa readRssi()
    \since 6.5
*/

/*!
    \fn void QLowEnergyController::disconnected()

    This signal is emitted when the controller disconnects from the remote
    Low Energy device or vice versa. On iOS and macOS this signal is unreliable
    if the controller is in the \l PeripheralRole. On Android the signal is emitted
    when the last connected device is disconnected. On BlueZ DBus backend the controller
    is considered disconnected when last client which has accessed the attributes has
    disconnected.
*/

/*!
    \fn void QLowEnergyController::stateChanged(ControllerState state)

    This signal is emitted when the controller's state changes. The new
    \a state can also be retrieved via \l state().

    \sa state()
*/

/*!
    \fn void QLowEnergyController::errorOccurred(QLowEnergyController::Error newError)

    This signal is emitted when an error occurs.
    The \a newError parameter describes the error that occurred.

    \sa error(), errorString()
    \since 6.2
*/

/*!
    \fn void QLowEnergyController::serviceDiscovered(const QBluetoothUuid &newService)

    This signal is emitted each time a new service is discovered. The
    \a newService parameter contains the UUID of the found service.

    This signal can only be emitted if the controller is in the \c CentralRole.

    \sa discoverServices(), discoveryFinished()
*/

/*!
    \fn void QLowEnergyController::discoveryFinished()

    This signal is emitted when the running service discovery finishes.
    The signal is not emitted if the discovery process finishes with
    an error.

    This signal can only be emitted if the controller is in the \l CentralRole.

    \sa discoverServices(), error()
*/

/*!
    \fn void QLowEnergyController::connectionUpdated(const QLowEnergyConnectionParameters &newParameters)

    This signal is emitted when the connection parameters change. This can happen as a result
    of calling \l requestConnectionUpdate() or due to other reasons, for instance because
    the other side of the connection requested new parameters. The new values can be retrieved
    from \a newParameters.

    \since 5.7
    \sa requestConnectionUpdate()
*/


void registerQLowEnergyControllerMetaType()
{
    static bool initDone = false;
    if (!initDone) {
        qRegisterMetaType<QLowEnergyController::ControllerState>();
        qRegisterMetaType<QLowEnergyController::Error>();
        qRegisterMetaType<QLowEnergyConnectionParameters>();
        qRegisterMetaType<QLowEnergyCharacteristic>();
        qRegisterMetaType<QLowEnergyDescriptor>();
        initDone = true;
    }
}

static QLowEnergyControllerPrivate *privateController(
        QLowEnergyController::Role role,
        const QBluetoothAddress& localDevice = QBluetoothAddress{})
{
#if QT_CONFIG(bluez) && !defined(QT_BLUEZ_NO_BTLE)
    // Central role
    // The minimum Bluez DBus version for central role is 5.42, otherwise we
    // fall back to kernel ATT backend. Application can also force the choice
    // with an environment variable (see bluetoothdVersion())
    //
    // Peripheral role
    // The DBus peripheral backend is the default, and for that we check the presence of
    // the required DBus APIs and bluez version. The application may opt-out of the DBus
    // peripheral role by setting the environment variable. The fall-back is the kernel ATT
    // backend
    //
    // ### Qt 7 consider removing the non-dbus bluez (kernel ATT) support

    QString adapterPathWithPeripheralSupport;
    if (role == QLowEnergyController::PeripheralRole
            && bluetoothdVersion() >= QVersionNumber(5, 56)
            && !qEnvironmentVariableIsSet("QT_BLUETOOTH_USE_KERNEL_PERIPHERAL")) {
        adapterPathWithPeripheralSupport = adapterWithDBusPeripheralInterface(localDevice);
    }

    if (role == QLowEnergyController::CentralRole
            && bluetoothdVersion() >= QVersionNumber(5, 42)) {
        qCDebug(QT_BT) << "Using BlueZ LE DBus API for central";
        return new QLowEnergyControllerPrivateBluezDBus();
    } else if (!adapterPathWithPeripheralSupport.isEmpty()) {
        qCDebug(QT_BT) << "Using BlueZ LE DBus API for peripheral";
        return new QLowEnergyControllerPrivateBluezDBus(adapterPathWithPeripheralSupport);
    } else {
        qCDebug(QT_BT) << "Using BlueZ kernel ATT interface for"
                       << (role == QLowEnergyController::CentralRole ? "central" : "peripheral");
        return new QLowEnergyControllerPrivateBluez();
    }
#elif defined(QT_ANDROID_BLUETOOTH)
    Q_UNUSED(role);
    Q_UNUSED(localDevice);
    return new QLowEnergyControllerPrivateAndroid();
#elif defined(QT_WINRT_BLUETOOTH)
    Q_UNUSED(role);
    Q_UNUSED(localDevice);
    return new QLowEnergyControllerPrivateWinRT();
#elif defined(Q_OS_DARWIN)
    Q_UNUSED(role);
    Q_UNUSED(localDevice);
    return new QLowEnergyControllerPrivateDarwin();
#else
    Q_UNUSED(role);
    Q_UNUSED(localDevice);
    return new QLowEnergyControllerPrivateCommon();
#endif
}

/*!
    Constructs a new instance of this class with \a parent.

    The \a remoteDevice must contain the address of the
    remote Bluetooth Low Energy device to which this object
    should attempt to connect later on.

    The connection is established via \a localDevice. If \a localDevice
    is invalid, the local default device is automatically selected. If
    \a localDevice specifies a local device that is not a local Bluetooth
    adapter, \l error() is set to \l InvalidBluetoothAdapterError once
    \l connectToDevice() is called.

    \note This is only supported on BlueZ
 */
QLowEnergyController::QLowEnergyController(
                            const QBluetoothDeviceInfo &remoteDevice,
                            const QBluetoothAddress &localDevice,
                            QObject *parent)
    : QObject(parent)
{
    d_ptr = privateController(CentralRole);

    Q_D(QLowEnergyController);
    d->q_ptr = this;
    d->role = CentralRole;
    d->deviceUuid = remoteDevice.deviceUuid();
    d->remoteDevice = remoteDevice.address();

    if (localDevice.isNull())
        d->localAdapter = QBluetoothLocalDevice().address();
    else
        d->localAdapter = localDevice;

    d->addressType = QLowEnergyController::PublicAddress;
    d->remoteName = remoteDevice.name();
    d->init();
}

/*!
   Returns a new object of this class that is in the \l CentralRole and has the
   parent object \a parent.
   The \a remoteDevice refers to the device that a connection will be established to later.

   The controller uses the local default Bluetooth adapter for the connection management.

   \sa QLowEnergyController::CentralRole
   \since 5.7
 */
QLowEnergyController *QLowEnergyController::createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                                          QObject *parent)
{
    return new QLowEnergyController(remoteDevice, QBluetoothAddress(), parent);
}

/*!
    Returns a new instance of this class with \a parent.

    The \a remoteDevice must contain the address of the remote Bluetooth Low
    Energy device to which this object should attempt to connect later on.

    The connection is established via \a localDevice. If \a localDevice is invalid,
    the local default device is automatically selected. If \a localDevice specifies
    a local device that is not a local Bluetooth adapter, \l error() is set to
    \l InvalidBluetoothAdapterError once \l connectToDevice() is called.

    Note that specifying the local device to be used for the connection is only
    possible when using BlueZ. All other platforms do not support this feature.

    \since 5.14
 */
QLowEnergyController *QLowEnergyController::createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                                          const QBluetoothAddress &localDevice,
                                                          QObject *parent)
{
    return new QLowEnergyController(remoteDevice, localDevice, parent);
}


/*!
   Returns a new object of this class that is in the \l PeripheralRole and has the
   parent object \a parent.
   Typically, the next steps are to add some services and finally
   call \l startAdvertising() on the returned object.

   The controller uses the local default Bluetooth adapter for the connection management.

   \sa QLowEnergyController::PeripheralRole
   \since 5.7
 */
QLowEnergyController *QLowEnergyController::createPeripheral(QObject *parent)
{
    return new QLowEnergyController(QBluetoothAddress(), parent);
}

/*!
    Returns a new object of this class that is in the \l PeripheralRole and has the
    parent object \a parent and is using \a localDevice.
    Typically, the next steps are to add some services and finally
    call \l startAdvertising() on the returned object.

    The peripheral is created on \a localDevice. If \a localDevice is invalid,
    the local default device is automatically selected. If \a localDevice specifies
    a local device that is not a local Bluetooth adapter, \l error() is set to
    \l InvalidBluetoothAdapterError.

    Selecting \a localDevice is only supported on Linux. On other platform,
    the parameter is ignored.

    \sa QLowEnergyController::PeripheralRole
    \since 6.2
 */
QLowEnergyController *QLowEnergyController::createPeripheral(const QBluetoothAddress &localDevice,
                                                             QObject *parent)
{
    return new QLowEnergyController(localDevice, parent);
}

QLowEnergyController::QLowEnergyController(const QBluetoothAddress &localDevice, QObject *parent)
    : QObject(parent)
{
    d_ptr = privateController(PeripheralRole, localDevice);

    Q_D(QLowEnergyController);
    d->q_ptr = this;
    d->role = PeripheralRole;

    if (localDevice.isNull())
        d->localAdapter = QBluetoothLocalDevice().address();
    else
        d->localAdapter = localDevice;

    d->init();
}

/*!
    Destroys the QLowEnergyController instance.
 */
QLowEnergyController::~QLowEnergyController()
{
    disconnectFromDevice(); //in case we were connected
    delete d_ptr;
}

/*!
  Returns the address of the local Bluetooth adapter being used for the
  communication.

  If this class instance was requested to use the default adapter
  but there was no default adapter when creating this
  class instance, the returned \l QBluetoothAddress will be null.

  \sa QBluetoothAddress::isNull()
 */
QBluetoothAddress QLowEnergyController::localAddress() const
{
    return d_ptr->localAdapter;
}

/*!
    Returns the address of the remote Bluetooth Low Energy device.

    For a controller in the \l CentralRole, this value will always be the one passed in when
    the controller object was created. For a controller in the \l PeripheralRole, this value
    is one of the currently connected client device addresses. This address will
    be invalid if the controller is not currently in the \l ConnectedState.
 */
QBluetoothAddress QLowEnergyController::remoteAddress() const
{
    return d_ptr->remoteDevice;
}

/*!
    Returns the unique identifier of the remote Bluetooth Low Energy device.

    On macOS/iOS/tvOS CoreBluetooth does not expose/accept hardware addresses for
    LE devices; instead developers are supposed to use unique 128-bit UUIDs, generated
    by CoreBluetooth. These UUIDS will stay constant for the same central <-> peripheral
    pair and we use them when connecting to a remote device. For a controller in the
    \l CentralRole, this value will always be the one passed in when the controller
    object was created. For a controller in the \l PeripheralRole, this value is invalid.

    \since 5.8
 */
QBluetoothUuid QLowEnergyController::remoteDeviceUuid() const
{
    return d_ptr->deviceUuid;
}

/*!
    Returns the name of the remote Bluetooth Low Energy device, if the controller is in the
    \l CentralRole. Otherwise the result is unspecified.

    \since 5.5
 */
QString QLowEnergyController::remoteName() const
{
    return d_ptr->remoteName;
}

/*!
    Returns the current state of the controller.

    \sa stateChanged()
 */
QLowEnergyController::ControllerState QLowEnergyController::state() const
{
    return d_ptr->state;
}

/*!
    Returns the type of \l remoteAddress(). By default, this value is initialized
    to \l PublicAddress.

    \sa setRemoteAddressType()
 */
QLowEnergyController::RemoteAddressType QLowEnergyController::remoteAddressType() const
{
    return d_ptr->addressType;
}

/*!
    Sets the remote address \a type. The type is required to connect
    to the remote Bluetooth Low Energy device.

    This attribute is only required to be set on Linux/BlueZ systems with older
    Linux kernels (v3.3 or lower), or if CAP_NET_ADMIN is not set for the executable.
    The default value of the attribute is \l RandomAddress.

    \note All other platforms handle this flag transparently and therefore applications
    can ignore it entirely. On Linux, the address type flag is not directly exposed
    by BlueZ although some use cases do require this information. The only way to detect
    the flag is via the Linux kernel's Bluetooth Management API (kernel
    version 3.4+ required). This API requires CAP_NET_ADMIN capabilities though. If the
    local QtBluetooth process has this capability set QtBluetooth will use the API. This
    assumes that \l QBluetoothDeviceDiscoveryAgent was used prior to calling
    \l QLowEnergyController::connectToDevice().
 */
void QLowEnergyController::setRemoteAddressType(
                    QLowEnergyController::RemoteAddressType type)
{
    d_ptr->addressType = type;
}

/*!
    Connects to the remote Bluetooth Low Energy device.

    This function does nothing if the controller's \l state()
    is not equal to \l UnconnectedState. The \l connected() signal is emitted
    once the connection is successfully established.

    On Linux/BlueZ systems, it is not possible to connect to the same
    remote device using two instances of this class. The second call
    to this function may fail with an error. This limitation may
    be removed in future releases.

    \sa disconnectFromDevice()
 */
void QLowEnergyController::connectToDevice()
{
    Q_D(QLowEnergyController);

    if (role() != CentralRole) {
        qCWarning(QT_BT) << "Connection can only be established while in central role";
        return;
    }

    if (!d->isValidLocalAdapter()) {
        qCWarning(QT_BT) << "connectToDevice() LE controller has invalid adapter";
        d->setError(QLowEnergyController::InvalidBluetoothAdapterError);
        return;
    }

    if (state() != QLowEnergyController::UnconnectedState)
        return;

    d->connectToDevice();
}

/*!
    Disconnects from the remote device.

    Any \l QLowEnergyService, \l QLowEnergyCharacteristic or \l QLowEnergyDescriptor
    instance that resulted from the current connection is automatically invalidated.
    Once any of those objects become invalid they remain invalid even if this
    controller object reconnects.

    This function does nothing if the controller is in the \l UnconnectedState.

    If the controller is in the peripheral role, it stops advertising and removes
    all services which have previously been added via \l addService().
    To reuse the QLowEnergyController instance the application must re-add services
    and restart the advertising mode by calling \l startAdvertising().

    \sa connectToDevice()
 */
void QLowEnergyController::disconnectFromDevice()
{
    Q_D(QLowEnergyController);

    if (state() == QLowEnergyController::UnconnectedState)
        return;

    d->invalidateServices();
    d->disconnectFromDevice();
}

/*!
    Initiates the service discovery process.

    The discovery progress is indicated via the \l serviceDiscovered() signal.
    The \l discoveryFinished() signal is emitted when the process has finished.

    If the controller instance is not connected or the controller has performed
    the service discovery already this function will do nothing.

    \note Some platforms internally cache the service list of a device
    which was discovered in the past. This can be problematic if the remote device
    changed its list of services or their inclusion tree. If this behavior is a
    problem, the best workaround is to temporarily turn Bluetooth off. This
    causes a reset of the cache data. Currently Android exhibits such a
    cache behavior.
 */
void QLowEnergyController::discoverServices()
{
    Q_D(QLowEnergyController);

    if (d->role != CentralRole) {
        qCWarning(QT_BT) << "Cannot discover services in peripheral role";
        return;
    }
    if (d->state != QLowEnergyController::ConnectedState)
        return;

    d->setState(QLowEnergyController::DiscoveringState);
    d->discoverServices();
}

/*!
    Returns the list of services offered by the remote device, if the controller is in
    the \l CentralRole. Otherwise, the result is unspecified.

    The list contains all primary and secondary services.

    \sa createServiceObject()
 */
QList<QBluetoothUuid> QLowEnergyController::services() const
{
    return d_ptr->serviceList.keys();
}

/*!
    Creates an instance of the service represented by \a serviceUuid.
    The \a serviceUuid parameter must have been obtained via
    \l services().

    The caller takes ownership of the returned pointer and may pass
    a \a parent parameter as default owner.

    This function returns a null pointer if no service with
    \a serviceUuid can be found on the remote device or the controller
    is disconnected.

    This function can return instances for secondary services
    too. The include relationships between services can be expressed
    via \l QLowEnergyService::includedServices().

    If this function is called multiple times using the same service UUID,
    the returned \l QLowEnergyService instances share their internal
    data. Therefore if one of the instances initiates the discovery
    of the service details, the other instances automatically
    transition into the discovery state too.

    \sa services()
 */
QLowEnergyService *QLowEnergyController::createServiceObject(
        const QBluetoothUuid &serviceUuid, QObject *parent)
{
    Q_D(QLowEnergyController);

    QLowEnergyService *service = nullptr;

    ServiceDataMap::const_iterator it = d->serviceList.constFind(serviceUuid);
    if (it != d->serviceList.constEnd()) {
        const QSharedPointer<QLowEnergyServicePrivate> &serviceData = it.value();

        service = new QLowEnergyService(serviceData, parent);
    }

    return service;
}

/*!
   Starts advertising the data given in \a advertisingData and \a scanResponseData, using
   the parameters set in \a parameters. The controller has to be in the \l PeripheralRole.
   If \a parameters indicates that the advertisement should be connectable, then this function
   also starts listening for incoming client connections.

   Providing \a scanResponseData is not required, as it is not applicable for certain
   configurations of \c parameters. \a advertisingData and \a scanResponseData are limited
   to 31 byte user data. If, for example, several 128bit uuids are added to \a advertisingData,
   the advertised packets may not contain all uuids. The existing limit may have caused the truncation
   of uuids. In such cases \a scanResponseData may be used for additional information.

   On BlueZ DBus backend BlueZ decides if, and which data, to use in a scan response. Therefore
   all advertisement data is recommended to set in the main \a advertisingData parameter. If both
   advertisement and scan response data is set, the scan response data is given precedence.

   If this object is currently not in the \l UnconnectedState, nothing happens.

   \since 5.7
   \sa stopAdvertising()
 */
void QLowEnergyController::startAdvertising(const QLowEnergyAdvertisingParameters &parameters,
                                            const QLowEnergyAdvertisingData &advertisingData,
                                            const QLowEnergyAdvertisingData &scanResponseData)
{
    Q_D(QLowEnergyController);
    if (role() != PeripheralRole) {
        qCWarning(QT_BT) << "Cannot start advertising in central role" << state();
        return;
    }
    if (state() != UnconnectedState) {
        qCWarning(QT_BT) << "Cannot start advertising in state" << state();
        return;
    }
    d->startAdvertising(parameters, advertisingData, scanResponseData);
}

/*!
   Stops advertising, if this object is currently in the advertising state.

   The controller has to be in the \l PeripheralRole for this function to work.
   It does not invalidate services which have previously been added via \l addService().

   \since 5.7
   \sa startAdvertising()
 */
void QLowEnergyController::stopAdvertising()
{
    Q_D(QLowEnergyController);
    if (state() != AdvertisingState) {
        qCDebug(QT_BT) << "stopAdvertising called in state" << state();
        return;
    }
    d->stopAdvertising();
}

/*!
  Constructs and returns a \l QLowEnergyService object with \a parent from \a service.
  The controller must be in the \l PeripheralRole and in the \l UnconnectedState. The \a service
  object must be valid.

  \note Once the peripheral instance is disconnected from the remote central device or
  if \l disconnectFromDevice() is manually called, every service definition that was
  previously added via this function is removed from the peripheral. Therefore this function
  must be called again before re-advertising this peripheral controller instance. The described
  behavior is connection specific and therefore not dependent on whether \l stopAdvertising()
  was called.

  \since 5.7
  \sa stopAdvertising(), disconnectFromDevice(), QLowEnergyServiceData::addIncludedService
 */
QLowEnergyService *QLowEnergyController::addService(const QLowEnergyServiceData &service,
                                                    QObject *parent)
{
    if (role() != PeripheralRole) {
        qCWarning(QT_BT) << "Services can only be added in the peripheral role";
        return nullptr;
    }
    if (state() != UnconnectedState) {
        qCWarning(QT_BT) << "Services can only be added in unconnected state";
        return nullptr;
    }
    if (!service.isValid()) {
        qCWarning(QT_BT) << "Not adding invalid service";
        return nullptr;
    }

#if defined(QT_ANDROID_BLUETOOTH)
    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "addService() failed due to missing permissions";
        return nullptr;
    }
#endif

    Q_D(QLowEnergyController);
    QLowEnergyService *newService = d->addServiceHelper(service);
    if (newService)
        newService->setParent(parent);

    return newService;
}


/*!
  Requests the controller to update the connection according to \a parameters.
  If the request is successful, the \l connectionUpdated() signal will be emitted
  with the actual new parameters. See the  \l QLowEnergyConnectionParameters class for more
  information on connection parameters.

  Android only indirectly permits the adjustment of this parameter set.
  The connection parameters are separated into three categories (high, low & balanced priority).
  Each category implies a pre-configured set of values for
  \l QLowEnergyConnectionParameters::minimumInterval(),
  \l QLowEnergyConnectionParameters::maximumInterval() and
  \l QLowEnergyConnectionParameters::latency(). Although the connection request is an asynchronous
  operation, Android does not provide a callback stating the result of the request. This is
  an acknowledged Android bug. Due to this bug Android does not emit the \l connectionUpdated()
  signal.

  \note Currently, this functionality is only implemented on Linux kernel backend and Android.

  \sa connectionUpdated()
  \since 5.7
 */
void QLowEnergyController::requestConnectionUpdate(const QLowEnergyConnectionParameters &parameters)
{
    switch (state()) {
    case ConnectedState:
    case DiscoveredState:
    case DiscoveringState:
        d_ptr->requestConnectionUpdate(parameters);
        break;
    default:
        qCWarning(QT_BT) << "Connection update request only possible in connected state";
    }
}

/*!
    Returns the last occurred error or \l NoError.
*/
QLowEnergyController::Error QLowEnergyController::error() const
{
    return d_ptr->error;
}

/*!
    Returns a textual representation of the last occurred error.
    The string is translated.
*/
QString QLowEnergyController::errorString() const
{
    return d_ptr->errorString;
}

/*!
   Returns the role that this controller object is in.

   The role is determined when constructing a QLowEnergyController instance
   using \l createCentral() or \l createPeripheral().

   \since 5.7
 */
QLowEnergyController::Role QLowEnergyController::role() const
{
    return d_ptr->role;
}

/*!
   Returns the MTU size.

   During connection setup, the ATT MTU size is negotiated.
   This method provides the result of this negotiation.
   It can be used to optimize packet sizes in some situations.
   The maximum amount of data which can be transferred in a single
   packet is \b {mtu-3} bytes. 3 bytes are required for the ATT
   protocol header.

   Before the connection setup and MTU negotiation, the
   default value of \c 23 will be returned.

   Not every platform exposes the MTU value. On those platforms (e.g. Linux)
   this function always returns \c -1.

   If the controller is in the \l PeripheralRole, there might be several
   central devices connected to it. In those cases this function returns
   the MTU of the last connection that was negotiated.

   \since 6.2
 */
int QLowEnergyController::mtu() const
{
    return d_ptr->mtu();
}

/*!
    readRssi() reads RSSI (received signal strength indicator) for a connected remote device.
    If the read was successful, the RSSI is then reported by rssiRead() signal.

    \note Prior to calling readRssi(), this controller must be connected to a peripheral.
    This controller must be created using createCentral().

    \note In case Bluetooth backend you are using does not support reading RSSI,
    the errorOccurred() signal is emitted with an error code QLowEnergyController::RssiReadError.
    At the moment platforms supporting reading RSSI include Android, iOS and macOS.

    \sa rssiRead(), connectToDevice(), state(), createCentral(), errorOccurred()
    \since 6.5
*/
void QLowEnergyController::readRssi()
{
    if (role() != CentralRole) {
        qCWarning(QT_BT, "Invalid role (peripheral), cannot read RSSI");
        return d_ptr->setError(RssiReadError); // This also emits.
    }

    switch (state()) {
    case UnconnectedState:
    case ConnectingState:
    case ClosingState:
        qCWarning(QT_BT, "Cannot read RSSI while not in 'Connected' state, connect first");
        return d_ptr->setError(RssiReadError); // Will emit.
    default:
        d_ptr->readRssi();
    }
}

QT_END_NAMESPACE

#include "moc_qlowenergycontroller.cpp"
