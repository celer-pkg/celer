// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "android/devicediscoverybroadcastreceiver_p.h"
#include <QCoreApplication>
#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothUuid>
#include "android/jni_android_p.h"
#include <QtCore/QHash>
#include <QtCore/qbitarray.h>
#include <algorithm>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

typedef QHash<jint, QBluetoothDeviceInfo::CoreConfigurations> JCachedBtTypes;
Q_GLOBAL_STATIC(JCachedBtTypes, cachedBtTypes)
typedef QHash<jint, QBluetoothDeviceInfo::MajorDeviceClass> JCachedMajorTypes;
Q_GLOBAL_STATIC(JCachedMajorTypes, cachedMajorTypes)

typedef QHash<jint, quint8> JCachedMinorTypes;
Q_GLOBAL_STATIC(JCachedMinorTypes, cachedMinorTypes)

static QBitArray initializeMinorCaches()
{
    const qsizetype numberOfMajorDeviceClasses = 11; // count QBluetoothDeviceInfo::MajorDeviceClass values

    // switch below used to ensure that we notice additions to MajorDeviceClass enum
    const QBluetoothDeviceInfo::MajorDeviceClass classes = QBluetoothDeviceInfo::ComputerDevice;
    switch (classes) {
    case QBluetoothDeviceInfo::MiscellaneousDevice:
    case QBluetoothDeviceInfo::ComputerDevice:
    case QBluetoothDeviceInfo::PhoneDevice:
    case QBluetoothDeviceInfo::NetworkDevice:
    case QBluetoothDeviceInfo::AudioVideoDevice:
    case QBluetoothDeviceInfo::PeripheralDevice:
    case QBluetoothDeviceInfo::ImagingDevice:
    case QBluetoothDeviceInfo::WearableDevice:
    case QBluetoothDeviceInfo::ToyDevice:
    case QBluetoothDeviceInfo::HealthDevice:
    case QBluetoothDeviceInfo::UncategorizedDevice:
        break;
    default:
        qCWarning(QT_BT_ANDROID) << "Unknown category of major device class:" << classes;
    }

    return QBitArray(numberOfMajorDeviceClasses, false);
}

Q_GLOBAL_STATIC_WITH_ARGS(QBitArray, initializedCacheTracker, (initializeMinorCaches()))


// class names
static const char javaBluetoothDeviceClassName[] = "android/bluetooth/BluetoothDevice";
static const char javaBluetoothClassDeviceMajorClassName[] = "android/bluetooth/BluetoothClass$Device$Major";
static const char javaBluetoothClassDeviceClassName[] = "android/bluetooth/BluetoothClass$Device";

// field names device type (LE vs classic)
static const char javaDeviceTypeClassic[] = "DEVICE_TYPE_CLASSIC";
static const char javaDeviceTypeDual[] = "DEVICE_TYPE_DUAL";
static const char javaDeviceTypeLE[] = "DEVICE_TYPE_LE";
static const char javaDeviceTypeUnknown[] = "DEVICE_TYPE_UNKNOWN";

struct MajorClassJavaToQtMapping
{
    const char javaFieldName[14];
    QBluetoothDeviceInfo::MajorDeviceClass qtMajor : 16;
};
static_assert(sizeof(MajorClassJavaToQtMapping) == 16);

static constexpr MajorClassJavaToQtMapping majorMappings[] = {
    { "AUDIO_VIDEO", QBluetoothDeviceInfo::AudioVideoDevice },
    { "COMPUTER", QBluetoothDeviceInfo::ComputerDevice },
    { "HEALTH", QBluetoothDeviceInfo::HealthDevice },
    { "IMAGING", QBluetoothDeviceInfo::ImagingDevice },
    { "MISC", QBluetoothDeviceInfo::MiscellaneousDevice },
    { "NETWORKING", QBluetoothDeviceInfo::NetworkDevice },
    { "PERIPHERAL", QBluetoothDeviceInfo::PeripheralDevice },
    { "PHONE", QBluetoothDeviceInfo::PhoneDevice },
    { "TOY", QBluetoothDeviceInfo::ToyDevice },
    { "UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedDevice },
    { "WEARABLE", QBluetoothDeviceInfo::WearableDevice },
};

// QBluetoothDeviceInfo::MajorDeviceClass value plus 1 matches index
// UncategorizedDevice shifts to index 0
static constexpr quint8 minorIndexSizes[] = {
  64,  // QBluetoothDevice::UncategorizedDevice
  61,  // QBluetoothDevice::MiscellaneousDevice
  18,  // QBluetoothDevice::ComputerDevice
  35,  // QBluetoothDevice::PhoneDevice
  62,  // QBluetoothDevice::NetworkDevice
  0,  // QBluetoothDevice::AudioVideoDevice
  56,  // QBluetoothDevice::PeripheralDevice
  63,  // QBluetoothDevice::ImagingDEvice
  49,  // QBluetoothDevice::WearableDevice
  42,  // QBluetoothDevice::ToyDevice
  26,  // QBluetoothDevice::HealthDevice
};

struct MinorClassJavaToQtMapping
{
    char const * javaFieldName;
    quint8 qtMinor;
};

static const MinorClassJavaToQtMapping minorMappings[] = {
    // QBluetoothDevice::AudioVideoDevice -> 17 entries
    { "AUDIO_VIDEO_CAMCORDER", QBluetoothDeviceInfo::Camcorder }, //index 0
    { "AUDIO_VIDEO_CAR_AUDIO", QBluetoothDeviceInfo::CarAudio },
    { "AUDIO_VIDEO_HANDSFREE", QBluetoothDeviceInfo::HandsFreeDevice },
    { "AUDIO_VIDEO_HEADPHONES", QBluetoothDeviceInfo::Headphones },
    { "AUDIO_VIDEO_HIFI_AUDIO", QBluetoothDeviceInfo::HiFiAudioDevice },
    { "AUDIO_VIDEO_LOUDSPEAKER", QBluetoothDeviceInfo::Loudspeaker },
    { "AUDIO_VIDEO_MICROPHONE", QBluetoothDeviceInfo::Microphone },
    { "AUDIO_VIDEO_PORTABLE_AUDIO", QBluetoothDeviceInfo::PortableAudioDevice },
    { "AUDIO_VIDEO_SET_TOP_BOX", QBluetoothDeviceInfo::SetTopBox },
    { "AUDIO_VIDEO_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedAudioVideoDevice },
    { "AUDIO_VIDEO_VCR", QBluetoothDeviceInfo::Vcr },
    { "AUDIO_VIDEO_VIDEO_CAMERA", QBluetoothDeviceInfo::VideoCamera },
    { "AUDIO_VIDEO_VIDEO_CONFERENCING", QBluetoothDeviceInfo::VideoConferencing },
    { "AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER", QBluetoothDeviceInfo::VideoDisplayAndLoudspeaker },
    { "AUDIO_VIDEO_VIDEO_GAMING_TOY", QBluetoothDeviceInfo::GamingDevice },
    { "AUDIO_VIDEO_VIDEO_MONITOR", QBluetoothDeviceInfo::VideoMonitor },
    { "AUDIO_VIDEO_WEARABLE_HEADSET", QBluetoothDeviceInfo::WearableHeadsetDevice },
    { nullptr, 0 }, // separator

    // QBluetoothDevice::ComputerDevice -> 7 entries
    { "COMPUTER_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedComputer }, // index 18
    { "COMPUTER_DESKTOP", QBluetoothDeviceInfo::DesktopComputer },
    { "COMPUTER_HANDHELD_PC_PDA", QBluetoothDeviceInfo::HandheldComputer },
    { "COMPUTER_LAPTOP", QBluetoothDeviceInfo::LaptopComputer },
    { "COMPUTER_PALM_SIZE_PC_PDA", QBluetoothDeviceInfo::HandheldClamShellComputer },
    { "COMPUTER_SERVER", QBluetoothDeviceInfo::ServerComputer },
    { "COMPUTER_WEARABLE", QBluetoothDeviceInfo::WearableComputer },
    { nullptr, 0 },  // separator

    // QBluetoothDevice::HealthDevice -> 8 entries
    { "HEALTH_BLOOD_PRESSURE", QBluetoothDeviceInfo::HealthBloodPressureMonitor }, // index 26
    { "HEALTH_DATA_DISPLAY", QBluetoothDeviceInfo::HealthDataDisplay },
    { "HEALTH_GLUCOSE", QBluetoothDeviceInfo::HealthGlucoseMeter },
    { "HEALTH_PULSE_OXIMETER", QBluetoothDeviceInfo::HealthPulseOximeter },
    { "HEALTH_PULSE_RATE", QBluetoothDeviceInfo::HealthStepCounter },
    { "HEALTH_THERMOMETER", QBluetoothDeviceInfo::HealthThermometer },
    { "HEALTH_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedHealthDevice },
    { "HEALTH_WEIGHING", QBluetoothDeviceInfo::HealthWeightScale },
    { nullptr, 0 }, // separator

    // QBluetoothDevice::PhoneDevice -> 6 entries
    { "PHONE_CELLULAR", QBluetoothDeviceInfo::CellularPhone }, // index 35
    { "PHONE_CORDLESS", QBluetoothDeviceInfo::CordlessPhone },
    { "PHONE_ISDN", QBluetoothDeviceInfo::CommonIsdnAccessPhone },
    { "PHONE_MODEM_OR_GATEWAY", QBluetoothDeviceInfo::WiredModemOrVoiceGatewayPhone },
    { "PHONE_SMART", QBluetoothDeviceInfo::SmartPhone },
    { "PHONE_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedPhone },
    { nullptr, 0 }, // separator

    // QBluetoothDevice::ToyDevice -> 6 entries
    { "TOY_CONTROLLER", QBluetoothDeviceInfo::ToyController }, // index 42
    { "TOY_DOLL_ACTION_FIGURE", QBluetoothDeviceInfo::ToyDoll },
    { "TOY_GAME", QBluetoothDeviceInfo::ToyGame },
    { "TOY_ROBOT", QBluetoothDeviceInfo::ToyRobot },
    { "TOY_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedToy },
    { "TOY_VEHICLE", QBluetoothDeviceInfo::ToyVehicle },
    { nullptr, 0 }, // separator

    // QBluetoothDevice::WearableDevice -> 6 entries
    { "WEARABLE_GLASSES", QBluetoothDeviceInfo::WearableGlasses }, // index 49
    { "WEARABLE_HELMET", QBluetoothDeviceInfo::WearableHelmet },
    { "WEARABLE_JACKET", QBluetoothDeviceInfo::WearableJacket },
    { "WEARABLE_PAGER", QBluetoothDeviceInfo::WearablePager },
    { "WEARABLE_UNCATEGORIZED", QBluetoothDeviceInfo::UncategorizedWearableDevice },
    { "WEARABLE_WRIST_WATCH", QBluetoothDeviceInfo::WearableWristWatch },
    { nullptr, 0 }, // separator

    // QBluetoothDevice::PeripheralDevice -> 3 entries
    // For some reason these are not mentioned in Android docs but still exist
    { "PERIPHERAL_NON_KEYBOARD_NON_POINTING", QBluetoothDeviceInfo::UncategorizedPeripheral }, // index 56
    { "PERIPHERAL_KEYBOARD", QBluetoothDeviceInfo::KeyboardPeripheral },
    { "PERIPHERAL_POINTING", QBluetoothDeviceInfo::PointingDevicePeripheral },
    { "PERIPHERAL_KEYBOARD_POINTING", QBluetoothDeviceInfo::KeyboardWithPointingDevicePeripheral },
    { nullptr, 0 }, // separator

    // the following entries do not exist on Android
    // we map them to the unknown minor version case
    // QBluetoothDevice::Miscellaneous
    { nullptr, 0 }, // index 61 & separator

    // QBluetoothDevice::NetworkDevice
    { nullptr, 0 }, // index 62 & separator

    // QBluetoothDevice::ImagingDevice
    { nullptr, 0 }, // index 63 & separator

    // QBluetoothDevice::UncategorizedDevice
    { nullptr, 0 }, // index 64 & separator
};

/*
    Advertising Data Type (AD type) for LE scan records, as defined in
    https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
*/
enum ADType {
    ADType16BitUuidIncomplete = 0x02,
    ADType16BitUuidComplete = 0x03,
    ADType32BitUuidIncomplete = 0x04,
    ADType32BitUuidComplete = 0x05,
    ADType128BitUuidIncomplete = 0x06,
    ADType128BitUuidComplete = 0x07,
    ADTypeShortenedLocalName = 0x08,
    ADTypeCompleteLocalName = 0x09,
    ADTypeServiceData16Bit = 0x16,
    ADTypeServiceData32Bit = 0x20,
    ADTypeServiceData128Bit = 0x21,
    ADTypeManufacturerSpecificData = 0xff,
    // .. more will be added when required
};

QBluetoothDeviceInfo::CoreConfigurations qtBtTypeForJavaBtType(jint javaType)
{
    const JCachedBtTypes::iterator it = cachedBtTypes()->find(javaType);
    if (it == cachedBtTypes()->end()) {

        if (javaType == QJniObject::getStaticField<jint>(
                            javaBluetoothDeviceClassName, javaDeviceTypeClassic)) {
            cachedBtTypes()->insert(javaType,
                                    QBluetoothDeviceInfo::BaseRateCoreConfiguration);
            return QBluetoothDeviceInfo::BaseRateCoreConfiguration;
        } else if (javaType == QJniObject::getStaticField<jint>(
                        javaBluetoothDeviceClassName, javaDeviceTypeLE)) {
            cachedBtTypes()->insert(javaType,
                                    QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
            return QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
        } else if (javaType == QJniObject::getStaticField<jint>(
                            javaBluetoothDeviceClassName, javaDeviceTypeDual)) {
            cachedBtTypes()->insert(javaType,
                                    QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
            return QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;
        } else if (javaType == QJniObject::getStaticField<jint>(
                                javaBluetoothDeviceClassName, javaDeviceTypeUnknown)) {
            cachedBtTypes()->insert(javaType,
                                QBluetoothDeviceInfo::UnknownCoreConfiguration);
        } else {
            qCWarning(QT_BT_ANDROID) << "Unknown Bluetooth device type value";
        }

        return QBluetoothDeviceInfo::UnknownCoreConfiguration;
    } else {
        return it.value();
    }
}

QBluetoothDeviceInfo::MajorDeviceClass resolveAndroidMajorClass(jint javaType)
{
    const JCachedMajorTypes::iterator it = cachedMajorTypes()->find(javaType);
    if (it == cachedMajorTypes()->end()) {
        QJniEnvironment env;
        // precache all major device class fields
        jint fieldValue;
        QBluetoothDeviceInfo::MajorDeviceClass result = QBluetoothDeviceInfo::UncategorizedDevice;
        auto clazz = env->FindClass(javaBluetoothClassDeviceMajorClassName);
        for (const auto &majorMapping : majorMappings) {
            auto fieldId = env->GetStaticFieldID(clazz, majorMapping.javaFieldName, "I");
            if (!env->ExceptionCheck())
                fieldValue = env->GetStaticIntField(clazz, fieldId);
            if (env.checkAndClearExceptions()) {
                qCWarning(QT_BT_ANDROID) << "Unknown BluetoothClass.Device.Major field" << javaType;

                // add fallback value because field not readable
                cachedMajorTypes()->insert(javaType, QBluetoothDeviceInfo::UncategorizedDevice);
            } else {
                cachedMajorTypes()->insert(fieldValue, majorMapping.qtMajor);
            }

            if (fieldValue == javaType)
                result = majorMapping.qtMajor;
        }

        return result;
    } else {
        return it.value();
    }
}

/*
    The index for major into the MinorClassJavaToQtMapping and initializedCacheTracker
    is major+1 except for UncategorizedDevice which is at index 0.
*/
int mappingIndexForMajor(QBluetoothDeviceInfo::MajorDeviceClass major)
{
    int mappingIndex = (int) major;
    if (major == QBluetoothDeviceInfo::UncategorizedDevice)
        mappingIndex = 0;
    else
        mappingIndex++;

    Q_ASSERT(mappingIndex >=0
             && mappingIndex <= (QBluetoothDeviceInfo::HealthDevice + 1));

    return mappingIndex;
}

void triggerCachingOfMinorsForMajor(QBluetoothDeviceInfo::MajorDeviceClass major)
{
    //qCDebug(QT_BT_ANDROID) << "Caching minor values for major" << major;
    int mappingIndex = mappingIndexForMajor(major);
    quint8 sizeIndex = minorIndexSizes[mappingIndex];

    while (minorMappings[sizeIndex].javaFieldName != nullptr) {
        jint fieldValue = QJniObject::getStaticField<jint>(
                    javaBluetoothClassDeviceClassName, minorMappings[sizeIndex].javaFieldName);

        Q_ASSERT(fieldValue >= 0);
        cachedMinorTypes()->insert(fieldValue, minorMappings[sizeIndex].qtMinor);
        sizeIndex++;
    }

    initializedCacheTracker()->setBit(mappingIndex);
}

quint8 resolveAndroidMinorClass(QBluetoothDeviceInfo::MajorDeviceClass major, jint javaMinor)
{
    // there are no minor device classes in java with value 0
    //qCDebug(QT_BT_ANDROID) << "received minor class device:" << javaMinor;
    if (javaMinor == 0)
        return 0;

    int mappingIndex = mappingIndexForMajor(major);

    // whenever we encounter a not yet seen major device class
    // we populate the cache with all its related minor values
    if (!initializedCacheTracker()->at(mappingIndex))
        triggerCachingOfMinorsForMajor(major);

    const JCachedMinorTypes::iterator it = cachedMinorTypes()->find(javaMinor);
    if (it == cachedMinorTypes()->end())
        return 0;
    else
        return it.value();
}


DeviceDiscoveryBroadcastReceiver::DeviceDiscoveryBroadcastReceiver(QObject* parent): AndroidBroadcastReceiver(parent)
{
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionFound>()));
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothAdapter, JavaNames::ActionDiscoveryStarted>()));
    addAction(QJniObject::fromString(
        valueForStaticField<QtJniTypes::BluetoothAdapter, JavaNames::ActionDiscoveryFinished>()));
}

// Runs in Java thread
void DeviceDiscoveryBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context)
    Q_UNUSED(env)

    QJniObject intentObject(intent);
    const QString action = intentObject.callMethod<jstring>("getAction").toString();

    qCDebug(QT_BT_ANDROID) << "DeviceDiscoveryBroadcastReceiver::onReceive() - event:" << action;

    if (action == valueForStaticField<QtJniTypes::BluetoothAdapter,
                                      JavaNames::ActionDiscoveryFinished>()) {
        emit finished();
    } else if (action == valueForStaticField<QtJniTypes::BluetoothAdapter,
                                             JavaNames::ActionDiscoveryStarted>()) {
        emit discoveryStarted();
    } else if (action == valueForStaticField<QtJniTypes::BluetoothDevice,
                                             JavaNames::ActionFound>()) {
        //get BluetoothDevice
        QJniObject keyExtra =
            QJniObject::fromString(valueForStaticField<QtJniTypes::BluetoothDevice,
                                                       JavaNames::ExtraDevice>());
        const QJniObject bluetoothDevice =
                intentObject.callMethod<QtJniTypes::Parcelable>("getParcelableExtra",
                                                                keyExtra.object<jstring>());

        if (!bluetoothDevice.isValid())
            return;

        keyExtra = QJniObject::fromString(valueForStaticField<QtJniTypes::BluetoothDevice,
                                                              JavaNames::ExtraRssi>());
        int rssi = intentObject.callMethod<jshort>("getShortExtra",
                                                   keyExtra.object<jstring>(), jshort(0));

        const QBluetoothDeviceInfo info = retrieveDeviceInfo(bluetoothDevice, rssi);
        if (info.isValid())
            emit deviceDiscovered(info, false);
    }
}

// Runs in Java thread
void DeviceDiscoveryBroadcastReceiver::onReceiveLeScan(
        JNIEnv */*env*/, jobject jBluetoothDevice, jint rssi, jbyteArray scanRecord)
{
    const QJniObject bluetoothDevice(jBluetoothDevice);
    if (!bluetoothDevice.isValid())
        return;

    const QBluetoothDeviceInfo info = retrieveDeviceInfo(bluetoothDevice, rssi, scanRecord);
    if (info.isValid())
        emit deviceDiscovered(info, true);
}

QBluetoothDeviceInfo DeviceDiscoveryBroadcastReceiver::retrieveDeviceInfo(const QJniObject &bluetoothDevice, int rssi, jbyteArray scanRecord)
{
    const QString deviceName = bluetoothDevice.callMethod<jstring>("getName").toString();
    const QBluetoothAddress deviceAddress(
                bluetoothDevice.callMethod<jstring>("getAddress").toString());
    const QJniObject bluetoothClass =
            bluetoothDevice.callMethod<QtJniTypes::BluetoothClass>("getBluetoothClass");

    if (!bluetoothClass.isValid())
        return QBluetoothDeviceInfo();

    QBluetoothDeviceInfo::MajorDeviceClass majorClass = resolveAndroidMajorClass(
                                bluetoothClass.callMethod<jint>("getMajorDeviceClass"));
    // major device class is 5 bits from index 8 - 12
    quint32 classType = ((quint32(majorClass) & 0x1f) << 8);

    jint javaMinor = bluetoothClass.callMethod<jint>("getDeviceClass");
    quint8 minorDeviceType = resolveAndroidMinorClass(majorClass, javaMinor);

    // minor device class is 6 bits from index 2 - 7
    classType |= ((quint32(minorDeviceType) & 0x3f) << 2);

    static constexpr quint32 services[] = {
        QBluetoothDeviceInfo::PositioningService,
        QBluetoothDeviceInfo::NetworkingService,
        QBluetoothDeviceInfo::RenderingService,
        QBluetoothDeviceInfo::CapturingService,
        QBluetoothDeviceInfo::ObjectTransferService,
        QBluetoothDeviceInfo::AudioService,
        QBluetoothDeviceInfo::TelephonyService,
        QBluetoothDeviceInfo::InformationService,
    };

    // Matching BluetoothClass.Service values
    quint32 serviceResult = 0;
    for (quint32 current : services) {
        int androidId = (current << 16); // Android values shift by 2 bytes compared to Qt enums
        if (bluetoothClass.callMethod<jboolean>("hasService", androidId))
            serviceResult |= current;
    }

    // service class info is 11 bits from index 13 - 23
    classType |= (serviceResult << 13);

    QBluetoothDeviceInfo info(deviceAddress, deviceName, classType);
    info.setRssi(rssi);
    QJniEnvironment env;
    if (scanRecord != nullptr) {
        // Parse scan record
        jboolean isCopy;
        jbyte *elems = env->GetByteArrayElements(scanRecord, &isCopy);
        const char *scanRecordBuffer = reinterpret_cast<const char *>(elems);
        const jsize scanRecordLength = env->GetArrayLength(scanRecord);

        QList<QBluetoothUuid> serviceUuids;
        jsize i = 0;

        // Spec 4.2, Vol 3, Part C, Chapter 11
        QString localName;
        while (i < scanRecordLength) {
            // sizeof(EIR Data) = sizeof(Length) + sizeof(EIR data Type) + sizeof(EIR Data)
            // Length = sizeof(EIR data Type) + sizeof(EIR Data)

            const int nBytes = scanRecordBuffer[i];
            if (nBytes == 0)
                break;

            if (i >= scanRecordLength - nBytes)
                break;

            const int adType = scanRecordBuffer[i+1];
            const char *dataPtr = &scanRecordBuffer[i+2];
            QBluetoothUuid foundService;

            switch (adType) {
            case ADType16BitUuidIncomplete:
            case ADType16BitUuidComplete:
                foundService = QBluetoothUuid(qFromLittleEndian<quint16>(dataPtr));
                break;
            case ADType32BitUuidIncomplete:
            case ADType32BitUuidComplete:
                foundService = QBluetoothUuid(qFromLittleEndian<quint32>(dataPtr));
                break;
            case ADType128BitUuidIncomplete:
            case ADType128BitUuidComplete:
                foundService =
                    QBluetoothUuid(qToBigEndian<QUuid::Id128Bytes>(qFromLittleEndian<QUuid::Id128Bytes>(dataPtr)));
                break;
            case ADTypeServiceData16Bit:
                if (nBytes >= 3) {
                    info.setServiceData(QBluetoothUuid(qFromLittleEndian<quint16>(dataPtr)),
                                        QByteArray(dataPtr + 2, nBytes - 3));
                }
                break;
            case ADTypeServiceData32Bit:
                if (nBytes >= 5) {
                    info.setServiceData(QBluetoothUuid(qFromLittleEndian<quint32>(dataPtr)),
                                        QByteArray(dataPtr + 4, nBytes - 5));
                }
                break;
            case ADTypeServiceData128Bit:
                if (nBytes >= 17) {
                    info.setServiceData(QBluetoothUuid(qToBigEndian<QUuid::Id128Bytes>(
                                                qFromLittleEndian<QUuid::Id128Bytes>(dataPtr))),
                                        QByteArray(dataPtr + 16, nBytes - 17));
                }
                break;
            case ADTypeManufacturerSpecificData:
                if (nBytes >= 3) {
                    info.setManufacturerData(qFromLittleEndian<quint16>(dataPtr),
                                              QByteArray(dataPtr + 2, nBytes - 3));
                }
                break;
            // According to Spec 5.0, Vol 3, Part C, Chapter 12.1
            // the device's local  name is utf8 encoded
            case ADTypeShortenedLocalName:
                if (localName.isEmpty())
                    localName = QString::fromUtf8(dataPtr, nBytes - 1);
                break;
            case ADTypeCompleteLocalName:
                localName = QString::fromUtf8(dataPtr, nBytes - 1);
                break;
            default:
                // qWarning() << "Unhandled AD Type" << Qt::hex << adType;
                // no other types supported yet and therefore skipped
                // https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
                break;
            }

            i += nBytes + 1;

            if (!foundService.isNull() && !serviceUuids.contains(foundService))
                serviceUuids.append(foundService);
        }

        if (info.name().isEmpty())
            info.setName(localName);

        info.setServiceUuids(serviceUuids);

        env->ReleaseByteArrayElements(scanRecord, elems, JNI_ABORT);
    }

    auto methodId = env.findMethod(bluetoothDevice.objectClass(), "getType", "()I");
    jint javaBtType = env->CallIntMethod(bluetoothDevice.object(), methodId);
    if (!env.checkAndClearExceptions()) {
        info.setCoreConfigurations(qtBtTypeForJavaBtType(javaBtType));
    }

    return info;
}

QT_END_NAMESPACE
