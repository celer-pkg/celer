// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (c) 2020 Andre Hartmann <aha_1980@gmx.de>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "peakcanbackend.h"
#include "peakcanbackend_p.h"
#include "peakcan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>
#include <vector>

#ifdef Q_OS_WIN32
#   include <QtCore/qwineventnotifier.h>
#else
#   include <QtCore/qsocketnotifier.h>
#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_PEAKCAN)

#ifndef LINK_LIBPCANBASIC
Q_GLOBAL_STATIC(QLibrary, pcanLibrary)
#endif

bool PeakCanBackend::canCreate(QString *errorReason)
{
#ifdef LINK_LIBPCANBASIC
    Q_UNUSED(errorReason);
#else
    static bool symbolsResolved = resolvePeakCanSymbols(pcanLibrary());
    if (Q_UNLIKELY(!symbolsResolved)) {
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot load library: %ls",
                qUtf16Printable(pcanLibrary()->errorString()));
        *errorReason = pcanLibrary()->errorString();
        return false;
    }
#endif

    char apiVersion[32];
    TPCANStatus stat = CAN_GetValue(PCAN_NONEBUS, PCAN_API_VERSION, apiVersion, sizeof(apiVersion));
    if (Q_UNLIKELY(stat != PCAN_ERROR_OK)) {
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot resolve PCAN-API version!");
        return false;
    }
    qCInfo(QT_CANBUS_PLUGINS_PEAKCAN, "Using PCAN-API version: %s", apiVersion);

    return true;
}

struct PcanChannel{
    char        name[6];
    TPCANHandle index;
};
static const PcanChannel pcanChannels[] = {
    { "usb0",  PCAN_USBBUS1  },
    { "usb1",  PCAN_USBBUS2  },
    { "usb2",  PCAN_USBBUS3  },
    { "usb3",  PCAN_USBBUS4  },
    { "usb4",  PCAN_USBBUS5  },
    { "usb5",  PCAN_USBBUS6  },
    { "usb6",  PCAN_USBBUS7  },
    { "usb7",  PCAN_USBBUS8  },
    { "usb8",  PCAN_USBBUS9  },
    { "usb9",  PCAN_USBBUS10 },
    { "usb10", PCAN_USBBUS11 },
    { "usb11", PCAN_USBBUS12 },
    { "usb12", PCAN_USBBUS13 },
    { "usb13", PCAN_USBBUS14 },
    { "usb14", PCAN_USBBUS15 },
    { "usb15", PCAN_USBBUS16 },
    { "pci0",  PCAN_PCIBUS1  },
    { "pci1",  PCAN_PCIBUS2  },
    { "pci2",  PCAN_PCIBUS3  },
    { "pci3",  PCAN_PCIBUS4  },
    { "pci4",  PCAN_PCIBUS5  },
    { "pci5",  PCAN_PCIBUS6  },
    { "pci6",  PCAN_PCIBUS7  },
    { "pci7",  PCAN_PCIBUS8  },
    { "pci8",  PCAN_PCIBUS9  },
    { "pci9",  PCAN_PCIBUS10 },
    { "pci10", PCAN_PCIBUS11 },
    { "pci11", PCAN_PCIBUS12 },
    { "pci12", PCAN_PCIBUS13 },
    { "pci13", PCAN_PCIBUS14 },
    { "pci14", PCAN_PCIBUS15 },
    { "pci15", PCAN_PCIBUS16 },
    { "none",  PCAN_NONEBUS  }
};

QList<QCanBusDeviceInfo> PeakCanBackend::interfacesByChannelCondition(Availability available)
{
    QList<QCanBusDeviceInfo> result;

    for (int i = 0; pcanChannels[i].index != PCAN_NONEBUS; ++i) {
        uint value = 0;
        const TPCANHandle index = pcanChannels[i].index;
        const TPCANStatus stat = ::CAN_GetValue(index, PCAN_CHANNEL_CONDITION,
                                                &value, sizeof(value));
        if ((stat == PCAN_ERROR_OK) && (value & uint(available))) {
            const TPCANStatus fdStat = ::CAN_GetValue(index, PCAN_CHANNEL_FEATURES,
                                                      &value, sizeof(value));
            const bool isFd = (fdStat == PCAN_ERROR_OK) && (value & FEATURE_FD_CAPABLE);

            char description[256] = {0};
            const TPCANStatus descStat = ::CAN_GetValue(index, PCAN_HARDWARE_NAME,
                                                        description, sizeof(description));
            if (descStat != PCAN_ERROR_OK)
                description[0] = 0;

            int channel = 0;
            const TPCANStatus chnStat = ::CAN_GetValue(index, PCAN_CONTROLLER_NUMBER,
                                                       &channel, sizeof(channel));
            if (chnStat != PCAN_ERROR_OK)
                channel = 0;

            QString alias;
            quint32 deviceId = 0;
            const TPCANStatus idStat = ::CAN_GetValue(index, PCAN_DEVICE_ID,
                                                      &deviceId, sizeof(deviceId));
            if (idStat == PCAN_ERROR_OK)
                alias = QString::number(deviceId);

            result.append(QCanBusDevice::createDeviceInfo(QStringLiteral("peakcan"),
                                           QLatin1String(pcanChannels[i].name),
                                           QString(), QLatin1String(description),
                                           alias, channel, false, isFd));
        }
    }

    return result;
}

static QString pcanChannelNameForIndex(uint index)
{
    const auto pcanChannel = std::find_if(std::begin(pcanChannels), std::end(pcanChannels),
                                          [index](PcanChannel channel) {
        return channel.index == index;
    });
    if (Q_LIKELY(pcanChannel != std::end(pcanChannels)))
        return pcanChannel->name;

    qWarning("%s: Cannot get channel name for index %u.", Q_FUNC_INFO, index);
    return QStringLiteral("none");
}

QList<QCanBusDeviceInfo> PeakCanBackend::interfacesByAttachedChannels(Availability available, bool *ok)
{
    *ok = true;
    quint32 count = 0;
    const TPCANStatus countStat = ::CAN_GetValue(0, PCAN_ATTACHED_CHANNELS_COUNT,
                                                 &count, sizeof(count));
    if (Q_UNLIKELY(countStat != PCAN_ERROR_OK)) {
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot query PCAN_ATTACHED_CHANNELS_COUNT.");
        *ok = false;
        return {};
    }
    if (count == 0)
        return {};

    std::vector<TPCANChannelInformation> infos(count);
    const TPCANStatus infosStat = ::CAN_GetValue(0, PCAN_ATTACHED_CHANNELS, infos.data(),
                                                 quint32(infos.size() * sizeof(TPCANChannelInformation)));
    if (Q_UNLIKELY(infosStat != PCAN_ERROR_OK)) {
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot query PCAN_ATTACHED_CHANNELS.");
        *ok = false;
        return {};
    }

    QList<QCanBusDeviceInfo> result;
    for (quint32 i = 0; i < count; ++i) {
        auto info = infos[i];
        if (info.channel_condition & uint(available)) {
            const QString name = pcanChannelNameForIndex(info.channel_handle);
            const QString description = info.device_name;
            const QString alias = QString::number(info.device_id);
            const int channel = info.controller_number;
            const bool isCanFd = (info.device_features & FEATURE_FD_CAPABLE);

            result.append(createDeviceInfo(QStringLiteral("peakcan"), name, QString(),
                                           description, alias,
                                           channel, false, isCanFd));
        }
    }

    return result;
}

QList<QCanBusDeviceInfo> PeakCanBackend::attachedInterfaces(Availability available)
{
#ifdef Q_OS_WIN
    bool ok = false;
    QList<QCanBusDeviceInfo> attachedChannelsResult = interfacesByAttachedChannels(available, &ok);
    if (ok)
        return attachedChannelsResult;
#endif

    QList<QCanBusDeviceInfo> result = interfacesByChannelCondition(available);
    return result;
}

QList<QCanBusDeviceInfo> PeakCanBackend::interfaces()
{
    return attachedInterfaces(Availability::Available);
}

QCanBusDeviceInfo PeakCanBackend::deviceInfo() const
{
    const uint index = d_ptr->channelIndex;
    const QString name = pcanChannelNameForIndex(index);
    const QList<QCanBusDeviceInfo> availableDevices = attachedInterfaces(Availability::Occupied);

    const auto deviceInfo = std::find_if(availableDevices.constBegin(),
                                         availableDevices.constEnd(),
                                         [name](const QCanBusDeviceInfo &info) {
        return name == info.name();
    });

    if (Q_LIKELY(deviceInfo != availableDevices.constEnd()))
        return *deviceInfo;

    qWarning("%s: Cannot get device info for index %u.", Q_FUNC_INFO, index);
    return QCanBusDevice::deviceInfo();
}

#if defined(Q_OS_WIN32)
class PeakCanReadNotifier : public QWinEventNotifier
{
    // no Q_OBJECT macro!
public:
    explicit PeakCanReadNotifier(PeakCanBackendPrivate *d, QObject *parent)
        : QWinEventNotifier(parent)
        , dptr(d)
    {
        setHandle(dptr->readHandle);

        connect(this, &QWinEventNotifier::activated, this, [this]() {
            dptr->startRead();
        });
    }

private:
    PeakCanBackendPrivate * const dptr;
};
#else
class PeakCanReadNotifier : public QSocketNotifier
{
    // no Q_OBJECT macro!
public:
    explicit PeakCanReadNotifier(PeakCanBackendPrivate *d, QObject *parent)
        : QSocketNotifier(d->readHandle, QSocketNotifier::Read, parent)
        , dptr(d)
    {
    }

protected:
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::SockAct) {
            dptr->startRead();
            return true;
        }
        return QSocketNotifier::event(e);
    }

private:
    PeakCanBackendPrivate * const dptr;
};
#endif

class PeakCanWriteNotifier : public QTimer
{
    // no Q_OBJECT macro!
public:
    PeakCanWriteNotifier(PeakCanBackendPrivate *d, QObject *parent)
        : QTimer(parent)
        , dptr(d)
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
    PeakCanBackendPrivate * const dptr;
};

PeakCanBackendPrivate::PeakCanBackendPrivate(PeakCanBackend *q)
    : q_ptr(q)
{
}

struct BitrateItem
{
    int bitrate;
    TPCANBaudrate code;
};

struct BitrateLessFunctor
{
    bool operator()( const BitrateItem &item1, const BitrateItem &item2) const
    {
        return item1.bitrate < item2.bitrate;
    }
};

static TPCANBaudrate bitrateCodeFromBitrate(int bitrate)
{
    static const BitrateItem bitratetable[] = {
        { 5000, PCAN_BAUD_5K },
        { 10000, PCAN_BAUD_10K },
        { 20000, PCAN_BAUD_20K },
        { 33000, PCAN_BAUD_33K },
        { 47000, PCAN_BAUD_47K },
        { 50000, PCAN_BAUD_50K },
        { 83000, PCAN_BAUD_83K },
        { 95000, PCAN_BAUD_95K },
        { 100000, PCAN_BAUD_100K },
        { 125000, PCAN_BAUD_125K },
        { 250000, PCAN_BAUD_250K },
        { 500000, PCAN_BAUD_500K },
        { 800000, PCAN_BAUD_800K },
        { 1000000, PCAN_BAUD_1M }
    };

    static const BitrateItem *endtable = bitratetable + (sizeof(bitratetable) / sizeof(*bitratetable));

    const BitrateItem item = { bitrate , 0 };
    const BitrateItem *where = std::lower_bound(bitratetable, endtable, item, BitrateLessFunctor());
    return where != endtable ? where->code : PCAN_BAUD_INVALID;
}

static QByteArray nominalBitrateString(int nominalBitrate)
{
    switch (nominalBitrate) {
    case 125000:
        return "f_clock=80000000, nom_brp=40, nom_tseg1=12, nom_tseg2=3, nom_sjw=1";
    case 250000:
        return "f_clock=80000000, nom_brp=20, nom_tseg1=12, nom_tseg2=3, nom_sjw=1";
    case 500000:
        return "f_clock=80000000, nom_brp=10, nom_tseg1=12, nom_tseg2=3, nom_sjw=1";
    case 1000000:
        return "f_clock=80000000, nom_brp=10, nom_tseg1=5,  nom_tseg2=2, nom_sjw=1";
    default:
        return QByteArray();
    }
}

static QByteArray dataBitrateString(int dataBitrate)
{
    switch (dataBitrate) {
    case 2000000:
        return ", data_brp=4, data_tseg1=7, data_tseg2=2, data_sjw=1";
    case 4000000:
        return ", data_brp=2, data_tseg1=7, data_tseg2=2, data_sjw=1";
    case 8000000:
        return ", data_brp=1, data_tseg1=7, data_tseg2=2, data_sjw=1";
    case 10000000:
        return ", data_brp=1, data_tseg1=5, data_tseg2=2, data_sjw=1";
    default:
        return QByteArray();
    }
}

static QByteArray bitrateStringFromBitrate(int nominalBitrate, int dataBitrate)
{
    QByteArray result = nominalBitrateString(nominalBitrate);

    if (result.isEmpty())
        return QByteArray();

    result += dataBitrateString(dataBitrate);

    return result;
}

bool PeakCanBackendPrivate::open()
{
    Q_Q(PeakCanBackend);

    const int nominalBitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    TPCANStatus st = PCAN_ERROR_OK;

    if (isFlexibleDatarateEnabled) {
        const int dataBitrate = q->configurationParameter(QCanBusDevice::DataBitRateKey).toInt();
        const QByteArray bitrateStr = bitrateStringFromBitrate(nominalBitrate, dataBitrate);
        st = ::CAN_InitializeFD(channelIndex, const_cast<char *>(bitrateStr.data()));
    } else {
        const TPCANBaudrate bitrateCode = bitrateCodeFromBitrate(nominalBitrate);
        st = ::CAN_Initialize(channelIndex, bitrateCode, 0, 0, 0);
    }

    if (Q_UNLIKELY(st != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(st);
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot initialize channel %u hardware: %ls",
                   uint(channelIndex), qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::ConnectionError);
        return false;
    }

#if defined(Q_OS_WIN32)
    if (readHandle == INVALID_HANDLE_VALUE) {
        readHandle = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (Q_UNLIKELY(!readHandle)) {
            const QString errorString = qt_error_string(int(::GetLastError()));
            qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot create channel %u receive event handler: %ls",
                       uint(channelIndex), qUtf16Printable(errorString));
            q->setError(errorString, QCanBusDevice::ConnectionError);
            return false;
        }
    }

    const TPCANStatus err = ::CAN_SetValue(channelIndex, PCAN_RECEIVE_EVENT, &readHandle, sizeof(readHandle));
    if (Q_UNLIKELY(err != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(err);
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot register channel %u receive event handler: %ls",
                   uint(channelIndex), qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::ConnectionError);
        return false;
    }

#else
    const TPCANStatus err = ::CAN_GetValue(channelIndex, PCAN_RECEIVE_EVENT, &readHandle, sizeof(readHandle));
    if (Q_UNLIKELY(err != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(err);
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot create channel %u receive event handler: %ls",
                   uint(channelIndex), qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::ConnectionError);
        return false;
    }
#endif

    writeNotifier = new PeakCanWriteNotifier(this, q);
    writeNotifier->setInterval(0);

    readNotifier = new PeakCanReadNotifier(this, q);
    readNotifier->setEnabled(true);

    isOpen = true;
    return true;
}

void PeakCanBackendPrivate::close()
{
    Q_Q(PeakCanBackend);

    delete readNotifier;
    readNotifier = nullptr;

    delete writeNotifier;
    writeNotifier = nullptr;

    quint32 value = 0;
    const TPCANStatus err = ::CAN_SetValue(channelIndex, PCAN_RECEIVE_EVENT, &value, sizeof(value));
    if (Q_UNLIKELY(err != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(err);
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot unregister receive event handler: %ls",
                   qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::ConnectionError);
    }

    const TPCANStatus st = ::CAN_Uninitialize(channelIndex);
    if (Q_UNLIKELY(st != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(st);
        qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot uninitialize channel %u: %ls",
                   uint(channelIndex), qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::ConnectionError);
    }

#if defined(Q_OS_WIN32)
    if (readHandle && (readHandle != INVALID_HANDLE_VALUE)) {
        const QString errorString = qt_error_string(int(::GetLastError()));
        if (Q_UNLIKELY(!::CloseHandle(readHandle))) {
            qCCritical(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot close read handle: %ls",
                       qUtf16Printable(errorString));
            q->setError(errorString, QCanBusDevice::ConnectionError);
        }
        readHandle = INVALID_HANDLE_VALUE;
    }
#else
    readHandle = -1;
#endif

    isOpen = false;
}

bool PeakCanBackendPrivate::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                                      const QVariant &value)
{
    Q_Q(PeakCanBackend);

    switch (key) {
    case QCanBusDevice::BitRateKey:
        return verifyBitRate(value.toInt());
    case QCanBusDevice::CanFdKey:
        isFlexibleDatarateEnabled = value.toBool();
        return true;
    case QCanBusDevice::DataBitRateKey: {
        const int dataBitrate = value.toInt();
        if (Q_UNLIKELY(dataBitrateString(dataBitrate).isEmpty())) {
            qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Unsupported data bitrate value: %d", dataBitrate);
            q->setError(PeakCanBackend::tr("Unsupported data bitrate value: %1.").arg(dataBitrate),
                        QCanBusDevice::ConfigurationError);
            return false;
        }
        return true;
    }
    default:
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Unsupported configuration key: %d", key);
        q->setError(PeakCanBackend::tr("Unsupported configuration key: %1").arg(key),
                    QCanBusDevice::ConfigurationError);
        return false;
    }
}

void PeakCanBackendPrivate::setupChannel(const QByteArray &interfaceName)
{
    const PcanChannel *chn = pcanChannels;
    while (chn->index != PCAN_NONEBUS && chn->name != interfaceName)
        ++chn;
    channelIndex = chn->index;
}

// Calls only when the device is closed
void PeakCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(PeakCanBackend);

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
}

QString PeakCanBackendPrivate::systemErrorString(TPCANStatus errorCode)
{
    QByteArray buffer(256, 0);
    if (Q_UNLIKELY(::CAN_GetErrorText(errorCode, 0, buffer.data()) != PCAN_ERROR_OK))
        return PeakCanBackend::tr("Unable to retrieve an error string");
    return QString::fromLatin1(buffer.constData(), -1);
}

enum CanFrameDlc {
    Dlc00 =  0,
    Dlc01 =  1,
    Dlc02 =  2,
    Dlc03 =  3,
    Dlc04 =  4,
    Dlc05 =  5,
    Dlc06 =  6,
    Dlc07 =  7,
    Dlc08 =  8,
    Dlc12 =  9,
    Dlc16 = 10,
    Dlc20 = 11,
    Dlc24 = 12,
    Dlc32 = 13,
    Dlc48 = 14,
    Dlc64 = 15
};

static CanFrameDlc sizeToDlc(int size)
{
    switch (size) {
    case 12:
        return Dlc12;
    case 16:
        return Dlc16;
    case 20:
        return Dlc20;
    case 24:
        return Dlc24;
    case 32:
        return Dlc32;
    case 48:
        return Dlc48;
    case 64:
        return Dlc64;
    default:
        if (size >= 0 && size <= 8)
            return static_cast<CanFrameDlc>(size);

        return Dlc00;
    }
}

static int dlcToSize(CanFrameDlc dlc)
{
    switch (dlc) {
    case Dlc00:
    case Dlc01:
    case Dlc02:
    case Dlc03:
    case Dlc04:
    case Dlc05:
    case Dlc06:
    case Dlc07:
    case Dlc08:
        return static_cast<int>(dlc);
    case Dlc12:
        return 12;
    case Dlc16:
        return 16;
    case Dlc20:
        return 20;
    case Dlc24:
        return 24;
    case Dlc32:
        return 32;
    case Dlc48:
        return 48;
    case Dlc64:
        return 64;
    }
    return 0;
}

void PeakCanBackendPrivate::startWrite()
{
    Q_Q(PeakCanBackend);

    if (!q->hasOutgoingFrames()) {
        writeNotifier->stop();
        return;
    }

    const QCanBusFrame frame = q->dequeueOutgoingFrame();
    const QByteArray payload = frame.payload();
    const qsizetype payloadSize = payload.size();
    TPCANStatus st = PCAN_ERROR_OK;

    if (isFlexibleDatarateEnabled) {
        TPCANMsgFD message = {};
        message.ID = frame.frameId();
        message.DLC = sizeToDlc(payloadSize);
        message.MSGTYPE = frame.hasExtendedFrameFormat() ? PCAN_MESSAGE_EXTENDED
                                                         : PCAN_MESSAGE_STANDARD;

        if (frame.hasFlexibleDataRateFormat())
            message.MSGTYPE |= PCAN_MESSAGE_FD;
        if (frame.hasBitrateSwitch())
            message.MSGTYPE |= PCAN_MESSAGE_BRS;

        if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
            message.MSGTYPE |= PCAN_MESSAGE_RTR; // we do not care about the payload
        else
            ::memcpy(message.DATA, payload.constData(), payloadSize);
        st = ::CAN_WriteFD(channelIndex, &message);
    } else if (frame.hasFlexibleDataRateFormat()) {
        const char errorString[] = "Cannot send CAN FD frame format as CAN FD is not enabled.";
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN(), errorString);
        q->setError(PeakCanBackend::tr(errorString), QCanBusDevice::WriteError);
    } else {
        TPCANMsg message = {};
        message.ID = frame.frameId();
        message.LEN = static_cast<quint8>(payloadSize);
        message.MSGTYPE = frame.hasExtendedFrameFormat() ? PCAN_MESSAGE_EXTENDED
                                                         : PCAN_MESSAGE_STANDARD;

        if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
            message.MSGTYPE |= PCAN_MESSAGE_RTR; // we do not care about the payload
        else
            ::memcpy(message.DATA, payload.constData(), payloadSize);
        st = ::CAN_Write(channelIndex, &message);
    }

    if (Q_UNLIKELY(st != PCAN_ERROR_OK)) {
        const QString errorString = systemErrorString(st);
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot write frame: %ls",
                  qUtf16Printable(errorString));
        q->setError(errorString, QCanBusDevice::WriteError);
    } else {
        emit q->framesWritten(qint64(1));
    }

    if (q->hasOutgoingFrames() && !writeNotifier->isActive())
        writeNotifier->start();
}

void PeakCanBackendPrivate::startRead()
{
    Q_Q(PeakCanBackend);

    QList<QCanBusFrame> newFrames;

    for (;;) {
        if (isFlexibleDatarateEnabled) {
            TPCANMsgFD message = {};
            TPCANTimestampFD timestamp = {};

            const TPCANStatus st = ::CAN_ReadFD(channelIndex, &message, &timestamp);
            if (st != PCAN_ERROR_OK) {
                if (Q_UNLIKELY(st != PCAN_ERROR_QRCVEMPTY)) {
                    const QString errorString = systemErrorString(st);
                    qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot read frame: %ls",
                              qUtf16Printable(errorString));
                    q->setError(errorString, QCanBusDevice::ReadError);
                }
                break;
            }

            // Filter out PCAN status frames, to avoid turning them
            // into QCanBusFrame::DataFrames with random canId
            if (Q_UNLIKELY(message.MSGTYPE & PCAN_MESSAGE_STATUS))
                continue;

            const int size = dlcToSize(static_cast<CanFrameDlc>(message.DLC));
            QCanBusFrame frame(message.ID, QByteArray(reinterpret_cast<const char *>(message.DATA), size));
            frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(static_cast<qint64>(timestamp)));
            frame.setExtendedFrameFormat(message.MSGTYPE & PCAN_MESSAGE_EXTENDED);
            frame.setFrameType((message.MSGTYPE & PCAN_MESSAGE_RTR)
                               ? QCanBusFrame::RemoteRequestFrame : QCanBusFrame::DataFrame);
            frame.setFlexibleDataRateFormat(message.MSGTYPE & PCAN_MESSAGE_FD);
            frame.setBitrateSwitch(message.MSGTYPE & PCAN_MESSAGE_BRS);
            frame.setErrorStateIndicator(message.MSGTYPE & PCAN_MESSAGE_ESI);

            newFrames.append(std::move(frame));
        } else {
            TPCANMsg message = {};
            TPCANTimestamp timestamp = {};

            const TPCANStatus st = ::CAN_Read(channelIndex, &message, &timestamp);
            if (st != PCAN_ERROR_OK) {
                if (Q_UNLIKELY(st != PCAN_ERROR_QRCVEMPTY)) {
                    const QString errorString = systemErrorString(st);
                    qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot read frame: %ls",
                              qUtf16Printable(errorString));
                    q->setError(errorString, QCanBusDevice::ReadError);
                }
                break;
            }

            // Filter out PCAN status frames, to avoid turning them
            // into QCanBusFrame::DataFrames with random canId
            if (Q_UNLIKELY(message.MSGTYPE & PCAN_MESSAGE_STATUS))
                continue;

            const int size = static_cast<int>(message.LEN);
            QCanBusFrame frame(message.ID, QByteArray(reinterpret_cast<const char *>(message.DATA), size));
            const quint64 millis = timestamp.millis + Q_UINT64_C(0x100000000) * timestamp.millis_overflow;
            const quint64 micros = Q_UINT64_C(1000) * millis + timestamp.micros;
            frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(static_cast<qint64>(micros)));
            frame.setExtendedFrameFormat(message.MSGTYPE & PCAN_MESSAGE_EXTENDED);
            frame.setFrameType((message.MSGTYPE & PCAN_MESSAGE_RTR)
                               ? QCanBusFrame::RemoteRequestFrame : QCanBusFrame::DataFrame);

            newFrames.append(std::move(frame));
        }
    }

    q->enqueueReceivedFrames(newFrames);
}

bool PeakCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(PeakCanBackend);

    if (Q_UNLIKELY(isOpen)) {
        const char errorString[] = "Cannot change bitrate for already opened device.";
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, errorString);
        q->setError(PeakCanBackend::tr(errorString), QCanBusDevice::ConfigurationError);
        return false;
    }

    bool isValidBitrate = false;
    if (q->configurationParameter(QCanBusDevice::CanFdKey).toBool())
        isValidBitrate = !nominalBitrateString(bitrate).isEmpty();
    else
        isValidBitrate = bitrateCodeFromBitrate(bitrate) != PCAN_BAUD_INVALID;

    if (Q_UNLIKELY(!isValidBitrate)) {
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Unsupported bitrate value: %d.", bitrate);
        q->setError(PeakCanBackend::tr("Unsupported bitrate value: %1.").arg(bitrate),
                    QCanBusDevice::ConfigurationError);
    }

    return isValidBitrate;
}

PeakCanBackend::PeakCanBackend(const QString &name, QObject *parent)
    : QCanBusDevice(parent)
    , d_ptr(new PeakCanBackendPrivate(this))
{
    Q_D(PeakCanBackend);

    d->setupChannel(name.toLatin1());
    d->setupDefaultConfigurations();
}

PeakCanBackend::~PeakCanBackend()
{
    Q_D(PeakCanBackend);

    if (d->isOpen)
        close();

    delete d_ptr;
}

bool PeakCanBackend::open()
{
    Q_D(PeakCanBackend);

    if (!d->isOpen) {
        if (Q_UNLIKELY(!d->open()))
            return false;

        // Apply all stored configurations except bitrate, because
        // the bitrate cannot be changed after opening the device
        const auto keys = configurationKeys();
        for (ConfigurationKey key : keys) {
            if (key == QCanBusDevice::BitRateKey || key == QCanBusDevice::DataBitRateKey)
                continue;
            const QVariant param = configurationParameter(key);
            const bool success = d->setConfigurationParameter(key, param);
            if (Q_UNLIKELY(!success)) {
                qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Cannot apply parameter: %d with value: %ls.",
                          key, qUtf16Printable(param.toString()));
            }
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void PeakCanBackend::close()
{
    Q_D(PeakCanBackend);

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void PeakCanBackend::setConfigurationParameter(ConfigurationKey key, const QVariant &value)
{
    Q_D(PeakCanBackend);

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool PeakCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(PeakCanBackend);

    if (Q_UNLIKELY(state() != QCanBusDevice::ConnectedState))
        return false;

    if (Q_UNLIKELY(!newData.isValid())) {
        const QString errorString = tr("Cannot write invalid QCanBusFrame");
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "%ls", qUtf16Printable(errorString));
        setError(errorString, QCanBusDevice::WriteError);
        return false;
    }

    if (Q_UNLIKELY(newData.frameType() != QCanBusFrame::DataFrame
                   && newData.frameType() != QCanBusFrame::RemoteRequestFrame)) {
        const QString errorString = tr("Cannot write QCanBusFrame with type %1").arg(newData.frameType());
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "%ls", qUtf16Printable(errorString));
        setError(errorString, QCanBusDevice::WriteError);
        return false;
    }

    enqueueOutgoingFrame(newData);

    if (!d->writeNotifier->isActive())
        d->writeNotifier->start();

    return true;
}

// TODO: Implement me
QString PeakCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);

    return QString();
}

void PeakCanBackend::resetController()
{
    close();
    open();
}

bool PeakCanBackend::hasBusStatus() const
{
    return true;
}

QCanBusDevice::CanBusStatus PeakCanBackend::busStatus()
{
    const TPCANStatus status = ::CAN_GetStatus(d_ptr->channelIndex);

    switch (status & PCAN_ERROR_ANYBUSERR) {
    case PCAN_ERROR_OK:
        return QCanBusDevice::CanBusStatus::Good;
    case PCAN_ERROR_BUSWARNING:
        return QCanBusDevice::CanBusStatus::Warning;
    case PCAN_ERROR_BUSPASSIVE:
        return QCanBusDevice::CanBusStatus::Error;
    case PCAN_ERROR_BUSOFF:
        return QCanBusDevice::CanBusStatus::BusOff;
    default:
        qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Unknown CAN bus status: %lu.", ulong(status));
        return QCanBusDevice::CanBusStatus::Unknown;
    }
}

QT_END_NAMESPACE
