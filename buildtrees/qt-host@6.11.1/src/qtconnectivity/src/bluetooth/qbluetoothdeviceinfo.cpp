// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdeviceinfo.h"
#include "qbluetoothdeviceinfo_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QBluetoothDeviceInfo)
#ifdef QT_WINRT_BLUETOOTH
QT_IMPL_METATYPE_EXTERN_TAGGED(QBluetoothDeviceInfo::Fields, QBluetoothDeviceInfo__Fields)
#endif

/*!
    \class QBluetoothDeviceInfo
    \inmodule QtBluetooth
    \brief The QBluetoothDeviceInfo class stores information about the Bluetooth
    device.

    \since 5.2

    QBluetoothDeviceInfo provides information about a Bluetooth device's name, address and class of device.
*/

/*!
    \enum QBluetoothDeviceInfo::MajorDeviceClass

    This enum describes a Bluetooth device's major device class.

    \value MiscellaneousDevice  A miscellaneous device.
    \value ComputerDevice       A computer device or PDA.
    \value PhoneDevice          A telephone device.
    \value NetworkDevice        A device that provides access to a local area network (since Qt 5.13).
    \value AudioVideoDevice     A device capable of playback or capture of audio and/or video.
    \value PeripheralDevice     A peripheral device such as a keyboard, mouse, and so on.
    \value ImagingDevice        An imaging device such as a display, printer, scanner or camera.
    \value WearableDevice       A wearable device such as a watch or pager.
    \value ToyDevice            A toy.
    \value HealthDevice         A health reated device such as heart rate or temperature monitor.
    \value UncategorizedDevice  A device that does not fit into any of the other device classes.
*/

/*!
    \enum QBluetoothDeviceInfo::Field

    This enum is used in conjuntion with the \l QBluetoothDeviceDiscoveryAgent::deviceUpdated() signal
    and indicates the field that changed.

    \value None             None of the values changed.
    \value RSSI             The \l rssi() value of the device changed.
    \value ManufacturerData The \l manufacturerData() field changed
    \value ServiceData      The \l serviceData() field changed
    \value All              Matches every possible field.

    \since 5.12
*/

/*!
    \enum QBluetoothDeviceInfo::MinorMiscellaneousClass

    This enum describes the minor device classes for miscellaneous Bluetooth devices.

    \value UncategorizedMiscellaneous   An uncategorized miscellaneous device.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorComputerClass

    This enum describes the minor device classes for computer devices.

    \value UncategorizedComputer        An uncategorized computer device.
    \value DesktopComputer              A desktop computer.
    \value ServerComputer               A server computer.
    \value LaptopComputer               A laptop computer.
    \value HandheldClamShellComputer    A clamshell handheld computer or PDA.
    \value HandheldComputer             A handheld computer or PDA.
    \value WearableComputer             A wearable computer.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorPhoneClass

    This enum describes the minor device classes for phone devices.

    \value UncategorizedPhone               An uncategorized phone device.
    \value CellularPhone                    A cellular phone.
    \value CordlessPhone                    A cordless phone.
    \value SmartPhone                       A smart phone.
    \value WiredModemOrVoiceGatewayPhone    A wired modem or voice gateway.
    \value CommonIsdnAccessPhone            A device that provides ISDN access.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorNetworkClass

    This enum describes the minor device classes for local area network access devices. Local area
    network access devices use the minor device class to specify the current network utilization.

    \value NetworkFullService       100% of the total bandwidth is available.
    \value NetworkLoadFactorOne     0 - 17% of the total bandwidth is currently being used.
    \value NetworkLoadFactorTwo     17 - 33% of the total bandwidth is currently being used.
    \value NetworkLoadFactorThree   33 - 50% of the total bandwidth is currently being used.
    \value NetworkLoadFactorFour    50 - 67% of the total bandwidth is currently being used.
    \value NetworkLoadFactorFive    67 - 83% of the total bandwidth is currently being used.
    \value NetworkLoadFactorSix     83 - 99% of the total bandwidth is currently being used.
    \value NetworkNoService         No network service available.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorAudioVideoClass

    This enum describes the minor device classes for audio/video devices.

    \value UncategorizedAudioVideoDevice    An uncategorized audio/video device.
    \value WearableHeadsetDevice            A wearable headset device.
    \value HandsFreeDevice                  A handsfree device.
    \value Microphone                       A microphone.
    \value Loudspeaker                      A loudspeaker.
    \value Headphones                       Headphones.
    \value PortableAudioDevice              A portable audio device.
    \value CarAudio                         A car audio device.
    \value SetTopBox                        A settop box.
    \value HiFiAudioDevice                  A HiFi audio device.
    \value Vcr                              A video cassette recorder.
    \value VideoCamera                      A video camera.
    \value Camcorder                        A video camera.
    \value VideoMonitor                     A video monitor.
    \value VideoDisplayAndLoudspeaker       A video display with built-in loudspeaker.
    \value VideoConferencing                A video conferencing device.
    \value GamingDevice                     A gaming device.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorPeripheralClass

    This enum describes the minor device classes for peripheral devices.

    \value UncategorizedPeripheral              An uncategorized peripheral device.
    \value KeyboardPeripheral                   A keyboard.
    \value PointingDevicePeripheral             A pointing device, for example a mouse.
    \value KeyboardWithPointingDevicePeripheral A keyboard with built-in pointing device.
    \value JoystickPeripheral                   A joystick.
    \value GamepadPeripheral                    A game pad.
    \value RemoteControlPeripheral              A remote control.
    \value SensingDevicePeripheral              A sensing device.
    \value DigitizerTabletPeripheral            A digitizer tablet peripheral.
    \value CardReaderPeripheral                 A card reader peripheral.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorImagingClass

    This enum describes the minor device classes for imaging devices.

    \value UncategorizedImagingDevice   An uncategorized imaging device.
    \value ImageDisplay                 A device capable of displaying images.
    \value ImageCamera                  A camera.
    \value ImageScanner                 An image scanner.
    \value ImagePrinter                 A printer.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorWearableClass

    This enum describes the minor device classes for wearable devices.

    \value UncategorizedWearableDevice  An uncategorized wearable device.
    \value WearableWristWatch           A wristwatch.
    \value WearablePager                A pager.
    \value WearableJacket               A jacket.
    \value WearableHelmet               A helmet.
    \value WearableGlasses              A pair of glasses.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorToyClass

    This enum describes the minor device classes for toy devices.

    \value UncategorizedToy     An uncategorized toy.
    \value ToyRobot             A toy robot.
    \value ToyVehicle           A toy vehicle.
    \value ToyDoll              A toy doll or action figure.
    \value ToyController        A controller.
    \value ToyGame              A game.
*/

/*!
    \enum QBluetoothDeviceInfo::MinorHealthClass

    This enum describes the minor device classes for health devices.

    \value UncategorizedHealthDevice    An uncategorized health device.
    \value HealthBloodPressureMonitor   A blood pressure monitor.
    \value HealthThermometer            A Thermometer.
    \value HealthWeightScale            A scale.
    \value HealthGlucoseMeter           A glucose meter.
    \value HealthPulseOximeter          A blood oxygen saturation meter.
    \value HealthDataDisplay            A data display.
    \value HealthStepCounter            A pedometer.
*/

/*!
    \enum QBluetoothDeviceInfo::ServiceClass

    This enum describes the service class of the Bluetooth device. The service class is used as a
    rudimentary form of service discovery. It is meant to provide a list of the types
    of services that the device might provide.

    \value NoService                The device does not provide any services.
    \value PositioningService       The device provides positioning services.
    \value NetworkingService        The device provides networking services.
    \value RenderingService         The device provides rendering services.
    \value CapturingService         The device provides capturing services.
    \value ObjectTransferService    The device provides object transfer services.
    \value AudioService             The device provides audio services.
    \value TelephonyService         The device provides telephony services.
    \value InformationService       The device provides information services.
    \value AllServices              The device provides services of all types.
*/

/*!
    \enum QBluetoothDeviceInfo::CoreConfiguration
    \since 5.4

    This enum describes the configuration of the device.

    \value UnknownCoreConfiguration             The type of the Bluetooth device cannot be determined.
    \value BaseRateCoreConfiguration            The device is a standard Bluetooth device.
    \value BaseRateAndLowEnergyCoreConfiguration    The device is a Bluetooth Smart device with support
                                                for standard and Low Energy device.
    \value LowEnergyCoreConfiguration           The device is a Bluetooth Low Energy device.
*/
QBluetoothDeviceInfoPrivate::QBluetoothDeviceInfoPrivate()
{
}

/*!
    Constructs an invalid QBluetoothDeviceInfo object.
*/
QBluetoothDeviceInfo::QBluetoothDeviceInfo() :
    d_ptr(new QBluetoothDeviceInfoPrivate)
{
}

/*!
    Constructs a QBluetoothDeviceInfo object with Bluetooth address \a address, device name
    \a name and the encoded class of device \a classOfDevice.

    The \a classOfDevice parameter is encoded in the following format

    \table
        \header \li Bits \li Size \li Description
        \row \li 0 - 1 \li 2 \li Unused, set to 0.
        \row \li 2 - 7 \li 6 \li Minor device class.
        \row \li 8 - 12 \li 5 \li Major device class.
        \row \li 13 - 23 \li 11 \li Service class.
    \endtable
*/
QBluetoothDeviceInfo::QBluetoothDeviceInfo(const QBluetoothAddress &address, const QString &name,
                                           quint32 classOfDevice) :
    d_ptr(new QBluetoothDeviceInfoPrivate)
{
    Q_D(QBluetoothDeviceInfo);

    d->address = address;
    d->name = name;

    d->minorDeviceClass = static_cast<quint8>((classOfDevice >> 2) & 0x3f);
    d->majorDeviceClass = static_cast<MajorDeviceClass>((classOfDevice >> 8) & 0x1f);
    d->serviceClasses = static_cast<ServiceClasses>((classOfDevice >> 13) & 0x7ff);

    d->valid = true;
    d->cached = false;
    d->rssi = 0;
}

/*!
    Constructs a QBluetoothDeviceInfo object with unique \a uuid, device name
    \a name and the encoded class of device \a classOfDevice.

    This constructor is required for Low Energy devices on \macos and iOS. CoreBluetooth
    API hides addresses and provides unique UUIDs to identify a device. This UUID is
    not the same thing as a service UUID and is required to work later with CoreBluetooth API
    and discovered devices.

    \since 5.5
*/
QBluetoothDeviceInfo::QBluetoothDeviceInfo(const QBluetoothUuid &uuid, const QString &name,
                                           quint32 classOfDevice) :
    d_ptr(new QBluetoothDeviceInfoPrivate)
{
    Q_D(QBluetoothDeviceInfo);

    d->name = name;
    d->deviceUuid = uuid;

    d->minorDeviceClass = static_cast<quint8>((classOfDevice >> 2) & 0x3f);
    d->majorDeviceClass = static_cast<MajorDeviceClass>((classOfDevice >> 8) & 0x1f);
    d->serviceClasses = static_cast<ServiceClasses>((classOfDevice >> 13) & 0x7ff);

    d->valid = true;
    d->cached = false;
    d->rssi = 0;
}

/*!
    Constructs a QBluetoothDeviceInfo that is a copy of \a other.
*/
QBluetoothDeviceInfo::QBluetoothDeviceInfo(const QBluetoothDeviceInfo &other) :
    d_ptr(new QBluetoothDeviceInfoPrivate)
{
    *this = other;
}

/*!
    Destroys the QBluetoothDeviceInfo.
*/
QBluetoothDeviceInfo::~QBluetoothDeviceInfo()
{
    delete d_ptr;
}

/*!
    Returns true if the QBluetoothDeviceInfo object is valid, otherwise returns false.
*/
bool QBluetoothDeviceInfo::isValid() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->valid;
}

/*!
  Returns the signal strength when the device was last scanned
  */
qint16 QBluetoothDeviceInfo::rssi() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->rssi;
}

/*!
  Set the \a signal strength value, used internally.
  */
void QBluetoothDeviceInfo::setRssi(qint16 signal)
{
    Q_D(QBluetoothDeviceInfo);
    d->rssi = signal;
}

/*!
    Makes a copy of the \a other and assigns it to this QBluetoothDeviceInfo object.
*/
QBluetoothDeviceInfo &QBluetoothDeviceInfo::operator=(const QBluetoothDeviceInfo &other)
{
    Q_D(QBluetoothDeviceInfo);

    d->address = other.d_func()->address;
    d->name = other.d_func()->name;
    d->minorDeviceClass = other.d_func()->minorDeviceClass;
    d->majorDeviceClass = other.d_func()->majorDeviceClass;
    d->serviceClasses = other.d_func()->serviceClasses;
    d->valid = other.d_func()->valid;
    d->cached = other.d_func()->cached;
    d->serviceUuids = other.d_func()->serviceUuids;
    d->manufacturerData = other.d_func()->manufacturerData;
    d->serviceData = other.d_func()->serviceData;
    d->rssi = other.d_func()->rssi;
    d->deviceCoreConfiguration = other.d_func()->deviceCoreConfiguration;
    d->deviceUuid = other.d_func()->deviceUuid;

    return *this;
}

/*!
    \fn bool QBluetoothDeviceInfo::operator==(const QBluetoothDeviceInfo &a, const QBluetoothDeviceInfo &b)
    \brief Returns \c true if the two QBluetoothDeviceInfo objects \a a and \a b are equal.
*/

/*!
    \fn bool QBluetoothDeviceInfo::operator!=(const QBluetoothDeviceInfo &a,
                                              const QBluetoothDeviceInfo &b)
    \brief Returns \c true if the two QBluetoothDeviceInfo objects \a a and \a b are not equal.
*/

/*!
    \brief Returns true if the \a other QBluetoothDeviceInfo object and this are identical.
    \internal
*/

bool QBluetoothDeviceInfo::equals(const QBluetoothDeviceInfo &a, const QBluetoothDeviceInfo &b)
{
    if (a.d_func()->cached != b.d_func()->cached)
        return false;
    if (a.d_func()->valid != b.d_func()->valid)
        return false;
    if (a.d_func()->majorDeviceClass != b.d_func()->majorDeviceClass)
        return false;
    if (a.d_func()->minorDeviceClass != b.d_func()->minorDeviceClass)
        return false;
    if (a.d_func()->serviceClasses != b.d_func()->serviceClasses)
        return false;
    if (a.d_func()->name != b.d_func()->name)
        return false;
    if (a.d_func()->address != b.d_func()->address)
        return false;
    if (a.d_func()->serviceUuids.size() != b.d_func()->serviceUuids.size())
        return false;
    if (a.d_func()->serviceUuids != b.d_func()->serviceUuids)
        return false;
    if (a.d_func()->manufacturerData != b.d_func()->manufacturerData)
        return false;
    if (a.d_func()->serviceData != b.d_func()->serviceData)
        return false;
    if (a.d_func()->deviceCoreConfiguration != b.d_func()->deviceCoreConfiguration)
        return false;
    if (a.d_func()->deviceUuid != b.d_func()->deviceUuid)
        return false;

    return true;
}

/*!
    Returns the address of the device.

    \note On iOS and \macos this address is invalid. Instead \l deviceUuid() should be used.
    Those two platforms do not expose Bluetooth addresses for found Bluetooth devices
    and utilize unique device identifiers.

    \sa deviceUuid()
*/
QBluetoothAddress QBluetoothDeviceInfo::address() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->address;
}

/*!
    Returns the name assigned to the device.
*/
QString QBluetoothDeviceInfo::name() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->name;
}

/*!
  Sets the \a name of the device.

  \since 6.2
 */
void QBluetoothDeviceInfo::setName(const QString &name)
{
    Q_D(QBluetoothDeviceInfo);

    d->name = name;
}

/*!
    Returns the service class of the device.
*/
QBluetoothDeviceInfo::ServiceClasses QBluetoothDeviceInfo::serviceClasses() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->serviceClasses;
}

/*!
    Returns the major device class of the device.
*/
QBluetoothDeviceInfo::MajorDeviceClass QBluetoothDeviceInfo::majorDeviceClass() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->majorDeviceClass;
}

/*!
    Returns the minor device class of the device. The actual information
    is context dependent on the value of \l majorDeviceClass().

    \sa MinorAudioVideoClass, MinorComputerClass, MinorHealthClass, MinorImagingClass,
    MinorMiscellaneousClass, MinorNetworkClass, MinorPeripheralClass, MinorPhoneClass,
    MinorToyClass, MinorWearableClass
*/
quint8 QBluetoothDeviceInfo::minorDeviceClass() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->minorDeviceClass;
}

/*!
    Sets the list of service UUIDs to \a uuids.
    \since 5.13
 */
void QBluetoothDeviceInfo::setServiceUuids(const QList<QBluetoothUuid> &uuids)
{
    Q_D(QBluetoothDeviceInfo);
    d->serviceUuids = uuids;
}

/*!
    Returns the list of service UUIDs supported by the device. Most commonly this
    list of UUIDs represents custom service UUIDs or a service UUID value specified
    by \l QBluetoothUuid::ServiceClassUuid.

    \sa serviceUuids()
    \since 6.0
*/
QList<QBluetoothUuid> QBluetoothDeviceInfo::serviceUuids() const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->serviceUuids;
}

/*!
    Returns all manufacturer IDs from advertisement packets attached to this device information.

    \sa manufacturerData(), setManufacturerData()

    \since 5.12
 */
QList<quint16> QBluetoothDeviceInfo::manufacturerIds() const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->manufacturerData.keys().toList();
}

/*!
    Returns the data associated with the given \a manufacturerId.

    Manufacturer data is defined by
    the Supplement to the Bluetooth Core Specification and consists of two segments:

    \list
    \li Manufacturer specific identifier code from the
    \l {https://www.bluetooth.com/specifications/assigned-numbers} {Assigned Numbers}
    Company Identifiers document
    \li Sequence of arbitrary data octets
    \endlist

    The interpretation of the data octets is defined by the manufacturer
    specified by the company identifier.

    \note The remote device may provide multiple data entries per manufacturerId.
    This function only returns the first entry. If all entries are needed use
    \l manufacturerData() which returns a multi hash.

    \sa manufacturerIds(), setManufacturerData()
    \since 5.12
 */
QByteArray QBluetoothDeviceInfo::manufacturerData(quint16 manufacturerId) const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->manufacturerData.value(manufacturerId);
}

/*!
    Sets the advertised manufacturer \a data for the given \a manufacturerId.
    Returns \c true if it was inserted, \c false if it was already known.

    Since Qt 5.14, different values for \a data and the same \a manufacturerId no longer
    replace each other but are accumulated for the duration of a device scan.

    \sa manufacturerData
    \since 5.12
*/
bool QBluetoothDeviceInfo::setManufacturerData(quint16 manufacturerId, const QByteArray &data)
{
    Q_D(QBluetoothDeviceInfo);
    auto it = d->manufacturerData.constFind(manufacturerId);
    while (it != d->manufacturerData.cend() && it.key() == manufacturerId) {
        if (*it == data)
            return false;
        it++;
    }

    d->manufacturerData.insert(manufacturerId, data);
    return true;
}

/*!
    Returns the complete set of all manufacturer data from advertisement packets.

    Some devices may provide multiple manufacturer data entries per manufacturer ID.
    An example might be a Bluetooth Low Energy device that sends a different manufacturer data via
    advertisement packets and scan response packets respectively. Therefore the returned hash table
    may have multiple entries per manufacturer ID or hash key.

    \sa setManufacturerData
    \since 5.12
*/
QMultiHash<quint16, QByteArray> QBluetoothDeviceInfo::manufacturerData() const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->manufacturerData;
}

/*!
    Returns all service data IDs from advertisement packets attached to this device information.

    \sa serviceData(), setServiceData()
    \since 6.3
 */
QList<QBluetoothUuid> QBluetoothDeviceInfo::serviceIds() const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->serviceData.keys().toList();
}

/*!
    Returns the data associated with the given \a serviceId.

    Service data is defined by
    the Supplement to the Bluetooth Core Specification and consists of two segments:

    \list
    \li Service UUID
    \li Sequence of arbitrary data octets
    \endlist

    \note The remote device may provide multiple data entries per \a serviceId.
    This function only returns the first entry. If all entries are needed use
    \l serviceData() which returns a multi hash.

    \sa serviceIds(), setServiceData()
    \since 6.3
 */
QByteArray QBluetoothDeviceInfo::serviceData(const QBluetoothUuid &serviceId) const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->serviceData.value(serviceId);
}

/*!
    Sets the advertised service \a data for the given \a serviceId.
    Returns \c true if it was inserted, \c false if it was already known.

    \sa serviceData
    \since 6.3
*/
bool QBluetoothDeviceInfo::setServiceData(const QBluetoothUuid &serviceId, const QByteArray &data)
{
    Q_D(QBluetoothDeviceInfo);
    auto it = d->serviceData.constFind(serviceId);
    while (it != d->serviceData.cend() && it.key() == serviceId) {
        if (*it == data)
            return false;
        it++;
    }

    d->serviceData.insert(serviceId, data);
    return true;
}

/*!
    Returns the complete set of all service data from advertisement packets.

    Some devices may provide multiple service data entries per service data ID.
    An example might be a Bluetooth Low Energy device that sends a different service data via
    advertisement packets and scan response packets respectively. Therefore the returned hash table
    may have multiple entries per service data ID or hash key.

    \sa setServiceData
    \since 6.3
*/
QMultiHash<QBluetoothUuid, QByteArray> QBluetoothDeviceInfo::serviceData() const
{
    Q_D(const QBluetoothDeviceInfo);
    return d->serviceData;
}

/*!
    Sets the CoreConfigurations of the device to \a coreConfigs. This will help to make a difference
    between regular and Low Energy devices.

    \sa coreConfigurations()
    \since 5.4
*/
void QBluetoothDeviceInfo::setCoreConfigurations(QBluetoothDeviceInfo::CoreConfigurations coreConfigs)
{
    Q_D(QBluetoothDeviceInfo);

    d->deviceCoreConfiguration = coreConfigs;
}

/*!

    Returns the configuration of the device. If device configuration is not set,
    basic rate device configuration will be returned.

    \sa setCoreConfigurations()
    \since 5.4
*/
QBluetoothDeviceInfo::CoreConfigurations QBluetoothDeviceInfo::coreConfigurations() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->deviceCoreConfiguration;
}

/*!
    Returns true if the QBluetoothDeviceInfo object is created from cached data.
*/
bool QBluetoothDeviceInfo::isCached() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->cached;
}

/*!
  Used by the system to set the \a cached flag if the QBluetoothDeviceInfo is created from cached data. Cached
  information may not be as accurate as data read from an active device.
  */
void QBluetoothDeviceInfo::setCached(bool cached)
{
    Q_D(QBluetoothDeviceInfo);

    d->cached = cached;
}

/*!
   Sets the unique identifier \a uuid for Bluetooth devices, that do not have addresses.
   This happens on \macos and iOS, where the CoreBluetooth API hides addresses, but provides
   UUIDs to identify devices/peripherals.

   This uuid is invalid on any other platform.

   \sa deviceUuid()
   \since 5.5
  */
void QBluetoothDeviceInfo::setDeviceUuid(const QBluetoothUuid &uuid)
{
    Q_D(QBluetoothDeviceInfo);

    d->deviceUuid = uuid;
}

/*!
   Returns a unique identifier for a Bluetooth device without an address.

   In general, this uuid is invalid on every platform but \macos and iOS.
   It is used as a workaround for those two platforms as they do not
   provide Bluetooth addresses for found Bluetooth Low Energy devices.
   Every other platform uses \l address() instead.

   \sa setDeviceUuid()
   \since 5.5
  */
QBluetoothUuid QBluetoothDeviceInfo::deviceUuid() const
{
    Q_D(const QBluetoothDeviceInfo);

    return d->deviceUuid;
}

QT_END_NAMESPACE
