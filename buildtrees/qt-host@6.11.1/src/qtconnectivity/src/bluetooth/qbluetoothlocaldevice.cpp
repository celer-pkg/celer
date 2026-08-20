// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothlocaldevice.h"
#include "qbluetoothlocaldevice_p.h"
#include "qbluetoothaddress.h"

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice__Pairing)
QT_IMPL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::HostMode, QBluetoothLocalDevice__HostMode)
QT_IMPL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::Error, QBluetoothLocalDevice__Error)

/*!
    \class QBluetoothLocalDevice
    \inmodule QtBluetooth
    \brief The QBluetoothLocalDevice class enables access to the local Bluetooth
    device.

    \since 5.2

    QBluetoothLocalDevice provides functions for getting and setting the state of local Bluetooth
    devices.

    On iOS, this class cannot be used because the platform does not expose
    any data or API which may provide information on the local Bluetooth device.
*/

/*!
    \enum QBluetoothLocalDevice::Pairing

    This enum describes the pairing state between the two Bluetooth devices.

    \value Unpaired         The Bluetooth devices are not paired.
    \value Paired           The Bluetooth devices are paired. The system will prompt the user for
                            authorization when the remote device initiates a connection to the
                            local device.
    \value AuthorizedPaired The Bluetooth devices are paired. The system will not prompt the user
                            for authorization when the remote device initiates a connection to the
                            local device.
*/

/*!
  \enum QBluetoothLocalDevice::Error

  This enum describes errors that maybe returned

  \value NoError        No known error
  \value PairingError   Error in pairing
  \value [since 6.4] MissingPermissionsError  The operating system requests
                                              permissions which were not
                                              granted by the user.
  \value UnknownError   Unknown error

*/

/*!
    \enum QBluetoothLocalDevice::HostMode

    This enum describes the most of the local Bluetooth device.

    \value HostPoweredOff       Power off the device
    \value HostConnectable      Remote Bluetooth devices can connect to the local Bluetooth device
    if they have previously been paired with it or otherwise know its address. This powers up the
    device if it was powered off.
    \value HostDiscoverable     Remote Bluetooth devices can discover the presence of the local
    Bluetooth device. The device will also be connectable, and powered on. On Android, this mode can only be active
    for a maximum of 5 minutes.
    \value HostDiscoverableLimitedInquiry Remote Bluetooth devices can discover the presence of the local
     Bluetooth device when performing a limited inquiry. This should be used for locating services that are
     only made discoverable for a limited period of time. This can speed up discovery between gaming devices,
     as service discovery can be skipped on devices not in LimitedInquiry mode. In this mode, the device will
     be connectable and powered on, if required. This mode is is not supported on Android.

    \note On \macos, it is not possible to set the \l hostMode(). The
    reported host modes are limited to HostPoweredOff and HostConnectable.

    \note On Windows, it is not possible to set the \l hostMode() to
    HostDiscoverable or HostDiscoverableLimitedInquiry. Using these modes is
    equivalent to HostConnectable.

    \note Starting from Android 13 (API level 33) the HostPoweredOff state relies on
    non-public Android API as the public one has been deprecated, see
    (\l {https://developer.android.com/reference/android/bluetooth/BluetoothAdapter#disable()}
    {disable()}). This may change in a future version of Android.

    \note At least on Android 12 the device's Bluetooth visibility setting may dictate the result
    of setting either HostDiscoverable or HostConnectable. For example if the visibility is set
    \e off, it may not be possible to enter the HostDiscoverable mode, but HostConnectable will be
    used instead. This may change in future version of Android.

*/

void registerQBluetoothLocalDeviceMetaType()
{
    static bool initDone = false;
    if (!initDone) {
        qRegisterMetaType<QBluetoothLocalDevice::HostMode>();
        qRegisterMetaType<QBluetoothLocalDevice::Pairing>();
        qRegisterMetaType<QBluetoothLocalDevice::Error>();
        initDone = true;
    }
}

#ifndef QT_OSX_BLUETOOTH

/*!
    Destroys the QBluetoothLocalDevice.
*/
QBluetoothLocalDevice::~QBluetoothLocalDevice()
{
    delete d_ptr;
}

/*!
    Returns \c true if the QBluetoothLocalDevice represents an available local Bluetooth device;
    otherwise return false.

    If the local Bluetooth adapter represented by an instance of this class
    is removed from the system (e.g. removal of the underlying Bluetooth dongle)
    then this instance will become invalid. An already invalid QBluetoothLocalDevice instance
    remains invalid even if the same Bluetooth adapter is returned to
    the system.

//! [android-permissions-valid]
    \note Starting from Android 12 (API level 31), the construction of this class requires
    \l {https://developer.android.com/guide/topics/connectivity/bluetooth/permissions}
    {bluetooth runtime permissions} (\e BLUETOOTH_SCAN and \e BLUETOOTH_CONNECT). If the
    permissions are not granted, the device will not be valid.
//! [android-permissions-valid]

    \sa allDevices()
*/
bool QBluetoothLocalDevice::isValid() const
{
    if (d_ptr)
        return d_ptr->isValid();
    return false;
}

#endif

/*!
    \fn void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)

    Sets the host mode of this local Bluetooth device to \a mode.

    Some transitions such as turning the device on or off may take some time. Therefore
    subsequent calls should only be made once the \l hostModeStateChanged() signal
    has concluded the previous request. If this is ignored the result of such a series
    of calls is not well defined.

    \note Due to varying security policies on the supported platforms, this method may have
    differing behaviors on the various platforms. For example the system may ask the user for
    confirmation before turning Bluetooth on or off and not all host modes may be supported.
    On \macos, it is not possbile to programmatically change the \l hostMode().
    A user can only switch Bluetooth on/off in the System Preferences.
    On Windows this method \e must be called from the UI thread because it might
    require user confirmation.
    Please refer to the platform specific Bluetooth documentation for details.
*/

/*!
    \fn QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const

    Returns the current host mode of this local Bluetooth device. On \macos, it is either
    HostPoweredOff or HostConnectable.
*/

/*!
    \fn QBluetoothLocalDevice::name() const

    Returns the name assgined by the user to this Bluetooth device.
*/

/*!
    \fn QBluetoothLocalDevice::address() const

    Returns the MAC address of this Bluetooth device.

    \note On Android, this function always returns the constant
    value \c {02:00:00:00:00:00} as local address starting with Android 6.0.
    The programmatic access to the device's local MAC address was removed.
*/

/*!
    \fn QList<QBluetoothLocalDevice> QBluetoothLocalDevice::allDevices()

    Returns a list of all available local Bluetooth devices. On \macos, there is
    only the "default" local device.
*/

/*!
  \fn QBluetoothLocalDevice::powerOn()

  Powers on the device after returning it to the hostMode() state, if it was powered off.

  \note Due to varying security policies on the supported platforms, this method may have
  differing behaviors on the various platforms. For example
  the system may ask the user for confirmation before turning Bluetooth on or off.
  On \macos it is not possible to power on/off Bluetooth.
  Please refer to the platform specific Bluetooth documentation for details.
*/

/*!
  \fn QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent)
    Constructs a QBluetoothLocalDevice with \a parent.

  \include qbluetoothlocaldevice.cpp android-permissions-valid
  \sa isValid()
*/

/*!
  \fn QBluetoothLocalDevice::hostModeStateChanged(QBluetoothLocalDevice::HostMode state)
  The \a state of the host has transitioned to a different HostMode.
*/

/*!
  \fn QBluetoothLocalDevice::deviceConnected(const QBluetoothAddress &address)
  \since 5.3

  This signal is emitted when the local device establishes a connection to a remote device
  with \a address.

  \sa deviceDisconnected(), connectedDevices()
*/

/*!
  \fn QBluetoothLocalDevice::deviceDisconnected(const QBluetoothAddress &address)
  \since 5.3

  This signal is emitted when the local device disconnects from a remote Bluetooth device
  with \a address.

  \sa deviceConnected(), connectedDevices()
*/

/*!
  \fn QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
  \since 5.3

  Returns the list of connected devices. This list is different from the list of currently
  paired devices.

  On Android and \macos, it is not possible to retrieve a list of connected devices. It is only possible to
  listen to (dis)connect changes. For convenience, this class monitors all connect
  and disconnect events since its instanciation and returns the current list when calling this function.
  Therefore it is possible that this function returns an empty list shortly after creating an
  instance.

  \sa deviceConnected(), deviceDisconnected()
*/

/*!
  \fn QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const

  Returns the current bluetooth pairing status of \a address, if it's unpaired, paired, or paired and authorized.
*/

/*!
  \fn QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)

  Set the \a pairing status with \a address.  The results are returned by the signal, pairingFinished().

  On Android and \macos, AuthorizedPaired is not possible and will have the same behavior as Paired.
  On Windows the exact pairing mode decision is up to the operating system.

  On \macos, it is not possible to unpair a device. If Unpaired is requested, \l pairingFinished()
  is immediately emitted although the device remains paired. It is possible to request the pairing
  for a previously unpaired device. In addition \l AuthorizedPaired has the same behavior as \l Paired.

  Caution: creating a pairing may take minutes, and may require the user to acknowledge.
*/

/*!
  \fn QBluetoothLocalDevice::pairingFinished(const QBluetoothAddress &address,
  QBluetoothLocalDevice::Pairing pairing)

  Pairing or unpairing has completed with \a address. Current pairing status is in \a pairing.
  If the pairing request was not successful, this signal will not be emitted. The errorOccurred()
  signal is emitted if the pairing request failed. The signal is only ever emitted for pairing
  requests which have previously requested by calling \l requestPairing() of the current object
  instance.
*/

/*!
  \fn QBluetoothLocalDevice::errorOccurred(QBluetoothLocalDevice::Error error)
  Signal emitted if there's an exceptional \a error while pairing.

  \since 6.2
*/

/*!
  \fn QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent = 0)

  Construct new QBluetoothLocalDevice for \a address. If \a address is default constructed
  the resulting local device selects the local default device.

  \include qbluetoothlocaldevice.cpp android-permissions-valid
  \sa isValid()
*/

QT_END_NAMESPACE

#include "moc_qbluetoothlocaldevice.cpp"
