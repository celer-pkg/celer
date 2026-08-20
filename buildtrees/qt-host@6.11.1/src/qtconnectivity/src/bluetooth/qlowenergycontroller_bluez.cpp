// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2016 Javier S. Pedro <maemo@javispedro.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "lecmaccalculator_p.h"
#include "qlowenergycontroller_bluez_p.h"
#include "qbluetoothsocketbase_p.h"
#include "qbluetoothsocket_bluez_p.h"
#include "qleadvertiser_bluez_p.h"
#include "bluez/bluez_data_p.h"
#include "bluez/hcimanager_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/remotedevicemanager_p.h"
#include "bluez/bluez5_helper_p.h"
#include "bluez/bluetoothmanagement_p.h"

#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyServiceData>

#include <algorithm>
#include <climits>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr quint16 ATT_DEFAULT_LE_MTU = 23;
constexpr quint16 ATT_MAX_LE_MTU = 0x200;

#define GATT_PRIMARY_SERVICE    quint16(0x2800)
#define GATT_SECONDARY_SERVICE  quint16(0x2801)
#define GATT_INCLUDED_SERVICE   quint16(0x2802)
#define GATT_CHARACTERISTIC     quint16(0x2803)

//GATT command sizes in bytes
#define ERROR_RESPONSE_HEADER_SIZE 5
#define FIND_INFO_REQUEST_HEADER_SIZE 5
#define GRP_TYPE_REQ_HEADER_SIZE 7
#define READ_BY_TYPE_REQ_HEADER_SIZE 7
#define READ_REQUEST_HEADER_SIZE 3
#define READ_BLOB_REQUEST_HEADER_SIZE 5
#define WRITE_REQUEST_HEADER_SIZE 3    // same size for WRITE_COMMAND header
#define PREPARE_WRITE_HEADER_SIZE 5
#define EXECUTE_WRITE_HEADER_SIZE 2
#define MTU_EXCHANGE_HEADER_SIZE 3

#define APPEND_VALUE true
#define NEW_VALUE false

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QBluetooth;
using namespace QtBluetoothPrivate; // for D-Bus wrappers

const int maxPrepareQueueSize = 1024;

/* returns false if the format is incorrect */
static bool dumpErrorInformation(const QByteArray &response)
{
    const char *data = response.constData();
    if (response.size() != 5
        || (static_cast<QBluezConst::AttCommand>(data[0])
            != QBluezConst::AttCommand::ATT_OP_ERROR_RESPONSE)) {
        qCWarning(QT_BT_BLUEZ) << QLatin1String("Not a valid error response");
        return false;
    }

    QBluezConst::AttCommand lastCommand = static_cast<QBluezConst::AttCommand>(data[1]);
    quint16 handle = bt_get_le16(&data[2]);
    QBluezConst::AttError errorCode = static_cast<QBluezConst::AttError>(data[4]);

    QString errorString;
    switch (errorCode) {
    case QBluezConst::AttError::ATT_ERROR_INVALID_HANDLE:
        errorString = QStringLiteral("invalid handle"); break;
    case QBluezConst::AttError::ATT_ERROR_READ_NOT_PERM:
        errorString = QStringLiteral("not readable attribute - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_WRITE_NOT_PERM:
        errorString = QStringLiteral("not writable attribute - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_INVALID_PDU:
        errorString = QStringLiteral("PDU invalid"); break;
    case QBluezConst::AttError::ATT_ERROR_INSUF_AUTHENTICATION:
        errorString = QStringLiteral("needs authentication - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_REQUEST_NOT_SUPPORTED:
        errorString = QStringLiteral("server does not support request"); break;
    case QBluezConst::AttError::ATT_ERROR_INVALID_OFFSET:
        errorString = QStringLiteral("offset past end of attribute"); break;
    case QBluezConst::AttError::ATT_ERROR_INSUF_AUTHORIZATION:
        errorString = QStringLiteral("need authorization - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_PREPARE_QUEUE_FULL:
        errorString = QStringLiteral("run out of prepare queue space"); break;
    case QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_FOUND:
        errorString = QStringLiteral("no attribute in given range found"); break;
    case QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_LONG:
        errorString = QStringLiteral("attribute not read/written using read blob"); break;
    case QBluezConst::AttError::ATT_ERROR_INSUF_ENCR_KEY_SIZE:
        errorString = QStringLiteral("need encryption key size - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_INVAL_ATTR_VALUE_LEN:
        errorString = QStringLiteral("written value is invalid size"); break;
    case QBluezConst::AttError::ATT_ERROR_UNLIKELY:
        errorString = QStringLiteral("unlikely error"); break;
    case QBluezConst::AttError::ATT_ERROR_INSUF_ENCRYPTION:
        errorString = QStringLiteral("needs encryption - permissions"); break;
    case QBluezConst::AttError::ATT_ERROR_UNSUPPRTED_GROUP_TYPE:
        errorString = QStringLiteral("unsupported group type"); break;
    case QBluezConst::AttError::ATT_ERROR_INSUF_RESOURCES:
        errorString = QStringLiteral("insufficient resources to complete request"); break;
    default:
        if (errorCode >= QBluezConst::AttError::ATT_ERROR_APPLICATION_START
            && errorCode <= QBluezConst::AttError::ATT_ERROR_APPLICATION_END)
            errorString =
                    QStringLiteral("application error: %1").arg(static_cast<quint8>(errorCode));
        else
            errorString = QStringLiteral("unknown error code");
        break;
    }

    qCDebug(QT_BT_BLUEZ) << "Error:" << errorCode << "Error description:" << errorString
                         << "last command:" << lastCommand << "handle:" << handle;

    return true;
}

static int getUuidSize(const QBluetoothUuid &uuid)
{
    return uuid.minimumSize() == 2 ? 2 : 16;
}

template<typename T> static void putDataAndIncrement(const T &src, char *&dst)
{
    putBtData(src, dst);
    dst += sizeof(T);
}
template<> void putDataAndIncrement(const QBluetoothUuid &uuid, char *&dst)
{
    bool ok;
    quint16 uuid16 = uuid.toUInt16(&ok);
    if (ok) {
        putBtData(uuid16, dst);
        dst += sizeof(uuid16);
    } else {
        QUuid::Id128Bytes btOrder = uuid.toBytes(QSysInfo::LittleEndian);
        memcpy(dst, btOrder.data, sizeof(btOrder));
        dst += sizeof(btOrder);
    }
}
template<> void putDataAndIncrement(const QByteArray &value, char *&dst)
{
    using namespace std;
    memcpy(dst, value.constData(), value.size());
    dst += value.size();
}

/*!
    \class QLowEnergyControllerPrivateBluez
    \inmodule QtBluetooth
    \internal
*/
QLowEnergyControllerPrivateBluez::QLowEnergyControllerPrivateBluez()
    : QLowEnergyControllerPrivate(),
      requestPending(false),
      mtuSize(ATT_DEFAULT_LE_MTU),
      securityLevelValue(-1),
      encryptionChangePending(false)
{
    registerQLowEnergyControllerMetaType();
    qRegisterMetaType<QList<QLowEnergyHandle> >();
}

void QLowEnergyControllerPrivateBluez::init()
{
    // The HCI manager is shared between this class and the advertiser
    hciManager = std::make_shared<HciManager>(localAdapter);

    if (!hciManager->isValid()){
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        return;
    }

    hciManager->monitorEvent(HciManager::HciEvent::EVT_ENCRYPT_CHANGE);
    connect(hciManager.get(), SIGNAL(encryptionChangedEvent(QBluetoothAddress,bool)),
            this, SLOT(encryptionChangedEvent(QBluetoothAddress,bool)));
    hciManager->monitorEvent(HciManager::HciEvent::EVT_LE_META_EVENT);
    hciManager->monitorAclPackets();
    connect(hciManager.get(), &HciManager::connectionComplete, this, [this](quint16 handle) {
        connectionHandle = handle;
        qCDebug(QT_BT_BLUEZ) << "received connection complete event, handle:" << handle;
    });
    connect(hciManager.get(), &HciManager::connectionUpdate, this,
            [this](quint16 handle, const QLowEnergyConnectionParameters &params) {
                if (handle == connectionHandle)
                    emit q_ptr->connectionUpdated(params);
            }
    );
    connect(hciManager.get(), &HciManager::signatureResolvingKeyReceived, this,
            [this](quint16 handle, bool remoteKey, const QUuid::Id128Bytes &csrk) {
                if (handle != connectionHandle)
                    return;
                if ((remoteKey && role == QLowEnergyController::CentralRole)
                        || (!remoteKey && role == QLowEnergyController::PeripheralRole)) {
                    return;
                }
                qCDebug(QT_BT_BLUEZ) << "received new signature resolving key";
                signingData.insert(remoteDevice.toUInt64(), SigningData(csrk));
        }
    );

    if (role == QLowEnergyController::CentralRole) {
        if (Q_UNLIKELY(!qEnvironmentVariableIsEmpty("BLUETOOTH_GATT_TIMEOUT"))) {
            bool ok = false;
            int value = qEnvironmentVariableIntValue("BLUETOOTH_GATT_TIMEOUT", &ok);
            if (ok)
                gattRequestTimeout = value;
        }

        // permit disabling of timeout behavior via environment variable
        if (gattRequestTimeout > 0) {
            qCWarning(QT_BT_BLUEZ) << "Enabling GATT request timeout behavior" << gattRequestTimeout;
            requestTimer = new QTimer(this);
            requestTimer->setSingleShot(true);
            requestTimer->setInterval(gattRequestTimeout);
            connect(requestTimer, &QTimer::timeout,
                    this, &QLowEnergyControllerPrivateBluez::handleGattRequestTimeout);
        }
    }
}

void QLowEnergyControllerPrivateBluez::handleGattRequestTimeout()
{
    // antyhing open that might require cancellation or a warning?
    if (encryptionChangePending) {
        // We cannot really recover for now but the warning is essential for debugging
        qCWarning(QT_BT_BLUEZ) << "****** Encryption change event blocking further GATT requests";
        return;
    }

    if (!openRequests.isEmpty() && requestPending) {
        const Request currentRequest = openRequests.dequeue();
        requestPending = false; // reset pending flag

        qCWarning(QT_BT_BLUEZ).nospace() << "****** Request type 0x" << currentRequest.command
                                         << " to server/peripheral timed out";
        qCWarning(QT_BT_BLUEZ) << "****** Looks like the characteristic or descriptor does NOT act in"
                               <<  "accordance to Bluetooth 4.x spec.";
        qCWarning(QT_BT_BLUEZ) << "****** Please check server implementation."
                               << "Continuing under reservation.";

        QBluezConst::AttCommand command = currentRequest.command;
        const auto createRequestErrorMessage = [](QBluezConst::AttCommand opcodeWithError,
                                                  QLowEnergyHandle handle) {
            QByteArray errorPackage(ERROR_RESPONSE_HEADER_SIZE, Qt::Uninitialized);
            errorPackage[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_ERROR_RESPONSE);
            errorPackage[1] = static_cast<quint8>(
                    opcodeWithError); // e.g. QBluezConst::AttCommand::ATT_OP_READ_REQUEST
            putBtData(handle, errorPackage.data() + 2); //
            errorPackage[4] = static_cast<quint8>(QBluezConst::AttError::ATT_ERROR_REQUEST_STALLED);

            return errorPackage;
        };

        switch (command) {
        case QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST: // MTU change request
            // never received reply to MTU request
            // it is safe to skip and go to next request
            break;
        case QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST: // primary or secondary service
                                                                    // discovery
        case QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST: // characteristic or included
                                                                   // service discovery
            // jump back into usual response handling with custom error code
            // 2nd param "0" as required by spec
            processReply(currentRequest, createRequestErrorMessage(command, 0));
            break;
        case QBluezConst::AttCommand::ATT_OP_READ_REQUEST: // read descriptor or characteristic
                                                           // value
        case QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST: // read long descriptor or
                                                                // characteristic
        case QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST: // write descriptor or characteristic
        {
            uint handleData = currentRequest.reference.toUInt();
            const QLowEnergyHandle charHandle = (handleData & 0xffff);
            const QLowEnergyHandle descriptorHandle = ((handleData >> 16) & 0xffff);
            processReply(currentRequest, createRequestErrorMessage(command,
                                descriptorHandle ? descriptorHandle : charHandle));
        } break;
        case QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST: // get descriptor information
            processReply(currentRequest, createRequestErrorMessage(
                                            command, currentRequest.reference2.toUInt()));
            break;
        case QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST: // prepare to write long desc or
                                                                    // char
        case QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST: // execute long write of desc or
                                                                    // char
        {
            uint handleData = currentRequest.reference.toUInt();
            const QLowEnergyHandle attrHandle = (handleData & 0xffff);
            processReply(currentRequest,
                         createRequestErrorMessage(command, attrHandle));
        } break;
        default:
            // not a command used by central role implementation
            qCWarning(QT_BT_BLUEZ) << "Missing response for ATT peripheral command: "
                                   << Qt::hex << command;
            break;
        }

        // spin openRequest queue further
        sendNextPendingRequest();
    }
}

QLowEnergyControllerPrivateBluez::~QLowEnergyControllerPrivateBluez()
{
    closeServerSocket();
    delete cmacCalculator;
    cmacCalculator = nullptr;
}

class ServerSocket
{
public:
    bool listen(const QBluetoothAddress &localAdapter)
    {
        m_socket = ::socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
        if (m_socket == -1) {
            qCWarning(QT_BT_BLUEZ) << "socket creation failed:" << qt_error_string(errno);
            return false;
        }
        sockaddr_l2 addr;

        // memset should be in std namespace for C++ compilers, but we also need to support
        // broken ones that put it in the global one.
        using namespace std;
        memset(&addr, 0, sizeof addr);

        addr.l2_family = AF_BLUETOOTH;
        addr.l2_cid = htobs(ATTRIBUTE_CHANNEL_ID);
        addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
        convertAddress(localAdapter.toUInt64(), addr.l2_bdaddr.b);
        if (::bind(m_socket, reinterpret_cast<sockaddr *>(&addr), sizeof addr) == -1) {
            qCWarning(QT_BT_BLUEZ) << "bind() failed:" << qt_error_string(errno);
            return false;
        }
        if (::listen(m_socket, 1)) {
            qCWarning(QT_BT_BLUEZ) << "listen() failed:" << qt_error_string(errno);
            return false;
        }
        return true;
    }

    ~ServerSocket()
    {
        if (m_socket != -1)
            close(m_socket);
    }

    int takeSocket()
    {
        const int socket = m_socket;
        m_socket = -1;
        return socket;
    }

private:
    int m_socket = -1;
};


void QLowEnergyControllerPrivateBluez::startAdvertising(const QLowEnergyAdvertisingParameters &params,
        const QLowEnergyAdvertisingData &advertisingData,
        const QLowEnergyAdvertisingData &scanResponseData)
{
    qCDebug(QT_BT_BLUEZ) << "Starting to advertise";
    if (!advertiser) {
        advertiser = new QLeAdvertiserBluez(params, advertisingData, scanResponseData, hciManager,
                                            this);
        connect(advertiser, &QLeAdvertiser::errorOccurred, this,
                &QLowEnergyControllerPrivateBluez::handleAdvertisingError);
    }
    setState(QLowEnergyController::AdvertisingState);
    advertiser->startAdvertising();
    if (params.mode() == QLowEnergyAdvertisingParameters::AdvNonConnInd
            || params.mode() == QLowEnergyAdvertisingParameters::AdvScanInd) {
        qCDebug(QT_BT_BLUEZ) << "Non-connectable advertising requested, "
                                "not listening for connections.";
        return;
    }

    ServerSocket serverSocket;
    if (!serverSocket.listen(localAdapter)) {
        setError(QLowEnergyController::AdvertisingError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    const int socketFd = serverSocket.takeSocket();
    serverSocketNotifier = new QSocketNotifier(socketFd, QSocketNotifier::Read, this);
    connect(serverSocketNotifier, &QSocketNotifier::activated, this,
            &QLowEnergyControllerPrivateBluez::handleConnectionRequest);
}

void QLowEnergyControllerPrivateBluez::stopAdvertising()
{
    setState(QLowEnergyController::UnconnectedState);
    advertiser->stopAdvertising();
}

void QLowEnergyControllerPrivateBluez::requestConnectionUpdate(const QLowEnergyConnectionParameters &params)
{
    // The spec says that the connection update command can be used by both slave and master
    // devices, but BlueZ allows it only for master devices. So for slave devices, we have to use a
    // connection parameter update request, which we need to wrap in an ACL command, as BlueZ
    // does not allow user-space sockets for the signaling channel.
    if (role == QLowEnergyController::CentralRole)
        hciManager->sendConnectionUpdateCommand(connectionHandle, params);
    else
        hciManager->sendConnectionParameterUpdateRequest(connectionHandle, params);
}

void QLowEnergyControllerPrivateBluez::connectToDevice()
{
    if (remoteDevice.isNull()) {
        qCWarning(QT_BT_BLUEZ) << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);
    if (l2cpSocket) {
        delete l2cpSocket;
        l2cpSocket = nullptr;
    }

    createServicesForCentralIfRequired();

    // check for active running connections
    // BlueZ 5.37+ (maybe even earlier versions) can have pending BTLE connections
    // Only one active L2CP socket to CID 0x4 possible at a time

    const QList<quint16> activeHandles = hciManager->activeLowEnergyConnections();
    if (!activeHandles.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot connect due to pending active LE connections";

        if (!device1Manager) {
            device1Manager = new RemoteDeviceManager(localAdapter, this);
            connect(device1Manager, &RemoteDeviceManager::finished,
                    this, &QLowEnergyControllerPrivateBluez::activeConnectionTerminationDone);
        }

        QList<QBluetoothAddress> connectedAddresses;
        for (const auto handle: activeHandles) {
            const QBluetoothAddress addr = hciManager->addressForConnectionHandle(handle);
            if (!addr.isNull())
                connectedAddresses.push_back(addr);
        }
        device1Manager->scheduleJob(RemoteDeviceManager::JobType::JobDisconnectDevice, connectedAddresses);
    } else {
        establishL2cpClientSocket();
    }
}

/*!
 * Handles outcome of attempts to close external connections.
 */
void QLowEnergyControllerPrivateBluez::activeConnectionTerminationDone()
{
    if (!device1Manager)
        return;

    qCDebug(QT_BT_BLUEZ) << "RemoteDeviceManager finished attempting"
                         << "to close external connections";

    QList<quint16> activeHandles = hciManager->activeLowEnergyConnections();
    if (!activeHandles.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot close pending external BTLE connections. Aborting connect attempt";
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        l2cpDisconnected();
        return;
    } else {
        establishL2cpClientSocket();
    }
}

/*!
 * Establishes the L2CP client socket.
 */
void QLowEnergyControllerPrivateBluez::establishL2cpClientSocket()
{
    //we are already in Connecting state

    l2cpSocket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol, this);
    connect(l2cpSocket, SIGNAL(connected()), this, SLOT(l2cpConnected()));
    connect(l2cpSocket, SIGNAL(disconnected()), this, SLOT(l2cpDisconnected()));
    connect(l2cpSocket, SIGNAL(errorOccurred(QBluetoothSocket::SocketError)), this,
            SLOT(l2cpErrorChanged(QBluetoothSocket::SocketError)));
    connect(l2cpSocket, SIGNAL(readyRead()), this, SLOT(l2cpReadyRead()));

    quint32 addressTypeToUse = (addressType == QLowEnergyController::PublicAddress)
                                    ? BDADDR_LE_PUBLIC : BDADDR_LE_RANDOM;
    if (BluetoothManagement::instance()->isMonitoringEnabled()) {
        // if monitoring is possible and it's private then we force it to the relevant option
        if (BluetoothManagement::instance()->isAddressRandom(remoteDevice)) {
            addressTypeToUse = BDADDR_LE_RANDOM;
        }
    }

    qCDebug(QT_BT_BLUEZ) << "addresstypeToUse:"
                         << (addressTypeToUse == BDADDR_LE_RANDOM
                                 ? QStringLiteral("Random") : QStringLiteral("Public"));

    l2cpSocket->d_ptr->lowEnergySocketType = addressTypeToUse;

    int sockfd = l2cpSocket->socketDescriptor();
    if (sockfd < 0) {
        qCWarning(QT_BT_BLUEZ) << "l2cp socket not initialised";
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_cid = htobs(ATTRIBUTE_CHANNEL_ID);
    addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    convertAddress(localAdapter.toUInt64(), addr.l2_bdaddr.b);

    // bind the socket to the local device
    if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qCWarning(QT_BT_BLUEZ) << qt_error_string(errno);
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    // connect
    // Unbuffered mode required to separate each GATT packet
    l2cpSocket->connectToService(remoteDevice, ATTRIBUTE_CHANNEL_ID,
                                 QIODevice::ReadWrite | QIODevice::Unbuffered);
    loadSigningDataIfNecessary(LocalSigningKey);
}

void QLowEnergyControllerPrivateBluez::createServicesForCentralIfRequired()
{
    bool ok = false;
    int value = qEnvironmentVariableIntValue("QT_DEFAULT_CENTRAL_SERVICES", &ok);
    if (Q_UNLIKELY(ok && value == 0))
        return; //nothing to do

    //do not add the services each time we start a connection
    if (localServices.contains(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::GenericAccess)))
        return;

    qCDebug(QT_BT_BLUEZ) << "Creating default GAP/GATT services";

    //populate Generic Access service
    //for now the values are static
    QLowEnergyServiceData gapServiceData;
    gapServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    gapServiceData.setUuid(QBluetoothUuid::ServiceClassUuid::GenericAccess);

    QLowEnergyCharacteristicData gapDeviceName;
    gapDeviceName.setUuid(QBluetoothUuid::CharacteristicType::DeviceName);
    gapDeviceName.setProperties(QLowEnergyCharacteristic::Read);

    QBluetoothLocalDevice mainAdapter;
    gapDeviceName.setValue(mainAdapter.name().toLatin1()); //static name

    QLowEnergyCharacteristicData gapAppearance;
    gapAppearance.setUuid(QBluetoothUuid::CharacteristicType::Appearance);
    gapAppearance.setProperties(QLowEnergyCharacteristic::Read);
    gapAppearance.setValue(QByteArray::fromHex("80")); // Generic Computer (0x80)

    QLowEnergyCharacteristicData gapPrivacyFlag;
    gapPrivacyFlag.setUuid(QBluetoothUuid::CharacteristicType::PeripheralPrivacyFlag);
    gapPrivacyFlag.setProperties(QLowEnergyCharacteristic::Read);
    gapPrivacyFlag.setValue(QByteArray::fromHex("00")); // disable privacy

    gapServiceData.addCharacteristic(gapDeviceName);
    gapServiceData.addCharacteristic(gapAppearance);
    gapServiceData.addCharacteristic(gapPrivacyFlag);

    Q_Q(QLowEnergyController);
    QLowEnergyService *service = addServiceHelper(gapServiceData);
    if (service)
        service->setParent(q);

    QLowEnergyServiceData gattServiceData;
    gattServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    gattServiceData.setUuid(QBluetoothUuid::ServiceClassUuid::GenericAttribute);

    QLowEnergyCharacteristicData serviceChangedChar;
    serviceChangedChar.setUuid(QBluetoothUuid::CharacteristicType::ServiceChanged);
    serviceChangedChar.setProperties(QLowEnergyCharacteristic::Indicate);
    //arbitrary range of 2 bit handle range (1-4
    serviceChangedChar.setValue(QByteArray::fromHex("0104"));

    const QLowEnergyDescriptorData clientConfig(
                        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                        QByteArray(2, 0));
    serviceChangedChar.addDescriptor(clientConfig);
    gattServiceData.addCharacteristic(serviceChangedChar);

    service = addServiceHelper(gattServiceData);
    if (service)
        service->setParent(q);
}

void QLowEnergyControllerPrivateBluez::l2cpConnected()
{
    Q_Q(QLowEnergyController);

    securityLevelValue = securityLevel();
    exchangeMTU();

    setState(QLowEnergyController::ConnectedState);
    emit q->connected();
}

void QLowEnergyControllerPrivateBluez::disconnectFromDevice()
{
    setState(QLowEnergyController::ClosingState);
    if (l2cpSocket)
        l2cpSocket->close();
    resetController();

    // this may happen when RemoteDeviceManager::JobType::JobDisconnectDevice
    // is pending.
    if (!l2cpSocket) {
        qWarning(QT_BT_BLUEZ) << "Unexpected closure of device. Cleaning up internal states.";
        l2cpDisconnected();
    }
}

void QLowEnergyControllerPrivateBluez::l2cpDisconnected()
{
    Q_Q(QLowEnergyController);

    if (role == QLowEnergyController::PeripheralRole) {
        storeClientConfigurations();
        remoteDevice.clear();
        remoteName.clear();
    }
    invalidateServices();
    resetController();
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

void QLowEnergyControllerPrivateBluez::l2cpErrorChanged(QBluetoothSocket::SocketError e)
{
    switch (e) {
    case QBluetoothSocket::SocketError::HostNotFoundError:
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        qCDebug(QT_BT_BLUEZ) << "The passed remote device address cannot be found";
        break;
    case QBluetoothSocket::SocketError::NetworkError:
        setError(QLowEnergyController::NetworkError);
        qCDebug(QT_BT_BLUEZ) << "Network IO error while talking to LE device";
        break;
    case QBluetoothSocket::SocketError::RemoteHostClosedError:
        setError(QLowEnergyController::RemoteHostClosedError);
        qCDebug(QT_BT_BLUEZ) << "Remote host closed the connection";
        break;
    case QBluetoothSocket::SocketError::UnknownSocketError:
    case QBluetoothSocket::SocketError::UnsupportedProtocolError:
    case QBluetoothSocket::SocketError::OperationError:
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
    default:
        // these errors shouldn't happen -> as it means
        // the code in this file has bugs
        qCDebug(QT_BT_BLUEZ) << "Unknown l2cp socket error: " << e << l2cpSocket->errorString();
        setError(QLowEnergyController::UnknownError);
        break;
    }

    invalidateServices();
    resetController();
    setState(QLowEnergyController::UnconnectedState);
}


void QLowEnergyControllerPrivateBluez::resetController()
{
    openRequests.clear();
    openPrepareWriteRequests.clear();
    scheduledIndications.clear();
    indicationInFlight = false;
    requestPending = false;
    encryptionChangePending = false;
    receivedMtuExchangeRequest = false;
    mtuSize = ATT_DEFAULT_LE_MTU;
    securityLevelValue = -1;
    connectionHandle = 0;

    if (role == QLowEnergyController::PeripheralRole) {
        // public API behavior requires stop of advertisement
        if (advertiser) {
            advertiser->stopAdvertising();
            delete advertiser;
            advertiser = nullptr;
        }
        localAttributes.clear();
    }
}

void QLowEnergyControllerPrivateBluez::restartRequestTimer()
{
    if (!requestTimer)
        return;

    if (gattRequestTimeout > 0)
        requestTimer->start(gattRequestTimeout);
}

void QLowEnergyControllerPrivateBluez::l2cpReadyRead()
{
    const QByteArray incomingPacket = l2cpSocket->readAll();
    qCDebug(QT_BT_BLUEZ) << "Received size:" << incomingPacket.size() << "data:"
                         << incomingPacket.toHex();
    if (incomingPacket.isEmpty())
        return;

    const QBluezConst::AttCommand command =
            static_cast<QBluezConst::AttCommand>(incomingPacket.constData()[0]);
    switch (command) {
    case QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_NOTIFICATION: {
        processUnsolicitedReply(incomingPacket);
        return;
    }
    case QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_INDICATION: {
        //send confirmation
        QByteArray packet;
        packet.append(static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_CONFIRMATION));
        sendPacket(packet);

        processUnsolicitedReply(incomingPacket);
        return;
    }
    //--------------------------------------------------
    // Peripheral side packet handling
    case QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST:
        handleExchangeMtuRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST:
        handleFindInformationRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_FIND_BY_TYPE_VALUE_REQUEST:
        handleFindByTypeValueRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST:
        handleReadByTypeRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_READ_REQUEST:
        handleReadRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST:
        handleReadBlobRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_READ_MULTIPLE_REQUEST:
        handleReadMultipleRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST:
        handleReadByGroupTypeRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST:
    case QBluezConst::AttCommand::ATT_OP_WRITE_COMMAND:
    case QBluezConst::AttCommand::ATT_OP_SIGNED_WRITE_COMMAND:
        handleWriteRequestOrCommand(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST:
        handlePrepareWriteRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST:
        handleExecuteWriteRequest(incomingPacket);
        return;
    case QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_CONFIRMATION:
        if (indicationInFlight) {
            indicationInFlight = false;
            sendNextIndication();
        } else {
            qCWarning(QT_BT_BLUEZ) << "received unexpected handle value confirmation";
        }
        return;
    //--------------------------------------------------
    default:
        //only solicited replies finish pending requests
        requestPending = false;
        break;
    }

    if (openRequests.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Received unexpected packet from peer, disconnecting.";
        disconnectFromDevice();
        return;
    }

    const Request request = openRequests.dequeue();
    processReply(request, incomingPacket);

    sendNextPendingRequest();
}

/*!
 * Called when the request for socket encryption has been
 * processed by the kernel. Such requests take time as the kernel
 * has to renegotiate the link parameters with the remote device.
 *
 * Therefore any such request delays the pending ATT commands until this
 * callback is called. The first pending request in the queue is the request
 * that triggered the encryption request.
 */
void QLowEnergyControllerPrivateBluez::encryptionChangedEvent(
        const QBluetoothAddress &address, bool wasSuccess)
{
    if (!encryptionChangePending) // somebody else caused change event
        return;

    if (remoteDevice != address)
        return;

    securityLevelValue = securityLevel();

    // On success continue to process ATT command queue
    if (!wasSuccess) {
        // We could not increase the security of the link
        // The next request was requeued due to security error
        // skip it to avoid endless loop of security negotiations
        Q_ASSERT(!openRequests.isEmpty());
        Request failedRequest = openRequests.takeFirst();

        if (failedRequest.command == QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST) {
            // Failing write requests trigger some sort of response
            uint ref = failedRequest.reference.toUInt();
            const QLowEnergyHandle charHandle = (ref & 0xffff);
            const QLowEnergyHandle descriptorHandle = ((ref >> 16) & 0xffff);

            QSharedPointer<QLowEnergyServicePrivate> service
                                                = serviceForHandle(charHandle);
            if (!service.isNull() && service->characteristicList.contains(charHandle)) {
                if (!descriptorHandle)
                    service->setError(QLowEnergyService::CharacteristicWriteError);
                else
                    service->setError(QLowEnergyService::DescriptorWriteError);
            }
        } else if (failedRequest.command == QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST) {
            uint handleData = failedRequest.reference.toUInt();
            const QLowEnergyHandle attrHandle = (handleData & 0xffff);
            const QByteArray newValue = failedRequest.reference2.toByteArray();

            // Prepare command failed, cancel pending prepare queue on
            // the device. The appropriate (Descriptor|Characteristic)WriteError
            // is emitted too once the execute write request comes through
            sendExecuteWriteRequest(attrHandle, newValue, true);
        }
    }

    encryptionChangePending = false;
    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::sendPacket(const QByteArray &packet)
{
    qint64 result = l2cpSocket->write(packet.constData(),
                                      packet.size());
    // We ignore result == 0 which is likely to be caused by EAGAIN.
    // This packet is effectively discarded but the controller can still recover

    if (result == -1) {
        qCDebug(QT_BT_BLUEZ) << "Cannot write L2CP packet:" << Qt::hex
                             << packet.toHex()
                             << l2cpSocket->errorString();
        setError(QLowEnergyController::NetworkError);
    } else if (result < packet.size()) {
        qCWarning(QT_BT_BLUEZ) << "L2CP write request incomplete:"
                               << result << "of" << packet.size();
    }

}

void QLowEnergyControllerPrivateBluez::sendNextPendingRequest()
{
    if (openRequests.isEmpty() || requestPending || encryptionChangePending)
        return;

    const Request &request = openRequests.head();
//    qCDebug(QT_BT_BLUEZ) << "Sending request, type:" << Qt::hex << request.command
//             << request.payload.toHex();

    requestPending = true;
    restartRequestTimer();
    sendPacket(request.payload);
}

QLowEnergyHandle parseReadByTypeCharDiscovery(
        QLowEnergyServicePrivate::CharData *charData,
        const char *data, quint16 elementLength)
{
    Q_ASSERT(charData);
    Q_ASSERT(data);
    Q_ASSERT(elementLength >= 5);

    QLowEnergyHandle attributeHandle = bt_get_le16(&data[0]);
    charData->properties =
            (QLowEnergyCharacteristic::PropertyTypes)(data[2] & 0xff);
    charData->valueHandle = bt_get_le16(&data[3]);

    // Bluetooth LE data comes as little endian
    if (elementLength == 7) // 16 bit uuid
        charData->uuid = QBluetoothUuid(bt_get_le16(&data[5]));
    else if (elementLength == 21) // 128 bit uuid
        charData->uuid = QUuid::fromBytes(&data[5], QSysInfo::LittleEndian);

    qCDebug(QT_BT_BLUEZ) << "Found handle:" << Qt::hex << attributeHandle
             << "properties:" << charData->properties
             << "value handle:" << charData->valueHandle
             << "uuid:" << charData->uuid.toString();

    return attributeHandle;
}

QLowEnergyHandle parseReadByTypeIncludeDiscovery(
        QList<QBluetoothUuid> *foundServices,
        const char *data, quint16 elementLength)
{
    Q_ASSERT(foundServices);
    Q_ASSERT(data);
    Q_ASSERT(elementLength >= 6);

    QLowEnergyHandle attributeHandle = bt_get_le16(&data[0]);

    // the next 2 elements are not required as we have discovered
    // all (primary/secondary) services already. Now we are only
    // interested in their relationship to each other
    // data[2] -> included service start handle
    // data[4] -> included service end handle

    // TODO: Spec v. 5.3, Vol. 3, Part G, 4.5.1 mentions that only
    // 16-bit UUID can be returned here. If the UUID is 128-bit,
    // then it is omitted from the response, and should be requested
    // separately with the ATT_READ_REQ command.

    if (elementLength == 8) //16 bit uuid
        foundServices->append(QBluetoothUuid(bt_get_le16(&data[6])));
    else if (elementLength == 22) // 128 bit uuid
        foundServices->append(QUuid::fromBytes(&data[6], QSysInfo::LittleEndian));

    qCDebug(QT_BT_BLUEZ) << "Found included service: " << Qt::hex
                         << attributeHandle << "uuid:" << *foundServices;

    return attributeHandle;
}

Q_DECL_COLD_FUNCTION
static void reportMalformedData(QBluezConst::AttCommand cmd, const QByteArray &response)
{
    qCDebug(QT_BT_BLUEZ, "%s malformed data: %s", qt_getEnumName(cmd),
            response.toHex().constData());
}

void QLowEnergyControllerPrivateBluez::processReply(
        const Request &request, const QByteArray &response)
{
    Q_Q(QLowEnergyController);

    // We already have an isEmpty() check at the only calling site that reads
    // incoming data, so Q_ASSERT is enough.
    Q_ASSERT(!response.isEmpty());

    QBluezConst::AttCommand command = static_cast<QBluezConst::AttCommand>(response.constData()[0]);

    bool isErrorResponse = false;
    // if error occurred 2. byte is previous request type
    if (command == QBluezConst::AttCommand::ATT_OP_ERROR_RESPONSE) {
        if (!dumpErrorInformation(response))
            return;
        command = static_cast<QBluezConst::AttCommand>(response.constData()[1]);
        isErrorResponse = true;
    }

    switch (command) {
    case QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST: // in case of error
    case QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_RESPONSE: {
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST);
        quint16 oldMtuSize = mtuSize;
        if (isErrorResponse) {
            mtuSize = ATT_DEFAULT_LE_MTU;
        } else {
            if (response.size() < 3) {
                reportMalformedData(command, response);
                break;
            }
            const char *data = response.constData();
            quint16 mtu = bt_get_le16(&data[1]);
            mtuSize = mtu;
            if (mtuSize < ATT_DEFAULT_LE_MTU)
                mtuSize = ATT_DEFAULT_LE_MTU;

            qCDebug(QT_BT_BLUEZ) << "Server MTU:" << mtu << "resulting mtu:" << mtuSize;
        }
        if (oldMtuSize != mtuSize)
            emit q->mtuChanged(mtuSize);
    } break;
    case QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST: // in case of error
    case QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_RESPONSE: {
        // Discovering services
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST);

        const quint16 type = request.reference.toUInt();

        if (isErrorResponse) {
            if (type == GATT_SECONDARY_SERVICE) {
                setState(QLowEnergyController::DiscoveredState);
                q->discoveryFinished();
            } else { // search for secondary services
                sendReadByGroupRequest(0x0001, 0xFFFF, GATT_SECONDARY_SERVICE);
            }
            break;
        }

        // response[1] == elementLength. According to the spec it should be
        // at least 4 bytes. See Spec v5.3, Vol 3, Part F, 3.4.4.10
        if (response.size() < 2 || response[1] < 4) {
            reportMalformedData(command, response);
            break;
        }

        QLowEnergyHandle start = 0, end = 0;
        const quint16 elementLength = response.constData()[1]; // value checked above
        const quint16 numElements = (response.size() - 2) / elementLength;
        quint16 offset = 2;
        const char *data = response.constData();
        for (int i = 0; i < numElements; i++) {
            start = bt_get_le16(&data[offset]);
            end = bt_get_le16(&data[offset+2]);

            QBluetoothUuid uuid;
            if (elementLength == 6) //16 bit uuid
                uuid = QBluetoothUuid(bt_get_le16(&data[offset+4]));
            else if (elementLength == 20) //128 bit uuid
                uuid = QUuid::fromBytes(&data[offset+4], QSysInfo::LittleEndian);
            //else -> do nothing

            offset += elementLength;


            qCDebug(QT_BT_BLUEZ) << "Found uuid:" << uuid << "start handle:" << Qt::hex
                     << start << "end handle:" << end;

            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = uuid;
            priv->startHandle = start;
            priv->endHandle = end;
            if (type != GATT_PRIMARY_SERVICE) //unset PrimaryService bit
                priv->type &= ~QLowEnergyService::PrimaryService;
            priv->setController(this);

            QSharedPointer<QLowEnergyServicePrivate> pointer(priv);

            serviceList.insert(uuid, pointer);
            emit q->serviceDiscovered(uuid);
        }

        if (end != 0xFFFF) {
            sendReadByGroupRequest(end+1, 0xFFFF, type);
        } else {
            if (type == GATT_SECONDARY_SERVICE) {
                setState(QLowEnergyController::DiscoveredState);
                emit q->discoveryFinished();
            } else { // search for secondary services
                sendReadByGroupRequest(0x0001, 0xFFFF, GATT_SECONDARY_SERVICE);
            }
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST: // in case of error
    case QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_RESPONSE: {
        // Discovering characteristics
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST);

        QSharedPointer<QLowEnergyServicePrivate> p =
                request.reference.value<QSharedPointer<QLowEnergyServicePrivate> >();
        const quint16 attributeType = request.reference2.toUInt();

        if (isErrorResponse) {
            if (attributeType == GATT_CHARACTERISTIC) {
                // we reached end of service handle
                // just finished up characteristic discovery
                // continue with values of characteristics
                if (!p->characteristicList.isEmpty()) {
                    readServiceValues(p->uuid, true);
                } else {
                    // discovery finished since the service doesn't have any
                    // characteristics
                    p->setState(QLowEnergyService::RemoteServiceDiscovered);
                }
            } else if (attributeType == GATT_INCLUDED_SERVICE) {
                // finished up include discovery
                // continue with characteristic discovery
                sendReadByTypeRequest(p, p->startHandle, GATT_CHARACTERISTIC);
            }
            break;
        }

        /* packet format:
         * if GATT_CHARACTERISTIC discovery (Spec 5.3, Vol. 3, Part G, 4.6)
         *      <opcode><elementLength>
         *          [<handle><property><charHandle><uuid>]+
         * The minimum elementLength is 7 bytes (uuid is always included)
         *
         * if GATT_INCLUDE discovery (Spec 5.3, Vol. 3, Part G, 4.5.1)
         *      <opcode><elementLength>
         *          [<handle><startHandle_included><endHandle_included><uuid>]+
         *  The minimum elementLength is 6 bytes (uuid can be omitted).
         *
         *  The uuid can be 16 or 128 bit.
         */

        const quint8 minimumElementLength = attributeType == GATT_CHARACTERISTIC ? 7 : 6;
        if (response.size() < 2 || response[1] < minimumElementLength) {
            reportMalformedData(command, response);
            break;
        }

        QLowEnergyHandle lastHandle;
        const quint16 elementLength = response.constData()[1];
        const quint16 numElements = (response.size() - 2) / elementLength;
        quint16 offset = 2;
        const char *data = response.constData();
        for (int i = 0; i < numElements; i++) {
            if (attributeType == GATT_CHARACTERISTIC) {
                QLowEnergyServicePrivate::CharData characteristic;
                lastHandle = parseReadByTypeCharDiscovery(
                            &characteristic, &data[offset], elementLength);
                p->characteristicList[lastHandle] = characteristic;
                offset += elementLength;
            } else if (attributeType == GATT_INCLUDED_SERVICE) {
                QList<QBluetoothUuid> includedServices;
                lastHandle = parseReadByTypeIncludeDiscovery(
                            &includedServices, &data[offset], elementLength);
                p->includedServices = includedServices;
                for (const QBluetoothUuid &uuid : std::as_const(includedServices)) {
                    if (serviceList.contains(uuid))
                        serviceList[uuid]->type |= QLowEnergyService::IncludedService;
                }
            }
        }

        if (lastHandle + 1 < p->endHandle) { // more chars to discover
            sendReadByTypeRequest(p, lastHandle + 1, attributeType);
        } else {
            if (attributeType == GATT_INCLUDED_SERVICE)
                sendReadByTypeRequest(p, p->startHandle, GATT_CHARACTERISTIC);
            else
                readServiceValues(p->uuid, true);
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_READ_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_READ_RESPONSE: {
        //Reading characteristics and descriptors
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_READ_REQUEST);

        uint handleData = request.reference.toUInt();
        const QLowEnergyHandle charHandle = (handleData & 0xffff);
        const QLowEnergyHandle descriptorHandle = ((handleData >> 16) & 0xffff);

        QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
        Q_ASSERT(!service.isNull());
        bool isServiceDiscoveryRun
                = !(service->state == QLowEnergyService::RemoteServiceDiscovered);

        if (isErrorResponse) {
            Q_ASSERT(!encryptionChangePending);
            QBluezConst::AttError err = static_cast<QBluezConst::AttError>(response.constData()[4]);
            encryptionChangePending = increaseEncryptLevelfRequired(err);
            if (encryptionChangePending) {
                // Just requested a security level change.
                // Retry the same command again once the change has happened
                openRequests.prepend(request);
                break;
            } else if (!isServiceDiscoveryRun) {
                // not encryption problem -> abort readCharacteristic()/readDescriptor() run
                if (!descriptorHandle)
                    service->setError(QLowEnergyService::CharacteristicReadError);
                else
                    service->setError(QLowEnergyService::DescriptorReadError);
            }
        } else {
            if (!descriptorHandle)
                updateValueOfCharacteristic(charHandle, response.mid(1), NEW_VALUE);
            else
                updateValueOfDescriptor(charHandle, descriptorHandle,
                                        response.mid(1), NEW_VALUE);

            if (response.size() == mtuSize) {
                qCDebug(QT_BT_BLUEZ) << "Switching to blob reads for"
                         << charHandle << descriptorHandle
                         << service->characteristicList[charHandle].uuid.toString();
                // Potentially more data -> switch to blob reads
                readServiceValuesByOffset(handleData, mtuSize-1,
                                          request.reference2.toBool());
                break;
            } else if (!isServiceDiscoveryRun) {
                // readCharacteristic() or readDescriptor() ongoing
                if (!descriptorHandle) {
                    QLowEnergyCharacteristic ch(service, charHandle);
                    emit service->characteristicRead(ch, response.mid(1));
                } else {
                    QLowEnergyDescriptor descriptor(service, charHandle, descriptorHandle);
                    emit service->descriptorRead(descriptor, response.mid(1));
                }
                break;
            }
        }

        if (request.reference2.toBool() && isServiceDiscoveryRun) {
            // we only run into this code path during the initial service discovery
            // and not when processing readCharacteristics() after service discovery

            //last characteristic -> progress to descriptor discovery
            //last descriptor -> service discovery is done
            if (!descriptorHandle)
                discoverServiceDescriptors(service->uuid);
            else
                service->setState(QLowEnergyService::RemoteServiceDiscovered);
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_READ_BLOB_RESPONSE: {
        //Reading characteristic or descriptor with value longer value than MTU
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST);

        uint handleData = request.reference.toUInt();
        const QLowEnergyHandle charHandle = (handleData & 0xffff);
        const QLowEnergyHandle descriptorHandle = ((handleData >> 16) & 0xffff);

        QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
        Q_ASSERT(!service.isNull());

        /*
         * READ_BLOB does not require encryption setup code. BLOB commands
         * are only issued after read request if the read request is too long
         * for single MTU. The preceding read request would have triggered
         * the setup of the encryption already.
         */
        if (!isErrorResponse) {
            quint16 length = 0;
            if (!descriptorHandle)
                length = updateValueOfCharacteristic(charHandle, response.mid(1), APPEND_VALUE);
            else
                length = updateValueOfDescriptor(charHandle, descriptorHandle,
                                        response.mid(1), APPEND_VALUE);

            if (response.size() == mtuSize) {
                readServiceValuesByOffset(handleData, length,
                                          request.reference2.toBool());
                break;
            } else if (service->state == QLowEnergyService::RemoteServiceDiscovered) {
                // readCharacteristic() or readDescriptor() ongoing
                if (!descriptorHandle) {
                    QLowEnergyCharacteristic ch(service, charHandle);
                    emit service->characteristicRead(ch, ch.value());
                } else {
                    QLowEnergyDescriptor descriptor(service, charHandle, descriptorHandle);
                    emit service->descriptorRead(descriptor, descriptor.value());
                }
                break;
            }
        } else {
            qWarning() << "READ BLOB for char:" << charHandle
                       << "descriptor:" << descriptorHandle << "on service"
                       << service->uuid.toString() << "failed (service discovery run:"
                       << (service->state == QLowEnergyService::RemoteServiceDiscovered) << ")";
        }

        if (request.reference2.toBool()) {
            //last overlong characteristic -> progress to descriptor discovery
            //last overlong descriptor -> service discovery is done

            if (!descriptorHandle)
                discoverServiceDescriptors(service->uuid);
            else
                service->setState(QLowEnergyService::RemoteServiceDiscovered);
        }

    } break;
    case QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_RESPONSE: {
        //Discovering descriptors
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST);

        /* packet format:
         *  <opcode><format>[<handle><descriptor_uuid>]+
         *
         *  The uuid can be 16 or 128 bit which is indicated by format.
         */

        QList<QLowEnergyHandle> keys = request.reference.value<QList<QLowEnergyHandle> >();
        if (keys.isEmpty()) {
            qCWarning(QT_BT_BLUEZ) << "Descriptor discovery for unknown characteristic received";
            break;
        }
        QLowEnergyHandle charHandle = keys.first();

        QSharedPointer<QLowEnergyServicePrivate> p =
                serviceForHandle(charHandle);
        Q_ASSERT(!p.isNull());

        if (isErrorResponse) {
            if (keys.size() == 1) {
                // no more descriptors to discover
                readServiceValues(p->uuid, false); //read descriptor values
            } else {
                // hop to the next descriptor
                keys.removeFirst();
                discoverNextDescriptor(p, keys, keys.first());
            }
            break;
        }

        // Spec 5.3, Vol. 3, Part F, 3.4.3.2
        if (response.size() < 6) {
            reportMalformedData(command, response);
            break;
        }

        const quint8 format = response[1];
        quint16 elementLength;
        switch (format) {
        case 0x01:
            elementLength = 2 + 2; //sizeof(QLowEnergyHandle) + 16bit uuid
            break;
        case 0x02:
            elementLength = 2 + 16; //sizeof(QLowEnergyHandle) + 128bit uuid
            break;
        default:
            qCWarning(QT_BT_BLUEZ) << "Unknown format in FIND_INFORMATION_RESPONSE";
            return;
        }

        const quint16 numElements = (response.size() - 2) / elementLength;

        quint16 offset = 2;
        QLowEnergyHandle descriptorHandle {};
        QBluetoothUuid uuid;
        const char *data = response.constData();
        for (int i = 0; i < numElements; i++) {
            descriptorHandle = bt_get_le16(&data[offset]);

            if (format == 0x01)
                uuid = QBluetoothUuid(bt_get_le16(&data[offset+2]));
            else if (format == 0x02)
                uuid = QUuid::fromBytes(&data[offset+2], QSysInfo::LittleEndian);

            offset += elementLength;

            // ignore all attributes which are not of type descriptor
            // examples are the characteristics value or
            bool ok = false;
            quint16 shortUuid = uuid.toUInt16(&ok);
            if (ok && shortUuid >= QLowEnergyServicePrivate::PrimaryService
                   && shortUuid <= QLowEnergyServicePrivate::Characteristic){
                qCDebug(QT_BT_BLUEZ) << "Suppressing primary/characteristic" << Qt::hex << shortUuid;
                continue;
            }

            // ignore value handle
            if (descriptorHandle == p->characteristicList[charHandle].valueHandle) {
                qCDebug(QT_BT_BLUEZ) << "Suppressing char handle" << Qt::hex << descriptorHandle;
                continue;
            }

            QLowEnergyServicePrivate::DescData data;
            data.uuid = uuid;
            p->characteristicList[charHandle].descriptorList.insert(
                        descriptorHandle, data);

            qCDebug(QT_BT_BLUEZ) << "Descriptor found, uuid:"
                                 << uuid.toString()
                                 << "descriptor handle:" << Qt::hex << descriptorHandle;
        }

        const QLowEnergyHandle nextPotentialHandle = descriptorHandle + 1;
        if (keys.size() == 1) {
            // Reached last characteristic of service

            // The endhandle of a service is always the last handle of
            // the current service. We must either continue until we have reached
            // the starting handle of the next service (endHandle+1) or
            // the last physical handle address (0xffff). Note that
            // the endHandle of the last service on the device is 0xffff.

            if ((p->endHandle != 0xffff && nextPotentialHandle >= p->endHandle + 1)
                    || (descriptorHandle == 0xffff)) {
                keys.removeFirst();
                // last descriptor of last characteristic found
                // continue with reading descriptor values
                readServiceValues(p->uuid, false);
            } else {
                discoverNextDescriptor(p, keys, nextPotentialHandle);
            }
        } else {
            if (nextPotentialHandle >= keys[1]) //reached next char
                keys.removeFirst();
            discoverNextDescriptor(p, keys, nextPotentialHandle);
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_WRITE_RESPONSE: {
        //Write command response
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST);

        uint ref = request.reference.toUInt();
        const QLowEnergyHandle charHandle = (ref & 0xffff);
        const QLowEnergyHandle descriptorHandle = ((ref >> 16) & 0xffff);

        QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
        if (service.isNull() || !service->characteristicList.contains(charHandle))
            break;

        if (isErrorResponse) {
            Q_ASSERT(!encryptionChangePending);
            QBluezConst::AttError err = static_cast<QBluezConst::AttError>(response.constData()[4]);
            encryptionChangePending = increaseEncryptLevelfRequired(err);
            if (encryptionChangePending) {
                openRequests.prepend(request);
                break;
            }

            if (!descriptorHandle)
                service->setError(QLowEnergyService::CharacteristicWriteError);
            else
                service->setError(QLowEnergyService::DescriptorWriteError);
            break;
        }

        const QByteArray newValue = request.reference2.toByteArray();
        if (!descriptorHandle) {
            QLowEnergyCharacteristic ch(service, charHandle);
            if (ch.properties() & QLowEnergyCharacteristic::Read)
                updateValueOfCharacteristic(charHandle, newValue, NEW_VALUE);
            emit service->characteristicWritten(ch, newValue);
        } else {
            updateValueOfDescriptor(charHandle, descriptorHandle, newValue, NEW_VALUE);
            QLowEnergyDescriptor descriptor(service, charHandle, descriptorHandle);
            emit service->descriptorWritten(descriptor, newValue);
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_RESPONSE: {
        //Prepare write command response
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST);

        uint handleData = request.reference.toUInt();
        const QLowEnergyHandle attrHandle = (handleData & 0xffff);
        const QByteArray newValue = request.reference2.toByteArray();
        const int writtenPayload = ((handleData >> 16) & 0xffff);

        if (isErrorResponse) {
            Q_ASSERT(!encryptionChangePending);
            QBluezConst::AttError err = static_cast<QBluezConst::AttError>(response.constData()[4]);
            encryptionChangePending = increaseEncryptLevelfRequired(err);
            if (encryptionChangePending) {
                openRequests.prepend(request);
                break;
            }
            //emits error on cancellation and aborts existing prepare reuqests
            sendExecuteWriteRequest(attrHandle, newValue, true);
        } else {
            if (writtenPayload < newValue.size()) {
                sendNextPrepareWriteRequest(attrHandle, newValue, writtenPayload);
            } else {
                sendExecuteWriteRequest(attrHandle, newValue, false);
            }
        }
    } break;
    case QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST: // error case
    case QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_RESPONSE: {
        // right now used in connection with long characteristic/descriptor value writes
        // not catering for reliable writes
        Q_ASSERT(request.command == QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST);

        uint handleData = request.reference.toUInt();
        const QLowEnergyHandle attrHandle = handleData & 0xffff;
        bool wasCancellation = !((handleData >> 16) & 0xffff);
        const QByteArray newValue = request.reference2.toByteArray();

        // is it a descriptor or characteristic?
        const QLowEnergyDescriptor descriptor = descriptorForHandle(attrHandle);
        QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(attrHandle);
        Q_ASSERT(!service.isNull());

        if (isErrorResponse || wasCancellation) {
            // charHandle == 0 -> cancellation
            if (descriptor.isValid())
                service->setError(QLowEnergyService::DescriptorWriteError);
            else
                service->setError(QLowEnergyService::CharacteristicWriteError);
        } else {
            if (descriptor.isValid()) {
                updateValueOfDescriptor(descriptor.characteristicHandle(),
                                        attrHandle, newValue, NEW_VALUE);
                emit service->descriptorWritten(descriptor, newValue);
            } else {
                QLowEnergyCharacteristic ch(service, attrHandle);
                if (ch.properties() & QLowEnergyCharacteristic::Read)
                    updateValueOfCharacteristic(attrHandle, newValue, NEW_VALUE);
                emit service->characteristicWritten(ch, newValue);
            }
        }
    } break;
    default:
        qCDebug(QT_BT_BLUEZ) << "Unknown packet: " << response.toHex();
        break;
    }
}

void QLowEnergyControllerPrivateBluez::discoverServices()
{
    sendReadByGroupRequest(0x0001, 0xFFFF, GATT_PRIMARY_SERVICE);
}

void QLowEnergyControllerPrivateBluez::sendReadByGroupRequest(
        QLowEnergyHandle start, QLowEnergyHandle end, quint16 type)
{
    //call for primary and secondary services
    quint8 packet[GRP_TYPE_REQ_HEADER_SIZE];

    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST);
    putBtData(start, &packet[1]);
    putBtData(end, &packet[3]);
    putBtData(type, &packet[5]);

    QByteArray data(GRP_TYPE_REQ_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  GRP_TYPE_REQ_HEADER_SIZE);
    qCDebug(QT_BT_BLUEZ) << "Sending read_by_group_type request, startHandle:" << Qt::hex
             << start << "endHandle:" << end << type;

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_REQUEST;
    request.reference = type;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::discoverServiceDetails(const QBluetoothUuid &service,
                                                              QLowEnergyService::DiscoveryMode mode)
{
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_BLUEZ) << "Discovery of unknown service" << service.toString()
                               << "not possible";
        return;
    }

    QSharedPointer<QLowEnergyServicePrivate> serviceData = serviceList.value(service);
    serviceData->mode = mode;
    serviceData->characteristicList.clear();
    sendReadByTypeRequest(serviceData, serviceData->startHandle, GATT_INCLUDED_SERVICE);
}

void QLowEnergyControllerPrivateBluez::sendReadByTypeRequest(
        QSharedPointer<QLowEnergyServicePrivate> serviceData,
        QLowEnergyHandle nextHandle, quint16 attributeType)
{
    quint8 packet[READ_BY_TYPE_REQ_HEADER_SIZE];

    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST);
    putBtData(nextHandle, &packet[1]);
    putBtData(serviceData->endHandle, &packet[3]);
    putBtData(attributeType, &packet[5]);

    QByteArray data(READ_BY_TYPE_REQ_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  READ_BY_TYPE_REQ_HEADER_SIZE);
    qCDebug(QT_BT_BLUEZ) << "Sending read_by_type request, startHandle:" << Qt::hex
             << nextHandle << "endHandle:" << serviceData->endHandle
             << "type:" << attributeType << "packet:" << data.toHex();

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_REQUEST;
    request.reference = QVariant::fromValue(serviceData);
    request.reference2 = attributeType;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

/*!
    \internal

    Reads all values of specific characteristic and descriptor. This function is
    used during the initial service discovery process.

    \a readCharacteristics determines whether we intend to read a characteristic;
    otherwise we read a descriptor.
 */
void QLowEnergyControllerPrivateBluez::readServiceValues(
        const QBluetoothUuid &serviceUuid, bool readCharacteristics)
{
    quint8 packet[READ_REQUEST_HEADER_SIZE];
    if (QT_BT_BLUEZ().isDebugEnabled()) {
        if (readCharacteristics)
            qCDebug(QT_BT_BLUEZ) << "Reading all characteristic values for"
                         << serviceUuid.toString();
        else
            qCDebug(QT_BT_BLUEZ) << "Reading all descriptor values for"
                         << serviceUuid.toString();
    }

    QSharedPointer<QLowEnergyServicePrivate> service = serviceList.value(serviceUuid);

    if (service->mode == QLowEnergyService::SkipValueDiscovery) {
        if (readCharacteristics) {
            // -> continue with descriptor discovery
            discoverServiceDescriptors(service->uuid);
        } else {
            service->setState(QLowEnergyService::RemoteServiceDiscovered);
        }
        return;
    }

    // pair.first -> target attribute
    // pair.second -> context information for read request
    QPair<QLowEnergyHandle, quint32> pair;

    // Create list of attribute handles which need to be read
    QList<QPair<QLowEnergyHandle, quint32> > targetHandles;

    CharacteristicDataMap::const_iterator charIt = service->characteristicList.constBegin();
    for ( ; charIt != service->characteristicList.constEnd(); ++charIt) {
        const QLowEnergyHandle charHandle = charIt.key();
        const QLowEnergyServicePrivate::CharData &charDetails = charIt.value();

        if (readCharacteristics) {
            // Collect handles of all characteristic value attributes

            // Don't try to read writeOnly characteristic
            if (!(charDetails.properties & QLowEnergyCharacteristic::Read))
                continue;

            pair.first = charDetails.valueHandle;
            pair.second  = charHandle;
            targetHandles.append(pair);

        } else {
            // Collect handles of all descriptor attributes
            DescriptorDataMap::const_iterator descIt = charDetails.descriptorList.constBegin();
            for ( ; descIt != charDetails.descriptorList.constEnd(); ++descIt) {
                const QLowEnergyHandle descriptorHandle = descIt.key();

                pair.first = descriptorHandle;
                pair.second = (charHandle | (descriptorHandle << 16));
                targetHandles.append(pair);
            }
        }
    }


    if (targetHandles.isEmpty()) {
        if (readCharacteristics) {
            // none of the characteristics is readable
            // -> continue with descriptor discovery
            discoverServiceDescriptors(service->uuid);
        } else {
            // characteristic w/o descriptors
            service->setState(QLowEnergyService::RemoteServiceDiscovered);
        }
        return;
    }

    for (qsizetype i = 0; i < targetHandles.size(); i++) {
        pair = targetHandles.at(i);
        packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_REQUEST);
        putBtData(pair.first, &packet[1]);

        QByteArray data(READ_REQUEST_HEADER_SIZE, Qt::Uninitialized);
        memcpy(data.data(), packet,  READ_REQUEST_HEADER_SIZE);

        Request request;
        request.payload = data;
        request.command = QBluezConst::AttCommand::ATT_OP_READ_REQUEST;
        request.reference = pair.second;
        // last entry?
        request.reference2 = QVariant((bool)(i + 1 == targetHandles.size()));
        openRequests.enqueue(request);
    }

    sendNextPendingRequest();
}

/*!
    \internal

    This function is used when reading a handle value that is
    longer than the mtuSize.

    The BLOB read request is prepended to the list of
    open requests to finish the current value read up before
    starting the next read request.
 */
void QLowEnergyControllerPrivateBluez::readServiceValuesByOffset(
        uint handleData, quint16 offset, bool isLastValue)
{
    const QLowEnergyHandle charHandle = (handleData & 0xffff);
    const QLowEnergyHandle descriptorHandle = ((handleData >> 16) & 0xffff);

    QByteArray data(READ_BLOB_REQUEST_HEADER_SIZE, Qt::Uninitialized);
    data[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST);

    QLowEnergyHandle handleToRead = charHandle;
    if (descriptorHandle) {
        handleToRead = descriptorHandle;
        qCDebug(QT_BT_BLUEZ) << "Reading descriptor via blob request"
                             << Qt::hex << descriptorHandle;
    } else {
        //charHandle is not the char's value handle
        QSharedPointer<QLowEnergyServicePrivate> service =
                serviceForHandle(charHandle);
        if (!service.isNull()
                && service->characteristicList.contains(charHandle)) {
            handleToRead = service->characteristicList[charHandle].valueHandle;
            qCDebug(QT_BT_BLUEZ) << "Reading characteristic via blob request"
                                 << Qt::hex << handleToRead;
        } else {
            Q_ASSERT(false);
        }
    }

    putBtData(handleToRead, data.data() + 1);
    putBtData(offset, data.data() + 3);

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_READ_BLOB_REQUEST;
    request.reference = handleData;
    request.reference2 = isLastValue;
    openRequests.prepend(request);
}

void QLowEnergyControllerPrivateBluez::discoverServiceDescriptors(
        const QBluetoothUuid &serviceUuid)
{
    qCDebug(QT_BT_BLUEZ) << "Discovering descriptor values for"
                         << serviceUuid.toString();
    QSharedPointer<QLowEnergyServicePrivate> service = serviceList.value(serviceUuid);

    if (service->characteristicList.isEmpty()) { // service has no characteristics
        // implies that characteristic & descriptor discovery can be skipped
        service->setState(QLowEnergyService::RemoteServiceDiscovered);
        return;
    }

    // start handle of all known characteristics
    QList<QLowEnergyHandle> keys = service->characteristicList.keys();
    std::sort(keys.begin(), keys.end());

    discoverNextDescriptor(service, keys, keys[0]);
}

void QLowEnergyControllerPrivateBluez::processUnsolicitedReply(const QByteArray &payload)
{
    Q_ASSERT(!payload.isEmpty());

    const char *data = payload.constData();
    const auto command = static_cast<QBluezConst::AttCommand>(data[0]);
    bool isNotification = (command
                           == QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_NOTIFICATION);

    if (payload.size() < 3) {
        reportMalformedData(command, payload);
        return;
    }

    const QLowEnergyHandle changedHandle = bt_get_le16(&data[1]);

    if (QT_BT_BLUEZ().isDebugEnabled()) {
        if (isNotification)
            qCDebug(QT_BT_BLUEZ) << "Change notification for handle" << Qt::hex << changedHandle;
        else
            qCDebug(QT_BT_BLUEZ) << "Change indication for handle" << Qt::hex << changedHandle;
    }

    const QLowEnergyCharacteristic ch = characteristicForHandle(changedHandle);
    if (ch.isValid() && ch.handle() == changedHandle) {
        if (ch.properties() & QLowEnergyCharacteristic::Read)
            updateValueOfCharacteristic(ch.attributeHandle(), payload.mid(3), NEW_VALUE);
        emit ch.d_ptr->characteristicChanged(ch, payload.mid(3));
    } else {
        qCWarning(QT_BT_BLUEZ) << "Cannot find matching characteristic for "
                                  "notification/indication";
    }
}

void QLowEnergyControllerPrivateBluez::exchangeMTU()
{
    qCDebug(QT_BT_BLUEZ) << "Exchanging MTU";

    quint8 packet[MTU_EXCHANGE_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST);
    putBtData(ATT_MAX_LE_MTU, &packet[1]);

    QByteArray data(MTU_EXCHANGE_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet, MTU_EXCHANGE_HEADER_SIZE);

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_REQUEST;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

int QLowEnergyControllerPrivateBluez::securityLevel() const
{
    int socket = l2cpSocket->socketDescriptor();
    if (socket < 0) {
        qCWarning(QT_BT_BLUEZ) << "Invalid l2cp socket, aborting getting of sec level";
        return -1;
    }

    struct bt_security secData;
    socklen_t length = sizeof(secData);
    memset(&secData, 0, length);

    if (getsockopt(socket, SOL_BLUETOOTH, BT_SECURITY, &secData, &length) == 0) {
        qCDebug(QT_BT_BLUEZ) << "Current l2cp sec level:" << secData.level;
        return secData.level;
    }

    if (errno != ENOPROTOOPT) //older kernel, fall back to L2CAP_LM option
        return -1;

    // cater for older kernels
    int optval;
    length = sizeof(optval);
    if (getsockopt(socket, SOL_L2CAP, L2CAP_LM, &optval, &length) == 0) {
        int level = BT_SECURITY_SDP;
        if (optval & L2CAP_LM_AUTH)
            level = BT_SECURITY_LOW;
        if (optval & L2CAP_LM_ENCRYPT)
            level = BT_SECURITY_MEDIUM;
        if (optval & L2CAP_LM_SECURE)
            level = BT_SECURITY_HIGH;

        qCDebug(QT_BT_BLUEZ) << "Current l2cp sec level (old):" << level;
        return level;
    }

    return -1;
}

bool QLowEnergyControllerPrivateBluez::setSecurityLevel(int level)
{
    if (level > BT_SECURITY_HIGH || level < BT_SECURITY_LOW)
        return false;

    int socket = l2cpSocket->socketDescriptor();
    if (socket < 0) {
        qCWarning(QT_BT_BLUEZ) << "Invalid l2cp socket, aborting setting of sec level";
        return false;
    }

    struct bt_security secData;
    socklen_t length = sizeof(secData);
    memset(&secData, 0, length);
    secData.level = level;

    if (setsockopt(socket, SOL_BLUETOOTH, BT_SECURITY, &secData, length) == 0) {
        qCDebug(QT_BT_BLUEZ) << "Setting new l2cp sec level:" << secData.level;
        return true;
    }

    if (errno != ENOPROTOOPT) //older kernel
        return false;

    int optval = 0;
    switch (level) { // fall through intendeds
        case BT_SECURITY_HIGH:
            optval |= L2CAP_LM_SECURE;
            Q_FALLTHROUGH();
        case BT_SECURITY_MEDIUM:
            optval |= L2CAP_LM_ENCRYPT;
            Q_FALLTHROUGH();
        case BT_SECURITY_LOW:
            optval |= L2CAP_LM_AUTH;
            break;
        default:
            return false;
    }

    if (setsockopt(socket, SOL_L2CAP, L2CAP_LM, &optval, sizeof(optval)) == 0) {
        qCDebug(QT_BT_BLUEZ) << "Old l2cp sec level:" << optval;
        return true;
    }

    return false;
}

void QLowEnergyControllerPrivateBluez::discoverNextDescriptor(
        QSharedPointer<QLowEnergyServicePrivate> serviceData,
        const QList<QLowEnergyHandle> pendingCharHandles,
        const QLowEnergyHandle startingHandle)
{
    Q_ASSERT(!pendingCharHandles.isEmpty());
    Q_ASSERT(!serviceData.isNull());

    qCDebug(QT_BT_BLUEZ) << "Sending find_info request" << Qt::hex
                         << pendingCharHandles << startingHandle;

    quint8 packet[FIND_INFO_REQUEST_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST);

    const QLowEnergyHandle charStartHandle = startingHandle;
    QLowEnergyHandle charEndHandle = 0;
    if (pendingCharHandles.size() == 1) //single characteristic
        charEndHandle = serviceData->endHandle;
    else
        charEndHandle = pendingCharHandles[1] - 1;

    putBtData(charStartHandle, &packet[1]);
    putBtData(charEndHandle, &packet[3]);

    QByteArray data(FIND_INFO_REQUEST_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet, FIND_INFO_REQUEST_HEADER_SIZE);

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_REQUEST;
    request.reference = QVariant::fromValue<QList<QLowEnergyHandle> >(pendingCharHandles);
    request.reference2 = startingHandle;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::sendNextPrepareWriteRequest(
        const QLowEnergyHandle handle, const QByteArray &newValue,
        quint16 offset)
{
    // is it a descriptor or characteristic?
    QLowEnergyHandle targetHandle = 0;
    const QLowEnergyDescriptor descriptor = descriptorForHandle(handle);
    if (descriptor.isValid())
        targetHandle = descriptor.handle();
    else
        targetHandle = characteristicForHandle(handle).handle();

    if (!targetHandle) {
        qCWarning(QT_BT_BLUEZ) << "sendNextPrepareWriteRequest cancelled due to invalid handle"
                               << handle;
        return;
    }

    quint8 packet[PREPARE_WRITE_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST);
    putBtData(targetHandle, &packet[1]); // attribute handle
    putBtData(offset, &packet[3]); // offset into newValue

    qCDebug(QT_BT_BLUEZ) << "Writing long characteristic (prepare):"
                         << Qt::hex << handle;


    const qsizetype maxAvailablePayload = qsizetype(mtuSize) - PREPARE_WRITE_HEADER_SIZE;
    const qsizetype requiredPayload = (std::min)(newValue.size() - offset, maxAvailablePayload);
    const qsizetype dataSize = PREPARE_WRITE_HEADER_SIZE + requiredPayload;

    Q_ASSERT((offset + requiredPayload) <= newValue.size());
    Q_ASSERT(dataSize <= mtuSize);

    QByteArray data(dataSize, Qt::Uninitialized);
    memcpy(data.data(), packet, PREPARE_WRITE_HEADER_SIZE);
    memcpy(&(data.data()[PREPARE_WRITE_HEADER_SIZE]), &(newValue.constData()[offset]),
                                               requiredPayload);

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_REQUEST;
    request.reference = (handle | ((offset + requiredPayload) << 16));
    request.reference2 = newValue;
    openRequests.enqueue(request);
}

/*!
    Sends an "Execute Write Request" for a long characteristic or descriptor write.
    This cannot be used for executes in relation to reliable write requests.

    A cancellation removes all pending prepare write request on the GATT server.
    Otherwise this function sends an execute request for all pending prepare
    write requests.
 */
void QLowEnergyControllerPrivateBluez::sendExecuteWriteRequest(
        const QLowEnergyHandle attrHandle, const QByteArray &newValue,
        bool isCancelation)
{
    quint8 packet[EXECUTE_WRITE_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST);
    if (isCancelation)
        packet[1] = 0x00; // cancel pending write prepare requests
    else
        packet[1] = 0x01; // execute pending write prepare requests

    QByteArray data(EXECUTE_WRITE_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet, EXECUTE_WRITE_HEADER_SIZE);

    qCDebug(QT_BT_BLUEZ) << "Sending Execute Write Request for long characteristic value"
                         << Qt::hex << attrHandle;

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_REQUEST;
    request.reference  = (attrHandle | ((isCancelation ? 0x00 : 0x01) << 16));
    request.reference2 = newValue;
    openRequests.prepend(request);
}


/*!
    Writes long (prepare write request), short (write request)
    and writeWithoutResponse characteristic values.

    TODO Reliable/prepare write across multiple characteristics is not supported
 */
void QLowEnergyControllerPrivateBluez::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    Q_ASSERT(!service.isNull());

    if (!service->characteristicList.contains(charHandle))
        return;

    QLowEnergyServicePrivate::CharData &charData = service->characteristicList[charHandle];
    if (role == QLowEnergyController::PeripheralRole)
        writeCharacteristicForPeripheral(charData, newValue);
    else
        writeCharacteristicForCentral(service, charHandle, charData.valueHandle, newValue, mode);
}

void QLowEnergyControllerPrivateBluez::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descriptorHandle,
        const QByteArray &newValue)
{
    Q_ASSERT(!service.isNull());

    if (role == QLowEnergyController::PeripheralRole)
        writeDescriptorForPeripheral(service, charHandle, descriptorHandle, newValue);
    else
        writeDescriptorForCentral(charHandle, descriptorHandle, newValue);
}

/*!
    \internal

    Reads the value of one specific characteristic.
 */
void QLowEnergyControllerPrivateBluez::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle)
{
    Q_ASSERT(!service.isNull());
    if (!service->characteristicList.contains(charHandle))
        return;

    const QLowEnergyServicePrivate::CharData &charDetails
            = service->characteristicList[charHandle];
    if (!(charDetails.properties & QLowEnergyCharacteristic::Read)) {
        // if this succeeds the device has a bug, char is advertised as
        // non-readable. We try to be permissive and let the remote
        // device answer to the read attempt
        qCWarning(QT_BT_BLUEZ) << "Reading non-readable char" << charHandle;
    }

    quint8 packet[READ_REQUEST_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_REQUEST);
    putBtData(charDetails.valueHandle, &packet[1]);

    QByteArray data(READ_REQUEST_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  READ_REQUEST_HEADER_SIZE);

    qCDebug(QT_BT_BLUEZ) << "Targeted reading characteristic" << Qt::hex << charHandle;

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_READ_REQUEST;
    request.reference = charHandle;
    // reference2 not really required but false prevents service discovery
    // code from running in QBluezConst::AttCommand::ATT_OP_READ_RESPONSE handler
    request.reference2 = false;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descriptorHandle)
{
    Q_ASSERT(!service.isNull());
    if (!service->characteristicList.contains(charHandle))
        return;

    const QLowEnergyServicePrivate::CharData &charDetails
            = service->characteristicList[charHandle];
    if (!charDetails.descriptorList.contains(descriptorHandle))
        return;

    quint8 packet[READ_REQUEST_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_REQUEST);
    putBtData(descriptorHandle, &packet[1]);

    QByteArray data(READ_REQUEST_HEADER_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  READ_REQUEST_HEADER_SIZE);

    qCDebug(QT_BT_BLUEZ) << "Targeted reading descriptor" << Qt::hex << descriptorHandle;

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_READ_REQUEST;
    request.reference = (charHandle | (descriptorHandle << 16));
    // reference2 not really required but false prevents service discovery
    // code from running in QBluezConst::AttCommand::ATT_OP_READ_RESPONSE handler
    request.reference2 = false;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

/*!
 * Returns true if the encryption change was successfully requested.
 * The request is triggered if we got a related ATT error.
 */
bool QLowEnergyControllerPrivateBluez::increaseEncryptLevelfRequired(
        QBluezConst::AttError errorCode)
{
    if (securityLevelValue == BT_SECURITY_HIGH)
        return false;

    switch (errorCode) {
    case QBluezConst::AttError::ATT_ERROR_INSUF_ENCRYPTION:
    case QBluezConst::AttError::ATT_ERROR_INSUF_AUTHENTICATION:
    case QBluezConst::AttError::ATT_ERROR_INSUF_ENCR_KEY_SIZE:
        if (!hciManager->isValid())
            return false;
        if (!hciManager->monitorEvent(HciManager::HciEvent::EVT_ENCRYPT_CHANGE))
            return false;
        if (securityLevelValue != BT_SECURITY_HIGH) {
            qCDebug(QT_BT_BLUEZ) << "Requesting encrypted link";
            if (setSecurityLevel(BT_SECURITY_HIGH)) {
                restartRequestTimer();
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

void QLowEnergyControllerPrivateBluez::handleAdvertisingError()
{
    qCWarning(QT_BT_BLUEZ) << "received advertising error";
    setError(QLowEnergyController::AdvertisingError);
    setState(QLowEnergyController::UnconnectedState);
}

bool QLowEnergyControllerPrivateBluez::checkPacketSize(const QByteArray &packet, int minSize,
                                                  int maxSize)
{
    if (maxSize == -1)
        maxSize = minSize;
    if (Q_LIKELY(packet.size() >= minSize && packet.size() <= maxSize))
        return true;
    qCWarning(QT_BT_BLUEZ) << "client request of type" << packet.at(0)
                           << "has unexpected packet size" << packet.size();
    sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), 0,
                      QBluezConst::AttError::ATT_ERROR_INVALID_PDU);
    return false;
}

bool QLowEnergyControllerPrivateBluez::checkHandle(const QByteArray &packet, QLowEnergyHandle handle)
{
    if (handle != 0 && handle <= lastLocalHandle)
        return true;
    sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                      QBluezConst::AttError::ATT_ERROR_INVALID_HANDLE);
    return false;
}

bool QLowEnergyControllerPrivateBluez::checkHandlePair(QBluezConst::AttCommand request,
                                                       QLowEnergyHandle startingHandle,
                                                       QLowEnergyHandle endingHandle)
{
    if (startingHandle == 0 || startingHandle > endingHandle) {
        qCDebug(QT_BT_BLUEZ) << "handle range invalid";
        sendErrorResponse(request, startingHandle, QBluezConst::AttError::ATT_ERROR_INVALID_HANDLE);
        return false;
    }
    return true;
}

void QLowEnergyControllerPrivateBluez::handleExchangeMtuRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.2

    if (!checkPacketSize(packet, 3))
        return;
    if (receivedMtuExchangeRequest) { // Client must only send this once per connection.
        qCDebug(QT_BT_BLUEZ) << "Client sent extraneous MTU exchange packet";
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), 0,
                          QBluezConst::AttError::ATT_ERROR_REQUEST_NOT_SUPPORTED);
        return;
    }
    receivedMtuExchangeRequest = true;

    // Send reply.
    QByteArray reply(MTU_EXCHANGE_HEADER_SIZE, Qt::Uninitialized);
    reply[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_EXCHANGE_MTU_RESPONSE);
    putBtData(ATT_MAX_LE_MTU, reply.data() + 1);
    sendPacket(reply);

    // Apply requested MTU.
    const quint16 clientRxMtu = bt_get_le16(packet.constData() + 1);
    mtuSize = std::clamp(clientRxMtu, ATT_DEFAULT_LE_MTU, ATT_MAX_LE_MTU);
    qCDebug(QT_BT_BLUEZ) << "MTU request from client:" << clientRxMtu
                         << "effective client RX MTU:" << mtuSize;
    qCDebug(QT_BT_BLUEZ) << "Sending server RX MTU" << ATT_MAX_LE_MTU;
}

void QLowEnergyControllerPrivateBluez::handleFindInformationRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.3.1-2

    if (!checkPacketSize(packet, 5))
        return;
    const QLowEnergyHandle startingHandle = bt_get_le16(packet.constData() + 1);
    const QLowEnergyHandle endingHandle = bt_get_le16(packet.constData() + 3);
    qCDebug(QT_BT_BLUEZ) << "client sends find information request; start:" << startingHandle
                         << "end:" << endingHandle;
    if (!checkHandlePair(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                         endingHandle))
        return;

    QList<Attribute> results = getAttributes(startingHandle, endingHandle);
    if (results.isEmpty()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                          QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_FOUND);
        return;
    }
    ensureUniformUuidSizes(results);

    QByteArray responsePrefix(2, Qt::Uninitialized);
    const int uuidSize = getUuidSize(results.first().type);
    responsePrefix[0] =
            static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_FIND_INFORMATION_RESPONSE);
    responsePrefix[1] = uuidSize == 2 ? 0x1 : 0x2;
    const int elementSize = sizeof(QLowEnergyHandle) + uuidSize;
    const auto elemWriter = [](const Attribute &attr, char *&data) {
        putDataAndIncrement(attr.handle, data);
        putDataAndIncrement(attr.type, data);
    };
    sendListResponse(responsePrefix, elementSize, results, elemWriter);

}

void QLowEnergyControllerPrivateBluez::handleFindByTypeValueRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.3.3-4

    if (!checkPacketSize(packet, 7, mtuSize))
        return;
    const QLowEnergyHandle startingHandle = bt_get_le16(packet.constData() + 1);
    const QLowEnergyHandle endingHandle = bt_get_le16(packet.constData() + 3);
    const quint16 type = bt_get_le16(packet.constData() + 5);
    const QByteArray value = QByteArray::fromRawData(packet.constData() + 7, packet.size() - 7);
    qCDebug(QT_BT_BLUEZ) << "client sends find by type value request; start:" << startingHandle
                         << "end:" << endingHandle << "type:" << type
                         << "value:" << value.toHex();
    if (!checkHandlePair(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                         endingHandle))
        return;

    const auto predicate = [value, this, type](const Attribute &attr) {
        return attr.type == QBluetoothUuid(type) && attr.value == value
                && checkReadPermissions(attr) == QBluezConst::AttError::ATT_ERROR_NO_ERROR;
    };
    const QList<Attribute> results = getAttributes(startingHandle, endingHandle, predicate);
    if (results.isEmpty()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                          QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_FOUND);
        return;
    }

    QByteArray responsePrefix(
            1, static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_FIND_BY_TYPE_VALUE_RESPONSE));
    const int elemSize = 2 * sizeof(QLowEnergyHandle);
    const auto elemWriter = [](const Attribute &attr, char *&data) {
        putDataAndIncrement(attr.handle, data);
        putDataAndIncrement(attr.groupEndHandle, data);
    };
    sendListResponse(responsePrefix, elemSize, results, elemWriter);
}

void QLowEnergyControllerPrivateBluez::handleReadByTypeRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.4.1-2

    if (!checkPacketSize(packet, 7, 21))
        return;
    const QLowEnergyHandle startingHandle = bt_get_le16(packet.constData() + 1);
    const QLowEnergyHandle endingHandle = bt_get_le16(packet.constData() + 3);
    const void * const typeStart = packet.constData() + 5;
    const bool is16BitUuid = packet.size() == 7;
    const bool is128BitUuid = packet.size() == 21;
    QBluetoothUuid type;
    if (is16BitUuid) {
        type = QBluetoothUuid(bt_get_le16(typeStart));
    } else if (is128BitUuid) {
        type = QUuid::fromBytes(typeStart, QSysInfo::LittleEndian);
    } else {
        qCWarning(QT_BT_BLUEZ) << "read by type request has invalid packet size" << packet.size();
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), 0,
                          QBluezConst::AttError::ATT_ERROR_INVALID_PDU);
        return;
    }
    qCDebug(QT_BT_BLUEZ) << "client sends read by type request, start:" << startingHandle
                         << "end:" << endingHandle << "type:" << type;
    if (!checkHandlePair(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                         endingHandle))
        return;

    // Get all attributes with matching type.
    QList<Attribute> results =
            getAttributes(startingHandle, endingHandle,
                          [type](const Attribute &attr) { return attr.type == type; });
    ensureUniformValueSizes(results);

    if (results.isEmpty()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                          QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_FOUND);
        return;
    }

    const QBluezConst::AttError error = checkReadPermissions(results);
    if (error != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)),
                          results.first().handle, error);
        return;
    }

    const qsizetype elementSize = sizeof(QLowEnergyHandle) + results.first().value.size();
    QByteArray responsePrefix(2, Qt::Uninitialized);
    responsePrefix[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BY_TYPE_RESPONSE);
    responsePrefix[1] = elementSize;
    const auto elemWriter = [](const Attribute &attr, char *&data) {
        putDataAndIncrement(attr.handle, data);
        putDataAndIncrement(attr.value, data);
    };
    sendListResponse(responsePrefix, elementSize, results, elemWriter);
}

void QLowEnergyControllerPrivateBluez::handleReadRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.4.3-4

    if (!checkPacketSize(packet, 3))
        return;
    const QLowEnergyHandle handle = bt_get_le16(packet.constData() + 1);
    qCDebug(QT_BT_BLUEZ) << "client sends read request; handle:" << handle;

    if (!checkHandle(packet, handle))
        return;
    const Attribute &attribute = localAttributes.at(handle);
    const QBluezConst::AttError permissionsError = checkReadPermissions(attribute);
    if (permissionsError != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          permissionsError);
        return;
    }

    const qsizetype sentValueLength = (std::min)(attribute.value.size(), qsizetype(mtuSize) - 1);
    QByteArray response(1 + sentValueLength, Qt::Uninitialized);
    response[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_RESPONSE);
    using namespace std;
    memcpy(response.data() + 1, attribute.value.constData(), sentValueLength);
    qCDebug(QT_BT_BLUEZ) << "sending response:" << response.toHex();
    sendPacket(response);
}

void QLowEnergyControllerPrivateBluez::handleReadBlobRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.4.5-6

    if (!checkPacketSize(packet, 5))
        return;
    const QLowEnergyHandle handle = bt_get_le16(packet.constData() + 1);
    const quint16 valueOffset = bt_get_le16(packet.constData() + 3);
    qCDebug(QT_BT_BLUEZ) << "client sends read blob request; handle:" << handle
                         << "offset:" << valueOffset;

    if (!checkHandle(packet, handle))
        return;
    const Attribute &attribute = localAttributes.at(handle);
    const QBluezConst::AttError permissionsError = checkReadPermissions(attribute);
    if (permissionsError != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          permissionsError);
        return;
    }
    if (valueOffset > attribute.value.size()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          QBluezConst::AttError::ATT_ERROR_INVALID_OFFSET);
        return;
    }
    if (attribute.value.size() <= mtuSize - 3) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_LONG);
        return;
    }

    // Yes, this value can be zero.
    const qsizetype sentValueLength = (std::min)(attribute.value.size() - valueOffset,
                                                 qsizetype(mtuSize) - 1);

    QByteArray response(1 + sentValueLength, Qt::Uninitialized);
    response[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BLOB_RESPONSE);
    using namespace std;
    memcpy(response.data() + 1, attribute.value.constData() + valueOffset, sentValueLength);
    qCDebug(QT_BT_BLUEZ) << "sending response:" << response.toHex();
    sendPacket(response);
}

void QLowEnergyControllerPrivateBluez::handleReadMultipleRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.4.7-8

    if (!checkPacketSize(packet, 5, mtuSize))
        return;
    QList<QLowEnergyHandle> handles((packet.size() - 1) / sizeof(QLowEnergyHandle));
    auto *packetPtr = reinterpret_cast<const QLowEnergyHandle *>(packet.constData() + 1);
    for (qsizetype i = 0; i < handles.size(); ++i, ++packetPtr)
        handles[i] = bt_get_le16(packetPtr);
    qCDebug(QT_BT_BLUEZ) << "client sends read multiple request for handles" << handles;

    const auto it = std::find_if(handles.constBegin(), handles.constEnd(),
            [this](QLowEnergyHandle handle) { return handle >= lastLocalHandle; });
    if (it != handles.constEnd()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), *it,
                          QBluezConst::AttError::ATT_ERROR_INVALID_HANDLE);
        return;
    }
    const QList<Attribute> results = getAttributes(handles.first(), handles.last());
    QByteArray response(
            1, static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_MULTIPLE_RESPONSE));
    for (const Attribute &attr : results) {
        const QBluezConst::AttError error = checkReadPermissions(attr);
        if (error != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
            sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), attr.handle,
                              error);
            return;
        }

        // Note: We do not abort if no more values fit into the packet, because we still have to
        //       report possible permission errors for the other handles.
        response += attr.value.left(mtuSize - response.size());
    }

    qCDebug(QT_BT_BLUEZ) << "sending response:" << response.toHex();
    sendPacket(response);
}

void QLowEnergyControllerPrivateBluez::handleReadByGroupTypeRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.4.9-10

    if (!checkPacketSize(packet, 7, 21))
        return;
    const QLowEnergyHandle startingHandle = bt_get_le16(packet.constData() + 1);
    const QLowEnergyHandle endingHandle = bt_get_le16(packet.constData() + 3);
    const bool is16BitUuid = packet.size() == 7;
    const bool is128BitUuid = packet.size() == 21;
    const void * const typeStart = packet.constData() + 5;
    QBluetoothUuid type;
    if (is16BitUuid) {
        type = QBluetoothUuid(bt_get_le16(typeStart));
    } else if (is128BitUuid) {
        type = QUuid::fromBytes(typeStart, QSysInfo::LittleEndian);
    } else {
        qCWarning(QT_BT_BLUEZ) << "read by group type request has invalid packet size"
                               << packet.size();
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), 0,
                          QBluezConst::AttError::ATT_ERROR_INVALID_PDU);
        return;
    }
    qCDebug(QT_BT_BLUEZ) << "client sends read by group type request, start:" << startingHandle
                         << "end:" << endingHandle << "type:" << type;

    if (!checkHandlePair(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                         endingHandle))
        return;
    if (type != QBluetoothUuid(static_cast<quint16>(GATT_PRIMARY_SERVICE))
            && type != QBluetoothUuid(static_cast<quint16>(GATT_SECONDARY_SERVICE))) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                          QBluezConst::AttError::ATT_ERROR_UNSUPPRTED_GROUP_TYPE);
        return;
    }

    QList<Attribute> results =
            getAttributes(startingHandle, endingHandle,
                          [type](const Attribute &attr) { return attr.type == type; });
    if (results.isEmpty()) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), startingHandle,
                          QBluezConst::AttError::ATT_ERROR_ATTRIBUTE_NOT_FOUND);
        return;
    }
    const QBluezConst::AttError error = checkReadPermissions(results);
    if (error != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)),
                          results.first().handle, error);
        return;
    }

    ensureUniformValueSizes(results);

    const qsizetype elementSize = 2 * sizeof(QLowEnergyHandle) + results.first().value.size();
    QByteArray responsePrefix(2, Qt::Uninitialized);
    responsePrefix[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_READ_BY_GROUP_RESPONSE);
    responsePrefix[1] = elementSize;
    const auto elemWriter = [](const Attribute &attr, char *&data) {
        putDataAndIncrement(attr.handle, data);
        putDataAndIncrement(attr.groupEndHandle, data);
        putDataAndIncrement(attr.value, data);
    };
    sendListResponse(responsePrefix, elementSize, results, elemWriter);
}

void QLowEnergyControllerPrivateBluez::updateLocalAttributeValue(
        QLowEnergyHandle handle,
        const QByteArray &value,
        QLowEnergyCharacteristic &characteristic,
        QLowEnergyDescriptor &descriptor)
{
    localAttributes[handle].value = value;
    for (const auto &service : std::as_const(localServices)) {
        if (handle < service->startHandle || handle > service->endHandle)
            continue;
        for (auto charIt = service->characteristicList.begin();
             charIt != service->characteristicList.end(); ++charIt) {
            QLowEnergyServicePrivate::CharData &charData = charIt.value();
            if (handle == charIt.key() + 1) { // Char value decl comes right after char decl.
                charData.value = value;
                characteristic = QLowEnergyCharacteristic(service, charIt.key());
                return;
            }
            for (auto descIt = charData.descriptorList.begin();
                 descIt != charData.descriptorList.end(); ++descIt) {
                if (handle == descIt.key()) {
                    descIt.value().value = value;
                    descriptor = QLowEnergyDescriptor(service, charIt.key(), handle);
                    return;
                }
            }
        }
    }
    qFatal("local services map inconsistent with local attribute map");
}

static bool isNotificationEnabled(quint16 clientConfigValue) { return clientConfigValue & 0x1; }
static bool isIndicationEnabled(quint16 clientConfigValue) { return clientConfigValue & 0x2; }

void QLowEnergyControllerPrivateBluez::writeCharacteristicForPeripheral(
        QLowEnergyServicePrivate::CharData &charData,
        const QByteArray &newValue)
{
    const QLowEnergyHandle valueHandle = charData.valueHandle;
    Q_ASSERT(valueHandle <= lastLocalHandle);
    Attribute &attribute = localAttributes[valueHandle];
    if (newValue.size() < attribute.minLength || newValue.size() > attribute.maxLength) {
        qCWarning(QT_BT_BLUEZ) << "ignoring value of invalid length" << newValue.size()
                               << "for attribute" << valueHandle;
        return;
    }
    attribute.value = newValue;
    charData.value = newValue;
    const bool hasNotifyProperty = attribute.properties & QLowEnergyCharacteristic::Notify;
    const bool hasIndicateProperty
            = attribute.properties & QLowEnergyCharacteristic::Indicate;
    if (!hasNotifyProperty && !hasIndicateProperty)
        return;
    for (const QLowEnergyServicePrivate::DescData &desc : std::as_const(charData.descriptorList)) {
        if (desc.uuid != QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)
            continue;

        // Notify/indicate currently connected client.
        const bool isConnected = state == QLowEnergyController::ConnectedState;
        if (isConnected) {
            Q_ASSERT(desc.value.size() == 2);
            quint16 configValue = bt_get_le16(desc.value.constData());
            if (isNotificationEnabled(configValue) && hasNotifyProperty) {
                sendNotification(valueHandle);
            } else if (isIndicationEnabled(configValue) && hasIndicateProperty) {
                if (indicationInFlight)
                    scheduledIndications << valueHandle;
                else
                    sendIndication(valueHandle);
            }
        }

        // Prepare notification/indication of unconnected, bonded clients.
        for (auto it = clientConfigData.begin(); it != clientConfigData.end(); ++it) {
            if (isConnected && it.key() == remoteDevice.toUInt64())
                continue;
            QList<ClientConfigurationData> &configDataList = it.value();
            for (ClientConfigurationData &configData : configDataList) {
                if (configData.charValueHandle != valueHandle)
                    continue;
                if ((isNotificationEnabled(configData.configValue) && hasNotifyProperty)
                        || (isIndicationEnabled(configData.configValue) && hasIndicateProperty)) {
                    configData.charValueWasUpdated = true;
                    break;
                }
            }
        }
        break;
    }
}

void QLowEnergyControllerPrivateBluez::writeCharacteristicForCentral(const QSharedPointer<QLowEnergyServicePrivate> &service,
        QLowEnergyHandle charHandle,
        QLowEnergyHandle valueHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    QByteArray packet(WRITE_REQUEST_HEADER_SIZE + newValue.size(), Qt::Uninitialized);
    putBtData(valueHandle, packet.data() + 1);
    memcpy(packet.data() + 3, newValue.constData(), newValue.size());
    bool writeWithResponse = false;
    switch (mode) {
    case QLowEnergyService::WriteWithResponse:
        if (newValue.size() > (mtuSize - WRITE_REQUEST_HEADER_SIZE)) {
            sendNextPrepareWriteRequest(charHandle, newValue, 0);
            sendNextPendingRequest();
            return;
        }
        // write value fits into single package
        packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST);
        writeWithResponse = true;
        break;
    case QLowEnergyService::WriteWithoutResponse:
        packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_WRITE_COMMAND);
        break;
    case QLowEnergyService::WriteSigned:
        packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_SIGNED_WRITE_COMMAND);
        if (!isBonded()) {
            qCWarning(QT_BT_BLUEZ) << "signed write not possible: requires bond between devices";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        }
        if (securityLevel() >= BT_SECURITY_MEDIUM) {
            qCWarning(QT_BT_BLUEZ) << "signed write not possible: not allowed on encrypted link";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        }
        const auto signingDataIt = signingData.find(remoteDevice.toUInt64());
        if (signingDataIt == signingData.end()) {
            qCWarning(QT_BT_BLUEZ) << "signed write not possible: no signature key found";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        }
        ++signingDataIt.value().counter;
        packet = LeCmacCalculator::createFullMessage(packet, signingDataIt.value().counter);
        const quint64 mac = LeCmacCalculator().calculateMac(packet, signingDataIt.value().key);
        packet.resize(packet.size() + sizeof mac);
        putBtData(mac, packet.data() + packet.size() - sizeof mac);
        storeSignCounter(LocalSigningKey);
        break;
    }

    qCDebug(QT_BT_BLUEZ) << "Writing characteristic" << Qt::hex << charHandle
                         << "(size:" << packet.size() << "with response:"
                         << (mode == QLowEnergyService::WriteWithResponse)
                         << "signed:" << (mode == QLowEnergyService::WriteSigned) << ")";

    // Advantage of write without response is the quick turnaround.
    // It can be sent at any time and does not produce responses.
    // Therefore we will not put them into the openRequest queue at all.
    if (!writeWithResponse) {
        sendPacket(packet);
        return;
    }

    Request request;
    request.payload = packet;
    request.command = QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST;
    request.reference = charHandle;
    request.reference2 = newValue;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::writeDescriptorForPeripheral(
        const QSharedPointer<QLowEnergyServicePrivate> &service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descriptorHandle,
        const QByteArray &newValue)
{
    Q_ASSERT(descriptorHandle <= lastLocalHandle);
    Attribute &attribute = localAttributes[descriptorHandle];
    if (newValue.size() < attribute.minLength || newValue.size() > attribute.maxLength) {
        qCWarning(QT_BT_BLUEZ) << "invalid value of size" << newValue.size()
                               << "for attribute" << descriptorHandle;
        return;
    }
    attribute.value = newValue;
    service->characteristicList[charHandle].descriptorList[descriptorHandle].value = newValue;
}

void QLowEnergyControllerPrivateBluez::writeDescriptorForCentral(
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descriptorHandle,
        const QByteArray &newValue)
{
    if (newValue.size() > (mtuSize - WRITE_REQUEST_HEADER_SIZE)) {
        sendNextPrepareWriteRequest(descriptorHandle, newValue, 0);
        sendNextPendingRequest();
        return;
    }

    quint8 packet[WRITE_REQUEST_HEADER_SIZE];
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST);
    putBtData(descriptorHandle, &packet[1]);

    const qsizetype size = WRITE_REQUEST_HEADER_SIZE + newValue.size();
    QByteArray data(size, Qt::Uninitialized);
    memcpy(data.data(), packet, WRITE_REQUEST_HEADER_SIZE);
    memcpy(&(data.data()[WRITE_REQUEST_HEADER_SIZE]), newValue.constData(), newValue.size());

    qCDebug(QT_BT_BLUEZ) << "Writing descriptor" << Qt::hex << descriptorHandle
                         << "(size:" << size << ")";

    Request request;
    request.payload = data;
    request.command = QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST;
    request.reference = (charHandle | (descriptorHandle << 16));
    request.reference2 = newValue;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerPrivateBluez::handleWriteRequestOrCommand(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.5.1-3

    const bool isRequest = static_cast<QBluezConst::AttCommand>(packet.at(0))
            == QBluezConst::AttCommand::ATT_OP_WRITE_REQUEST;
    const bool isSigned = static_cast<QBluezConst::AttCommand>(packet.at(0))
            == QBluezConst::AttCommand::ATT_OP_SIGNED_WRITE_COMMAND;
    if (!checkPacketSize(packet, isSigned ? 15 : 3, mtuSize))
        return;
    const QLowEnergyHandle handle = bt_get_le16(packet.constData() + 1);
    qCDebug(QT_BT_BLUEZ) << "client sends" << (isSigned ? "signed" : "") << "write"
                         << (isRequest ? "request" : "command") << "for handle" << handle;

    if (!checkHandle(packet, handle))
        return;

    Attribute &attribute = localAttributes[handle];
    const QLowEnergyCharacteristic::PropertyType type = isRequest
            ? QLowEnergyCharacteristic::Write : isSigned
              ? QLowEnergyCharacteristic::WriteSigned : QLowEnergyCharacteristic::WriteNoResponse;
    const QBluezConst::AttError permissionsError = checkPermissions(attribute, type);
    if (permissionsError != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          permissionsError);
        return;
    }

    int valueLength;
    if (isSigned) {
        if (!isBonded()) {
            qCWarning(QT_BT_BLUEZ) << "Ignoring signed write from non-bonded device.";
            return;
        }
        if (securityLevel() >= BT_SECURITY_MEDIUM) {
            qCWarning(QT_BT_BLUEZ) << "Ignoring signed write on encrypted link.";
            return;
        }
        const auto signingDataIt = signingData.find(remoteDevice.toUInt64());
        if (signingDataIt == signingData.constEnd()) {
            qCWarning(QT_BT_BLUEZ) << "No CSRK found for peer device, ignoring signed write";
            return;
        }

        const quint32 signCounter = getBtData<quint32>(packet.data() + packet.size() - 12);
        if (signCounter < signingDataIt.value().counter + 1) {
            qCWarning(QT_BT_BLUEZ) << "Client's' sign counter" << signCounter
                                   << "not greater than local sign counter"
                                   << signingDataIt.value().counter
                                   << "; ignoring signed write command.";
            return;
        }

        const quint64 macFromClient = getBtData<quint64>(packet.data() + packet.size() - 8);
        const bool signatureCorrect = verifyMac(packet.left(packet.size() - 12),
                signingDataIt.value().key, signCounter, macFromClient);
        if (!signatureCorrect) {
            qCWarning(QT_BT_BLUEZ) << "Signed Write packet has wrong signature, disconnecting";
            disconnectFromDevice(); // Recommended by spec v4.2, Vol 3, part C, 10.4.2
            return;
        }

        signingDataIt.value().counter = signCounter;
        storeSignCounter(RemoteSigningKey);
        valueLength = packet.size() - 15;
    } else {
        valueLength = packet.size() - 3;
    }

    if (valueLength > attribute.maxLength) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          QBluezConst::AttError::ATT_ERROR_INVAL_ATTR_VALUE_LEN);
        return;
    }

    // If the attribute value has a fixed size and the value in the packet is shorter,
    // then we overwrite only the start of the attribute value and keep the rest.
    QByteArray value = packet.mid(3, valueLength);
    if (attribute.minLength == attribute.maxLength && valueLength < attribute.minLength)
        value += attribute.value.mid(valueLength, attribute.maxLength - valueLength);

    QLowEnergyCharacteristic characteristic;
    QLowEnergyDescriptor descriptor;
    updateLocalAttributeValue(handle, value, characteristic, descriptor);

    if (isRequest) {
        const QByteArray response =
                QByteArray(1, static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_WRITE_RESPONSE));
        sendPacket(response);
    }

    if (characteristic.isValid()) {
        emit characteristic.d_ptr->characteristicChanged(characteristic, value);
    } else {
        Q_ASSERT(descriptor.isValid());
        emit descriptor.d_ptr->descriptorWritten(descriptor, value);
    }
}

void QLowEnergyControllerPrivateBluez::handlePrepareWriteRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.6.1

    if (!checkPacketSize(packet, 5, mtuSize))
        return;
    const quint16 handle = bt_get_le16(packet.constData() + 1);
    qCDebug(QT_BT_BLUEZ) << "client sends prepare write request for handle" << handle;

    if (!checkHandle(packet, handle))
        return;
    const Attribute &attribute = localAttributes.at(handle);
    const QBluezConst::AttError permissionsError =
            checkPermissions(attribute, QLowEnergyCharacteristic::Write);
    if (permissionsError != QBluezConst::AttError::ATT_ERROR_NO_ERROR) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          permissionsError);
        return;
    }
    if (openPrepareWriteRequests.size() >= maxPrepareQueueSize) {
        sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)), handle,
                          QBluezConst::AttError::ATT_ERROR_PREPARE_QUEUE_FULL);
        return;
    }

    // The value is not checked here, but on the Execute request.
    openPrepareWriteRequests << WriteRequest(handle, bt_get_le16(packet.constData() + 3),
                                                    packet.mid(5));

    QByteArray response = packet;
    response[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_PREPARE_WRITE_RESPONSE);
    sendPacket(response);
}

void QLowEnergyControllerPrivateBluez::handleExecuteWriteRequest(const QByteArray &packet)
{
    // Spec v4.2, Vol 3, Part F, 3.4.6.3

    if (!checkPacketSize(packet, 2))
        return;
    const bool cancel = packet.at(1) == 0;
    qCDebug(QT_BT_BLUEZ) << "client sends execute write request; flag is"
                         << (cancel ? "cancel" : "flush");

    QList<WriteRequest> requests = openPrepareWriteRequests;
    openPrepareWriteRequests.clear();
    QList<QLowEnergyCharacteristic> characteristics;
    QList<QLowEnergyDescriptor> descriptors;
    if (!cancel) {
        for (const WriteRequest &request : std::as_const(requests)) {
            Attribute &attribute = localAttributes[request.handle];
            if (request.valueOffset > attribute.value.size()) {
                sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)),
                                  request.handle, QBluezConst::AttError::ATT_ERROR_INVALID_OFFSET);
                return;
            }
            const QByteArray newValue = attribute.value.left(request.valueOffset) + request.value;
            if (newValue.size() > attribute.maxLength) {
                sendErrorResponse(static_cast<QBluezConst::AttCommand>(packet.at(0)),
                                  request.handle,
                                  QBluezConst::AttError::ATT_ERROR_INVAL_ATTR_VALUE_LEN);
                return;
            }
            QLowEnergyCharacteristic characteristic;
            QLowEnergyDescriptor descriptor;
            // TODO: Redundant attribute lookup for the case of the same handle appearing
            //       more than once.
            updateLocalAttributeValue(request.handle, newValue, characteristic, descriptor);
            if (characteristic.isValid()) {
                characteristics << characteristic;
            } else if (descriptor.isValid()) {
                Q_ASSERT(descriptor.isValid());
                descriptors << descriptor;
            }
        }
    }

    sendPacket(QByteArray(
            1, static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_EXECUTE_WRITE_RESPONSE)));

    for (const QLowEnergyCharacteristic &characteristic : std::as_const(characteristics))
        emit characteristic.d_ptr->characteristicChanged(characteristic, characteristic.value());
    for (const QLowEnergyDescriptor &descriptor : std::as_const(descriptors))
        emit descriptor.d_ptr->descriptorWritten(descriptor, descriptor.value());
}

void QLowEnergyControllerPrivateBluez::sendErrorResponse(QBluezConst::AttCommand request,
                                                         quint16 handle, QBluezConst::AttError code)
{
    // An ATT command never receives an error response.
    if (request == QBluezConst::AttCommand::ATT_OP_WRITE_COMMAND
        || request == QBluezConst::AttCommand::ATT_OP_SIGNED_WRITE_COMMAND)
        return;

    QByteArray packet(ERROR_RESPONSE_HEADER_SIZE, Qt::Uninitialized);
    packet[0] = static_cast<quint8>(QBluezConst::AttCommand::ATT_OP_ERROR_RESPONSE);
    packet[1] = static_cast<quint8>(request);
    putBtData(handle, packet.data() + 2);
    packet[4] = static_cast<quint8>(code);
    qCWarning(QT_BT_BLUEZ) << "sending error response; request:"
                           << request << "handle:" << handle
                           << "code:" << code;
    sendPacket(packet);
}

void QLowEnergyControllerPrivateBluez::sendListResponse(const QByteArray &packetStart,
                                                        qsizetype elemSize,
                                                        const QList<Attribute> &attributes,
                                                        const ElemWriter &elemWriter)
{
    const qsizetype offset = packetStart.size();
    const qsizetype elemCount = (std::min)(attributes.size(), (mtuSize - offset) / elemSize);
    const qsizetype totalPacketSize = offset + elemCount * elemSize;
    QByteArray response(totalPacketSize, Qt::Uninitialized);
    using namespace std;
    memcpy(response.data(), packetStart.constData(), offset);
    char *data = response.data() + offset;
    for_each(attributes.constBegin(), attributes.constBegin() + elemCount,
             [&data, elemWriter](const Attribute &attr) { elemWriter(attr, data); });
    qCDebug(QT_BT_BLUEZ) << "sending response:" << response.toHex();
    sendPacket(response);
}

void QLowEnergyControllerPrivateBluez::sendNotification(QLowEnergyHandle handle)
{
    sendNotificationOrIndication(QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_NOTIFICATION, handle);
}

void QLowEnergyControllerPrivateBluez::sendIndication(QLowEnergyHandle handle)
{
    Q_ASSERT(!indicationInFlight);
    indicationInFlight = true;
    sendNotificationOrIndication(QBluezConst::AttCommand::ATT_OP_HANDLE_VAL_INDICATION, handle);
}

void QLowEnergyControllerPrivateBluez::sendNotificationOrIndication(QBluezConst::AttCommand opCode,
                                                                    QLowEnergyHandle handle)
{
    Q_ASSERT(handle <= lastLocalHandle);
    const Attribute &attribute = localAttributes.at(handle);
    const qsizetype maxValueLength = (std::min)(attribute.value.size(), qsizetype(mtuSize) - 3);
    QByteArray packet(3 + maxValueLength, Qt::Uninitialized);
    packet[0] = static_cast<quint8>(opCode);
    putBtData(handle, packet.data() + 1);
    using namespace std;
    memcpy(packet.data() + 3, attribute.value.constData(), maxValueLength);
    qCDebug(QT_BT_BLUEZ) << "sending notification/indication:" << packet.toHex();
    sendPacket(packet);
}

void QLowEnergyControllerPrivateBluez::sendNextIndication()
{
    if (!scheduledIndications.isEmpty())
        sendIndication(scheduledIndications.takeFirst());
}

static QString nameOfRemoteCentral(const QBluetoothAddress &peerAddress)
{
    const QString peerAddressString = peerAddress.toString();
    initializeBluez5();
    OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus());
    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Device1")) {
                if (ifaceValues.value(QStringLiteral("Address")).toString() == peerAddressString)
                    return ifaceValues.value(QStringLiteral("Alias")).toString();
            }
        }
    }
    return QString();
}

void QLowEnergyControllerPrivateBluez::handleConnectionRequest()
{
    if (state != QLowEnergyController::AdvertisingState) {
        qCWarning(QT_BT_BLUEZ) << "Incoming connection request in unexpected state" << state;
        return;
    }
    Q_ASSERT(serverSocketNotifier);
    serverSocketNotifier->setEnabled(false);
    sockaddr_l2 clientAddr;
    socklen_t clientAddrSize = sizeof clientAddr;
    const int clientSocket = accept(serverSocketNotifier->socket(),
                                    reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
    if (clientSocket == -1) {
        // Not fatal in itself. The next one might succeed.
        qCWarning(QT_BT_BLUEZ) << "accept() failed:" << qt_error_string(errno);
        serverSocketNotifier->setEnabled(true);
        return;
    }

    remoteDevice = QBluetoothAddress(convertAddress(clientAddr.l2_bdaddr.b));
    remoteName = nameOfRemoteCentral(remoteDevice);
    qCDebug(QT_BT_BLUEZ) << "GATT connection from device" << remoteDevice << remoteName;

    if (connectionHandle == 0)
        qCWarning(QT_BT_BLUEZ) << "Received client connection, but no connection complete event";

    if (l2cpSocket) {
        disconnect(l2cpSocket);
        if (l2cpSocket->isOpen())
            l2cpSocket->close();

        l2cpSocket->deleteLater();
        l2cpSocket = nullptr;
    }
    closeServerSocket();

    QBluetoothSocketPrivateBluez *rawSocketPrivate = new QBluetoothSocketPrivateBluez();
    l2cpSocket = new QBluetoothSocket(
                rawSocketPrivate, QBluetoothServiceInfo::L2capProtocol, this);
    connect(l2cpSocket, &QBluetoothSocket::disconnected,
            this, &QLowEnergyControllerPrivateBluez::l2cpDisconnected);
    connect(l2cpSocket, &QBluetoothSocket::errorOccurred, this,
            &QLowEnergyControllerPrivateBluez::l2cpErrorChanged);
    connect(l2cpSocket, &QIODevice::readyRead, this, &QLowEnergyControllerPrivateBluez::l2cpReadyRead);
    l2cpSocket->d_ptr->lowEnergySocketType = addressType == QLowEnergyController::PublicAddress
            ? BDADDR_LE_PUBLIC : BDADDR_LE_RANDOM;
    l2cpSocket->setSocketDescriptor(clientSocket, QBluetoothServiceInfo::L2capProtocol,
            QBluetoothSocket::SocketState::ConnectedState, QIODevice::ReadWrite | QIODevice::Unbuffered);
    restoreClientConfigurations();
    loadSigningDataIfNecessary(RemoteSigningKey);

    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::ConnectedState);
    emit q->connected();
}

void QLowEnergyControllerPrivateBluez::closeServerSocket()
{
    if (!serverSocketNotifier)
        return;
    serverSocketNotifier->disconnect();
    close(serverSocketNotifier->socket());
    serverSocketNotifier->deleteLater();
    serverSocketNotifier = nullptr;
}

bool QLowEnergyControllerPrivateBluez::isBonded() const
{
    // Pairing does not necessarily imply bonding, but we don't know whether the
    // bonding flag was set in the original pairing request.
    return QBluetoothLocalDevice(localAdapter).pairingStatus(remoteDevice)
            != QBluetoothLocalDevice::Unpaired;
}

QList<QLowEnergyControllerPrivateBluez::TempClientConfigurationData>
QLowEnergyControllerPrivateBluez::gatherClientConfigData()
{
    QList<TempClientConfigurationData> data;
    for (const auto &service : std::as_const(localServices)) {
        for (auto charIt = service->characteristicList.begin();
             charIt != service->characteristicList.end(); ++charIt) {
            QLowEnergyServicePrivate::CharData &charData = charIt.value();
            for (auto descIt = charData.descriptorList.begin();
                 descIt != charData.descriptorList.end(); ++descIt) {
                QLowEnergyServicePrivate::DescData &descData = descIt.value();
                if (descData.uuid == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration) {
                    data << TempClientConfigurationData(&descData, charData.valueHandle,
                                                        descIt.key());
                    break;
                }
            }
        }
    }
    return data;
}

void QLowEnergyControllerPrivateBluez::storeClientConfigurations()
{
    if (!isBonded()) {
        clientConfigData.remove(remoteDevice.toUInt64());
        return;
    }
    QList<ClientConfigurationData> clientConfigs;
    const QList<TempClientConfigurationData> &tempConfigList = gatherClientConfigData();
    for (const auto &tempConfigData : tempConfigList) {
        Q_ASSERT(tempConfigData.descData->value.size() == 2);
        const quint16 value = bt_get_le16(tempConfigData.descData->value.constData());
        if (value != 0) {
            clientConfigs << ClientConfigurationData(tempConfigData.charValueHandle,
                                                     tempConfigData.configHandle, value);
        }
    }
    clientConfigData.insert(remoteDevice.toUInt64(), clientConfigs);
}

void QLowEnergyControllerPrivateBluez::restoreClientConfigurations()
{
    const QList<TempClientConfigurationData> &tempConfigList = gatherClientConfigData();
    const QList<ClientConfigurationData> &restoredClientConfigs = isBonded()
            ? clientConfigData.value(remoteDevice.toUInt64())
            : QList<ClientConfigurationData>();
    QList<QLowEnergyHandle> notifications;
    for (const auto &tempConfigData : tempConfigList) {
        bool wasRestored = false;
        for (const auto &restoredData : restoredClientConfigs) {
            if (restoredData.charValueHandle == tempConfigData.charValueHandle) {
                Q_ASSERT(tempConfigData.descData->value.size() == 2);
                putBtData(restoredData.configValue, tempConfigData.descData->value.data());
                wasRestored = true;
                if (restoredData.charValueWasUpdated) {
                    if (isNotificationEnabled(restoredData.configValue))
                        notifications << restoredData.charValueHandle;
                    else if (isIndicationEnabled(restoredData.configValue))
                        scheduledIndications << restoredData.charValueHandle;
                }
                break;
            }
        }
        if (!wasRestored)
            tempConfigData.descData->value = QByteArray(2, 0); // Default value.
        Q_ASSERT(lastLocalHandle >= tempConfigData.configHandle);
        Q_ASSERT(tempConfigData.configHandle > tempConfigData.charValueHandle);
        localAttributes[tempConfigData.configHandle].value = tempConfigData.descData->value;
    }

    for (const QLowEnergyHandle handle : std::as_const(notifications))
        sendNotification(handle);
    sendNextIndication();
}

void QLowEnergyControllerPrivateBluez::loadSigningDataIfNecessary(SigningKeyType keyType)
{
    const auto signingDataIt = signingData.constFind(remoteDevice.toUInt64());
    if (signingDataIt != signingData.constEnd())
        return; // We are up to date for this device.
    const QString settingsFilePath = keySettingsFilePath();
    if (!QFileInfo(settingsFilePath).exists()) {
        qCDebug(QT_BT_BLUEZ) << "No settings found for peer device.";
        return;
    }
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    const QString group = signingKeySettingsGroup(keyType);
    settings.beginGroup(group);
    const QByteArray keyString = settings.value(QLatin1String("Key")).toByteArray();
    if (keyString.isEmpty()) {
        qCDebug(QT_BT_BLUEZ) << "Group" << group << "not found in settings file";
        return;
    }
    const QByteArray keyData = QByteArray::fromHex(keyString);
    if (keyData.size() != qsizetype(sizeof(BluezUint128))) {
        qCWarning(QT_BT_BLUEZ) << "Signing key in settings file has invalid size"
                               << keyString.size();
        return;
    }
    const quint32 counter = settings.value(QLatin1String("Counter"), 0).toUInt();
    using namespace std;
    BluezUint128 csrk;
    memcpy(csrk.data, keyData.constData(), keyData.size());
    signingData.insert(remoteDevice.toUInt64(), SigningData(csrk, counter - 1));
}

void QLowEnergyControllerPrivateBluez::storeSignCounter(SigningKeyType keyType) const
{
    const auto signingDataIt = signingData.constFind(remoteDevice.toUInt64());
    if (signingDataIt == signingData.constEnd())
        return;
    const QString settingsFilePath = keySettingsFilePath();
    if (!QFileInfo(settingsFilePath).exists())
        return;
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    if (!settings.isWritable())
        return;
    settings.beginGroup(signingKeySettingsGroup(keyType));
    const QString counterKey = QLatin1String("Counter");
    if (!settings.allKeys().contains(counterKey))
        return;
    const quint32 counterValue = signingDataIt.value().counter + 1;
    if (counterValue == settings.value(counterKey).toUInt())
        return;
    settings.setValue(counterKey, counterValue);
}

QString QLowEnergyControllerPrivateBluez::signingKeySettingsGroup(SigningKeyType keyType) const
{
    return QLatin1String(keyType == LocalSigningKey ? "LocalSignatureKey" : "RemoteSignatureKey");
}

QString QLowEnergyControllerPrivateBluez::keySettingsFilePath() const
{
    return QString::fromLatin1("/var/lib/bluetooth/%1/%2/info")
            .arg(localAdapter.toString(), remoteDevice.toString());
}

static QByteArray uuidToByteArray(const QBluetoothUuid &uuid)
{
    QByteArray ba(sizeof(uuid), Qt::Uninitialized);
    char *ptr = ba.data();
    putDataAndIncrement(uuid, ptr);
    ba.resize(ptr - ba.constData());
    return ba;
}

void QLowEnergyControllerPrivateBluez::addToGenericAttributeList(const QLowEnergyServiceData &service,
                                                            QLowEnergyHandle startHandle)
{
    // Construct generic attribute data for the service with handles as keys.
    // Otherwise a number of request handling functions will be awkward to write
    // as well as computationally inefficient.

    localAttributes.resize(lastLocalHandle + 1);
    Attribute serviceAttribute;
    serviceAttribute.handle = startHandle;
    serviceAttribute.type = QBluetoothUuid(static_cast<quint16>(service.type()));
    serviceAttribute.properties = QLowEnergyCharacteristic::Read;
    serviceAttribute.value = uuidToByteArray(service.uuid());
    QLowEnergyHandle currentHandle = startHandle;
    const QList<QLowEnergyService *> includedServices = service.includedServices();
    for (const QLowEnergyService * const service : includedServices) {
        Attribute attribute;
        attribute.handle = ++currentHandle;
        attribute.type = QBluetoothUuid(GATT_INCLUDED_SERVICE);
        attribute.properties = QLowEnergyCharacteristic::Read;
        const bool includeUuidInValue = service->serviceUuid().minimumSize() == 2;
        attribute.value.resize((2 + includeUuidInValue) * sizeof(QLowEnergyHandle));
        char *valueData = attribute.value.data();
        putDataAndIncrement(service->d_ptr->startHandle, valueData);
        putDataAndIncrement(service->d_ptr->endHandle, valueData);
        if (includeUuidInValue)
            putDataAndIncrement(service->serviceUuid(), valueData);
        localAttributes[attribute.handle] = attribute;
    }
    const QList<QLowEnergyCharacteristicData> characteristics = service.characteristics();
    for (const QLowEnergyCharacteristicData &cd : characteristics) {
        Attribute attribute;

        // Characteristic declaration;
        attribute.handle = ++currentHandle;
        attribute.groupEndHandle = attribute.handle + 1 + cd.descriptors().size();
        attribute.type = QBluetoothUuid(GATT_CHARACTERISTIC);
        attribute.properties = QLowEnergyCharacteristic::Read;
        attribute.value.resize(1 + sizeof(QLowEnergyHandle) + cd.uuid().minimumSize());
        char *valueData = attribute.value.data();
        putDataAndIncrement(static_cast<quint8>(cd.properties()), valueData);
        putDataAndIncrement(QLowEnergyHandle(currentHandle + 1), valueData);
        putDataAndIncrement(cd.uuid(), valueData);
        localAttributes[attribute.handle] = attribute;

        // Characteristic value declaration.
        attribute.handle = ++currentHandle;
        attribute.groupEndHandle = attribute.handle;
        attribute.type = cd.uuid();
        attribute.properties = cd.properties();
        attribute.readConstraints = cd.readConstraints();
        attribute.writeConstraints = cd.writeConstraints();
        attribute.value = cd.value();
        attribute.minLength = cd.minimumValueLength();
        attribute.maxLength = cd.maximumValueLength();
        localAttributes[attribute.handle] = attribute;

        const QList<QLowEnergyDescriptorData> descriptors = cd.descriptors();
        for (const QLowEnergyDescriptorData &dd : descriptors) {
            attribute.handle = ++currentHandle;
            attribute.groupEndHandle = attribute.handle;
            attribute.type = dd.uuid();
            attribute.properties = QLowEnergyCharacteristic::PropertyTypes();
            attribute.readConstraints = AttAccessConstraints();
            attribute.writeConstraints = AttAccessConstraints();
            attribute.minLength = 0;
            attribute.maxLength = INT_MAX;

            // Spec v4.2, Vol. 3, Part G, 3.3.3.x
            if (attribute.type == QBluetoothUuid::DescriptorType::CharacteristicExtendedProperties) {
                attribute.properties = QLowEnergyCharacteristic::Read;
                attribute.minLength = attribute.maxLength = 2;
            } else if (attribute.type == QBluetoothUuid::DescriptorType::CharacteristicPresentationFormat) {
                attribute.properties = QLowEnergyCharacteristic::Read;
                attribute.minLength = attribute.maxLength = 7;
            } else if (attribute.type == QBluetoothUuid::DescriptorType::CharacteristicAggregateFormat) {
                attribute.properties = QLowEnergyCharacteristic::Read;
                attribute.minLength = 4;
            } else if (attribute.type == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration
                       || attribute.type == QBluetoothUuid::DescriptorType::ServerCharacteristicConfiguration) {
                attribute.properties = QLowEnergyCharacteristic::Read
                        | QLowEnergyCharacteristic::Write
                        | QLowEnergyCharacteristic::WriteNoResponse
                        | QLowEnergyCharacteristic::WriteSigned;
                attribute.writeConstraints = dd.writeConstraints();
                attribute.minLength = attribute.maxLength = 2;
            } else {
                if (dd.isReadable())
                    attribute.properties |= QLowEnergyCharacteristic::Read;
                if (dd.isWritable()) {
                    attribute.properties |= QLowEnergyCharacteristic::Write;
                    attribute.properties |= QLowEnergyCharacteristic::WriteNoResponse;
                    attribute.properties |= QLowEnergyCharacteristic::WriteSigned;
                }
                attribute.readConstraints = dd.readConstraints();
                attribute.writeConstraints = dd.writeConstraints();
            }

            attribute.value = dd.value();
            if (attribute.value.size() < attribute.minLength
                    || attribute.value.size() > attribute.maxLength) {
                qCWarning(QT_BT_BLUEZ) << "attribute of type" << attribute.type
                                       << "has invalid length of" << attribute.value.size()
                                       << "bytes";
                attribute.value = QByteArray(attribute.minLength, 0);
            }
            localAttributes[attribute.handle] = attribute;
        }
    }
    serviceAttribute.groupEndHandle = currentHandle;
    localAttributes[serviceAttribute.handle] = serviceAttribute;
}

int QLowEnergyControllerPrivateBluez::mtu() const
{
    return mtuSize;
}

void QLowEnergyControllerPrivateBluez::ensureUniformAttributes(
        QList<Attribute> &attributes, const std::function<int(const Attribute &)> &getSize)
{
    if (attributes.isEmpty())
        return;
    const int firstSize = getSize(attributes.first());
    const auto it = std::find_if(attributes.begin() + 1, attributes.end(),
            [firstSize, getSize](const Attribute &attr) { return getSize(attr) != firstSize; });
    if (it != attributes.end())
        attributes.erase(it, attributes.end());

}

void QLowEnergyControllerPrivateBluez::ensureUniformUuidSizes(QList<Attribute> &attributes)
{
    ensureUniformAttributes(attributes,
                            [](const Attribute &attr) { return getUuidSize(attr.type); });
}

void QLowEnergyControllerPrivateBluez::ensureUniformValueSizes(QList<Attribute> &attributes)
{
    ensureUniformAttributes(attributes,
                            [](const Attribute &attr) { return attr.value.size(); });
}

QList<QLowEnergyControllerPrivateBluez::Attribute>
QLowEnergyControllerPrivateBluez::getAttributes(QLowEnergyHandle startHandle,
                                                QLowEnergyHandle endHandle,
                                                const AttributePredicate &attributePredicate)
{
    QList<Attribute> results;
    if (startHandle > lastLocalHandle)
        return results;
    if (lastLocalHandle == 0) // We have no services at all.
        return results;
    Q_ASSERT(startHandle <= endHandle); // Must have been checked before.
    const QLowEnergyHandle firstHandle = qMin(startHandle, lastLocalHandle);
    const QLowEnergyHandle lastHandle = qMin(endHandle, lastLocalHandle);
    for (QLowEnergyHandle i = firstHandle; i <= lastHandle; ++i) {
        const Attribute &attr = localAttributes.at(i);
        if (attributePredicate(attr))
            results << attr;
    }
    return results;
}

QBluezConst::AttError
QLowEnergyControllerPrivateBluez::checkPermissions(const Attribute &attr,
                                                   QLowEnergyCharacteristic::PropertyType type)
{
    const bool isReadAccess = type == QLowEnergyCharacteristic::Read;
    const bool isWriteCommand = type == QLowEnergyCharacteristic::WriteNoResponse;
    const bool isWriteAccess = type == QLowEnergyCharacteristic::Write
            || type == QLowEnergyCharacteristic::WriteSigned
            || isWriteCommand;
    Q_ASSERT(isReadAccess || isWriteAccess);
    if (!(attr.properties & type)) {
        if (isReadAccess)
            return QBluezConst::AttError::ATT_ERROR_READ_NOT_PERM;

        // The spec says: If an attribute requires a signed write, then a non-signed write command
        // can also be used if the link is encrypted.
        const bool unsignedWriteOk = isWriteCommand
                && (attr.properties & QLowEnergyCharacteristic::WriteSigned)
                && securityLevel() >= BT_SECURITY_MEDIUM;
        if (!unsignedWriteOk)
            return QBluezConst::AttError::ATT_ERROR_WRITE_NOT_PERM;
    }
    const AttAccessConstraints constraints = isReadAccess
            ? attr.readConstraints : attr.writeConstraints;
    if (constraints.testFlag(AttAccessConstraint::AttAuthorizationRequired))
        return QBluezConst::AttError::ATT_ERROR_INSUF_AUTHORIZATION; // TODO: emit signal (and offer
                                                                     // authorization function)?
    if (constraints.testFlag(AttAccessConstraint::AttEncryptionRequired)
        && securityLevel() < BT_SECURITY_MEDIUM)
        return QBluezConst::AttError::ATT_ERROR_INSUF_ENCRYPTION;
    if (constraints.testFlag(AttAccessConstraint::AttAuthenticationRequired)
        && securityLevel() < BT_SECURITY_HIGH)
        return QBluezConst::AttError::ATT_ERROR_INSUF_AUTHENTICATION;
    if (false)
        return QBluezConst::AttError::ATT_ERROR_INSUF_ENCR_KEY_SIZE;
    return QBluezConst::AttError::ATT_ERROR_NO_ERROR;
}

QBluezConst::AttError QLowEnergyControllerPrivateBluez::checkReadPermissions(const Attribute &attr)
{
    return checkPermissions(attr, QLowEnergyCharacteristic::Read);
}

QBluezConst::AttError
QLowEnergyControllerPrivateBluez::checkReadPermissions(QList<Attribute> &attributes)
{
    if (attributes.isEmpty())
        return QBluezConst::AttError::ATT_ERROR_NO_ERROR;

    // The logic prescribed in the spec is as follows:
    //    1) If the first in a list of matching attributes has a permissions error,
    //       then that error is returned via an error response.
    //    2) If any other element of that list would cause a permissions error, then all
    //       attributes from this one on are not part of the result set, but no error is returned.
    const QBluezConst::AttError error = checkReadPermissions(attributes.first());
    if (error != QBluezConst::AttError::ATT_ERROR_NO_ERROR)
        return error;
    const auto it =
            std::find_if(attributes.begin() + 1, attributes.end(), [this](const Attribute &attr) {
                return checkReadPermissions(attr) != QBluezConst::AttError::ATT_ERROR_NO_ERROR;
            });
    if (it != attributes.end())
        attributes.erase(it, attributes.end());
    return QBluezConst::AttError::ATT_ERROR_NO_ERROR;
}

bool QLowEnergyControllerPrivateBluez::verifyMac(const QByteArray &message, BluezUint128 csrk,
                                             quint32 signCounter, quint64 expectedMac)
{
    if (!cmacCalculator)
        cmacCalculator = new LeCmacCalculator;
    return cmacCalculator->verify(LeCmacCalculator::createFullMessage(message, signCounter), csrk,
                                expectedMac);
}

QT_END_NAMESPACE

#include "moc_qlowenergycontroller_bluez_p.cpp"
