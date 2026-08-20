// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "systeccanbackend.h"
#include "systeccanbackend_p.h"
#include "systeccan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_SYSTECCAN)

Q_GLOBAL_STATIC(QLibrary, systecLibrary)

bool SystecCanBackend::canCreate(QString *errorReason)
{
    static bool symbolsResolved = resolveSystecCanSymbols(systecLibrary());
    if (Q_UNLIKELY(!symbolsResolved)) {
        *errorReason = systecLibrary()->errorString();
        return false;
    }
    return true;
}

QCanBusDeviceInfo SystecCanBackend::createDeviceInfo(const QString &serialNumber,
                                                     const QString &description,
                                                     uint deviceNumber,
                                                     int channelNumber)
{
    const QString name = QString::fromLatin1("can%1.%2").arg(deviceNumber).arg(channelNumber);
    return QCanBusDevice::createDeviceInfo(QStringLiteral("systeccan"), name,
                                           serialNumber, description,
                                           QString(), channelNumber, false, false);
}

static QString descriptionString(uint productCode)
{
    switch (productCode & USBCAN_PRODCODE_MASK_PID) {
    case USBCAN_PRODCODE_PID_GW001:       return QStringLiteral("USB-CANmodul (G1)");
    case USBCAN_PRODCODE_PID_GW002:       return QStringLiteral("USB-CANmodul (G2)");
    case USBCAN_PRODCODE_PID_MULTIPORT:   return QStringLiteral("Multiport CAN-to-USB (G3)");
    case USBCAN_PRODCODE_PID_BASIC:       return QStringLiteral("USB-CANmodul1 (G3)");
    case USBCAN_PRODCODE_PID_ADVANCED:    return QStringLiteral("USB-CANmodul2 (G3)");
    case USBCAN_PRODCODE_PID_USBCAN8:     return QStringLiteral("USB-CANmodul8 (G3)");
    case USBCAN_PRODCODE_PID_USBCAN16:    return QStringLiteral("USB-CANmodul16 (G3)");
    case USBCAN_PRODCODE_PID_ADVANCED_G4: return QStringLiteral("USB-CANmodul2 (G4)");
    case USBCAN_PRODCODE_PID_BASIC_G4:    return QStringLiteral("USB-CANmodul1 (G4)");
    default:                              return QStringLiteral("Unknown");
    }
}

static void DRV_CALLBACK_TYPE ucanEnumCallback(DWORD index, BOOL isUsed,
                                               tUcanHardwareInfoEx *hardwareInfo,
                                               tUcanHardwareInitInfo *initInfo,
                                               void *args)
{
    auto result = static_cast<QList<QCanBusDeviceInfo> *>(args);

    Q_UNUSED(index);
    Q_UNUSED(isUsed);

    const QString serialNumber = QString::number(hardwareInfo->m_dwSerialNr);
    const QString description = descriptionString(hardwareInfo->m_dwProductCode);
    result->append(SystecCanBackend::createDeviceInfo(serialNumber, description,
                                                      hardwareInfo->m_bDeviceNr, 0));
    if (USBCAN_CHECK_SUPPORT_TWO_CHANNEL(hardwareInfo)) {
        result->append(SystecCanBackend::createDeviceInfo(serialNumber, description,
                                                          hardwareInfo->m_bDeviceNr, 1));
    }

    initInfo->m_fTryNext = true; // continue enumerating with next device
}

QList<QCanBusDeviceInfo> SystecCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> result;

    ::UcanEnumerateHardware(&ucanEnumCallback, &result, false, 0, quint8(~0), 0, quint32(~0), 0, quint32(~0));

    return result;
}

class OutgoingEventNotifier : public QTimer
{
public:
    OutgoingEventNotifier(SystecCanBackendPrivate *d, QObject *parent) :
        QTimer(parent),
        dptr(d)
    {
    }

protected:
    void timerEvent(QTimerEvent *e) override
    {
        if (e->timerId() == timerId()) {
            dptr->startWrite();
            return;
        }
        QTimer::timerEvent(e);
    }

private:
    SystecCanBackendPrivate * const dptr;
};

SystecCanBackendPrivate::SystecCanBackendPrivate(SystecCanBackend *q) :
    q_ptr(q),
    incomingEventHandler(new IncomingEventHandler(this, q))
{
}

static uint bitrateCodeFromBitrate(int bitrate)
{
    struct BitrateItem {
        int bitrate;
        uint code;
    } bitrateTable[] = {
        {   10000, USBCAN_BAUDEX_10kBit  },
        {   20000, USBCAN_BAUDEX_20kBit  },
        {   50000, USBCAN_BAUDEX_50kBit  },
        {  100000, USBCAN_BAUDEX_100kBit },
        {  125000, USBCAN_BAUDEX_125kBit },
        {  250000, USBCAN_BAUDEX_250kBit },
        {  500000, USBCAN_BAUDEX_500kBit },
        {  800000, USBCAN_BAUDEX_800kBit },
        { 1000000, USBCAN_BAUDEX_1MBit   }
    };

    const int entries = (sizeof(bitrateTable) / sizeof(*bitrateTable));
    for (int i = 0; i < entries; ++i)
        if (bitrateTable[i].bitrate == bitrate)
            return bitrateTable[i].code;

    return 0;
}

void IncomingEventHandler::customEvent(QEvent *event)
{
    dptr->eventHandler(event);
}

/*
 * Do not call functions of USBCAN32.DLL directly from this callback handler.
 * Use events or windows messages to notify the event to the application.
 */
static void DRV_CALLBACK_TYPE ucanCallback(tUcanHandle, quint32 event, quint8, void *args)
{
    QEvent::Type type = static_cast<QEvent::Type>(QEvent::User + event);
    IncomingEventHandler *handler = static_cast<IncomingEventHandler *>(args);
    qApp->postEvent(handler, new QEvent(type));
}

bool SystecCanBackendPrivate::open()
{
    Q_Q(SystecCanBackend);

    const UCANRET initHardwareRes = ::UcanInitHardwareEx(&handle, device, ucanCallback, incomingEventHandler);
    if (Q_UNLIKELY(initHardwareRes != USBCAN_SUCCESSFUL)) {
        q->setError(systemErrorString(initHardwareRes), QCanBusDevice::ConnectionError);
        return false;
    }

    const int bitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    const bool receiveOwn = q->configurationParameter(QCanBusDevice::ReceiveOwnKey).toBool();

    tUcanInitCanParam param = {};
    param.m_dwSize = sizeof(param);
    param.m_bMode  = receiveOwn ? kUcanModeTxEcho : kUcanModeNormal;
    param.m_bOCR   = USBCAN_OCR_DEFAULT;
    param.m_dwACR  = USBCAN_ACR_ALL;
    param.m_dwAMR  = USBCAN_AMR_ALL;
    param.m_dwBaudrate = bitrateCodeFromBitrate(bitrate);
    param.m_wNrOfRxBufferEntries = USBCAN_DEFAULT_BUFFER_ENTRIES;
    param.m_wNrOfTxBufferEntries = USBCAN_DEFAULT_BUFFER_ENTRIES;

    const UCANRET initCanResult = ::UcanInitCanEx2(handle, channel, &param);
    if (Q_UNLIKELY(initCanResult != USBCAN_SUCCESSFUL)) {
        ::UcanDeinitHardware(handle);
        q->setError(systemErrorString(initCanResult), QCanBusDevice::ConnectionError);
        return false;
    }

    return true;
}

void SystecCanBackendPrivate::close()
{
    Q_Q(SystecCanBackend);

    enableWriteNotification(false);

    if (outgoingEventNotifier) {
        delete outgoingEventNotifier;
        outgoingEventNotifier = nullptr;
    }

    const UCANRET deinitCanRes = UcanDeinitCanEx(handle, channel);
    if (Q_UNLIKELY(deinitCanRes != USBCAN_SUCCESSFUL))
        q->setError(systemErrorString(deinitCanRes), QCanBusDevice::ConfigurationError);

    // TODO: other channel keeps working?
    const UCANRET deinitHardwareRes = UcanDeinitHardware(handle);
    if (Q_UNLIKELY(deinitHardwareRes != USBCAN_SUCCESSFUL))
        emit q->setError(systemErrorString(deinitHardwareRes), QCanBusDevice::ConnectionError);
}

void SystecCanBackendPrivate::eventHandler(QEvent *event)
{
    const int code = event->type() - QEvent::User;

    if (code == USBCAN_EVENT_RECEIVE)
        readAllReceivedMessages();
}

bool SystecCanBackendPrivate::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                                        const QVariant &value)
{
    Q_Q(SystecCanBackend);

    switch (key) {
    case QCanBusDevice::BitRateKey:
        return verifyBitRate(value.toInt());
    case QCanBusDevice::ReceiveOwnKey:
        if (Q_UNLIKELY(q->state() != QCanBusDevice::UnconnectedState)) {
            q->setError(SystecCanBackend::tr("Cannot configure TxEcho for open device"),
                        QCanBusDevice::ConfigurationError);
            return false;
        }
        return true;
    default:
        q->setError(SystecCanBackend::tr("Unsupported configuration key: %1").arg(key),
                    QCanBusDevice::ConfigurationError);
        return false;
    }
}

bool SystecCanBackendPrivate::setupChannel(const QString &interfaceName)
{
    Q_Q(SystecCanBackend);

    const QRegularExpression re(QStringLiteral("can(\\d)\\.(\\d)"));
    const QRegularExpressionMatch match = re.match(interfaceName);

    if (Q_LIKELY(match.hasMatch())) {
        device = quint8(match.captured(1).toUShort());
        channel = quint8(match.captured(2).toUShort());
    } else {
        q->setError(SystecCanBackend::tr("Invalid interface '%1'.")
                    .arg(interfaceName), QCanBusDevice::ConnectionError);
        return false;
    }

    return true;
}

void SystecCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(SystecCanBackend);

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
    q->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, false);
}

QString SystecCanBackendPrivate::systemErrorString(int errorCode)
{
    switch (errorCode) {
    case USBCAN_ERR_RESOURCE:
        return SystecCanBackend::tr("Could not create a resource (memory, handle, ...)");
    case USBCAN_ERR_MAXMODULES:
        return SystecCanBackend::tr("The maximum number of open modules is exceeded");
    case USBCAN_ERR_HWINUSE:
        return SystecCanBackend::tr("The module is already in use");
    case USBCAN_ERR_ILLVERSION:
        return SystecCanBackend::tr("The software versions of the module and library are incompatible");
    case USBCAN_ERR_ILLHW:
        return SystecCanBackend::tr("The module with the corresponding device number is not connected");
    case USBCAN_ERR_ILLHANDLE:
        return SystecCanBackend::tr("Wrong USB-CAN-Handle handed over to the function");
    case USBCAN_ERR_ILLPARAM:
        return SystecCanBackend::tr("Wrong parameter handed over to the function");
    case USBCAN_ERR_BUSY:
        return SystecCanBackend::tr("Instruction can not be processed at this time");
    case USBCAN_ERR_TIMEOUT:
        return SystecCanBackend::tr("No answer from the module");
    case USBCAN_ERR_IOFAILED:
        return SystecCanBackend::tr("A request for the driver failed");
    case USBCAN_ERR_DLL_TXFULL:
        return SystecCanBackend::tr("The message did not fit into the transmission queue");
    case USBCAN_ERR_MAXINSTANCES:
        return SystecCanBackend::tr("Maximum number of applications is reached");
    case USBCAN_ERR_CANNOTINIT:
        return SystecCanBackend::tr("CAN-interface is not yet initialized");
    case USBCAN_ERR_DISCONNECT:
        return SystecCanBackend::tr("USB-CANmodul was disconnected");
    case USBCAN_ERR_NOHWCLASS:
        return SystecCanBackend::tr("The needed device class does not exist");
    case USBCAN_ERR_ILLCHANNEL:
        return SystecCanBackend::tr("Illegal CAN channel for GW-001/GW-002");
    case USBCAN_ERR_ILLHWTYPE:
        return SystecCanBackend::tr("The API function can not be used with this hardware");
    case USBCAN_ERR_SERVER_TIMEOUT:
        return SystecCanBackend::tr("The command server did not send a reply to a command");
    default:
        return SystecCanBackend::tr("Unknown error code '%1'.").arg(errorCode);
    }
}

void SystecCanBackendPrivate::enableWriteNotification(bool enable)
{
    Q_Q(SystecCanBackend);

    if (outgoingEventNotifier) {
        if (enable) {
            if (!outgoingEventNotifier->isActive())
                outgoingEventNotifier->start();
        } else {
            outgoingEventNotifier->stop();
        }
    } else if (enable) {
        outgoingEventNotifier = new OutgoingEventNotifier(this, q);
        outgoingEventNotifier->start(0);
    }
}

void SystecCanBackendPrivate::startWrite()
{
    Q_Q(SystecCanBackend);

    if (!q->hasOutgoingFrames()) {
        enableWriteNotification(false);
        return;
    }

    const QCanBusFrame frame = q->dequeueOutgoingFrame();
    const QByteArray payload = frame.payload();
    const qsizetype payloadSize = payload.size();

    tCanMsgStruct message = {};

    message.m_dwID = frame.frameId();
    message.m_bDLC = quint8(payloadSize);

    message.m_bFF = frame.hasExtendedFrameFormat() ? USBCAN_MSG_FF_EXT : USBCAN_MSG_FF_STD;

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
        message.m_bFF |= USBCAN_MSG_FF_RTR; // remote request frame without payload
    else
        ::memcpy(message.m_bData, payload.constData(), payloadSize);

    const UCANRET result = ::UcanWriteCanMsgEx(handle, channel, &message, nullptr);
    if (Q_UNLIKELY(result != USBCAN_SUCCESSFUL))
        q->setError(systemErrorString(result), QCanBusDevice::WriteError);
    else
        emit q->framesWritten(qint64(1));

    if (q->hasOutgoingFrames())
        enableWriteNotification(true);
}

void SystecCanBackendPrivate::readAllReceivedMessages()
{
    Q_Q(SystecCanBackend);

    QList<QCanBusFrame> newFrames;

    for (;;) {
        tCanMsgStruct message = {};

        const UCANRET result = ::UcanReadCanMsgEx(handle, &channel, &message, nullptr);
        if (result == USBCAN_WARN_NODATA)
            break;

        if (Q_UNLIKELY(result != USBCAN_SUCCESSFUL)) {
            // handle errors

            q->setError(systemErrorString(result), QCanBusDevice::ReadError);
            break;
        }

        QCanBusFrame frame(message.m_dwID,
                           QByteArray(reinterpret_cast<const char *>(message.m_bData),
                                      int(message.m_bDLC)));

        // TODO: Timestamp can also be set to 100 us resolution with kUcanModeHighResTimer
        frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(message.m_dwTime * 1000));
        frame.setExtendedFrameFormat(message.m_bFF & USBCAN_MSG_FF_EXT);
        frame.setLocalEcho(message.m_bFF & USBCAN_MSG_FF_ECHO);
        frame.setFrameType((message.m_bFF & USBCAN_MSG_FF_RTR)
                           ? QCanBusFrame::RemoteRequestFrame
                           : QCanBusFrame::DataFrame);

        newFrames.append(std::move(frame));
    }

    q->enqueueReceivedFrames(newFrames);
}

bool SystecCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(SystecCanBackend);

    if (Q_UNLIKELY(q->state() != QCanBusDevice::UnconnectedState)) {
        q->setError(SystecCanBackend::tr("Cannot configure bitrate for open device"),
                    QCanBusDevice::ConfigurationError);
        return false;
    }

    if (Q_UNLIKELY(bitrateCodeFromBitrate(bitrate) == 0)) {
        q->setError(SystecCanBackend::tr("Unsupported bitrate %1.").arg(bitrate),
                    QCanBusDevice::ConfigurationError);
        return false;
    }

    return true;
}

void SystecCanBackendPrivate::resetController()
{
    ::UcanResetCan(handle);
}

QCanBusDevice::CanBusStatus SystecCanBackendPrivate::busStatus()
{
    Q_Q(SystecCanBackend);

    tStatusStruct status = {};
    const UCANRET result = ::UcanGetStatus(handle, &status);

    if (Q_UNLIKELY(result != USBCAN_SUCCESSFUL)) {
        qCWarning(QT_CANBUS_PLUGINS_SYSTECCAN, "Can not query CAN bus status.");
        q->setError(SystecCanBackend::tr("Can not query CAN bus status."), QCanBusDevice::ConfigurationError);
        return QCanBusDevice::CanBusStatus::Unknown;
    }

    if (status.m_wCanStatus & USBCAN_CANERR_BUSOFF)
        return QCanBusDevice::CanBusStatus::BusOff;

    if (status.m_wCanStatus & USBCAN_CANERR_BUSHEAVY)
        return QCanBusDevice::CanBusStatus::Error;

    if (status.m_wCanStatus & USBCAN_CANERR_BUSLIGHT)
        return QCanBusDevice::CanBusStatus::Warning;

    if (status.m_wCanStatus == USBCAN_CANERR_OK)
        return QCanBusDevice::CanBusStatus::Good;

    return QCanBusDevice::CanBusStatus::Unknown;
}

SystecCanBackend::SystecCanBackend(const QString &name, QObject *parent) :
    QCanBusDevice(parent),
    d_ptr(new SystecCanBackendPrivate(this))
{
    Q_D(SystecCanBackend);

    d->setupChannel(name);
    d->setupDefaultConfigurations();
}

SystecCanBackend::~SystecCanBackend()
{
    if (state() == QCanBusDevice::ConnectedState)
        close();

    delete d_ptr;
}

bool SystecCanBackend::open()
{
    Q_D(SystecCanBackend);

    if (!d->open())
        return false;

    // Apply all stored configurations except bitrate and receive own,
    // because these cannot be applied after opening the device
    const auto keys = configurationKeys();
    for (ConfigurationKey key : keys) {
        if (key == BitRateKey || key == ReceiveOwnKey)
            continue;
        const QVariant param = configurationParameter(key);
        const bool success = d->setConfigurationParameter(key, param);
        if (Q_UNLIKELY(!success)) {
            qCWarning(QT_CANBUS_PLUGINS_SYSTECCAN, "Cannot apply parameter %d with value %ls.",
                      key, qUtf16Printable(param.toString()));
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void SystecCanBackend::close()
{
    Q_D(SystecCanBackend);

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void SystecCanBackend::setConfigurationParameter(ConfigurationKey key, const QVariant &value)
{
    Q_D(SystecCanBackend);

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool SystecCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(SystecCanBackend);

    if (Q_UNLIKELY(state() != QCanBusDevice::ConnectedState))
        return false;

    if (Q_UNLIKELY(!newData.isValid())) {
        setError(tr("Cannot write invalid QCanBusFrame"), QCanBusDevice::WriteError);
        return false;
    }

    const QCanBusFrame::FrameType type = newData.frameType();
    if (Q_UNLIKELY(type != QCanBusFrame::DataFrame && type != QCanBusFrame::RemoteRequestFrame)) {
        setError(tr("Unable to write a frame with unacceptable type"),
                 QCanBusDevice::WriteError);
        return false;
    }

    // CAN FD frame format is not supported by the hardware yet
    if (Q_UNLIKELY(newData.hasFlexibleDataRateFormat())) {
        setError(tr("CAN FD frame format not supported"), QCanBusDevice::WriteError);
        return false;
    }

    enqueueOutgoingFrame(newData);
    d->enableWriteNotification(true);

    return true;
}

// TODO: Implement me
QString SystecCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);

    return QString();
}

void SystecCanBackend::resetController()
{
    Q_D(SystecCanBackend);
    d->resetController();
}

bool SystecCanBackend::hasBusStatus() const
{
    return true;
}

QCanBusDevice::CanBusStatus SystecCanBackend::busStatus()
{
    Q_D(SystecCanBackend);

    return d->busStatus();
}

QCanBusDeviceInfo SystecCanBackend::deviceInfo() const
{
    tUcanHardwareInfoEx hardwareInfo = {};
    UcanGetHardwareInfoEx2(d_ptr->handle, &hardwareInfo, nullptr, nullptr);

    const QString serialNumber = QString::number(hardwareInfo.m_dwSerialNr);
    const QString description = descriptionString(hardwareInfo.m_dwProductCode);
    return createDeviceInfo(serialNumber, description, hardwareInfo.m_bDeviceNr, d_ptr->channel);
}

QT_END_NAMESPACE
