// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "vectorcanbackend.h"
#include "vectorcanbackend_p.h"
#include "vectorcan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qwineventnotifier.h>
#include <QtCore/qcoreapplication.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_VECTORCAN)

#ifndef LINK_LIBVECTORCAN
Q_GLOBAL_STATIC(QLibrary, vectorcanLibrary)
#endif

bool VectorCanBackend::canCreate(QString *errorReason)
{
#ifdef LINK_LIBVECTORCAN
    return true;
#else
    static bool symbolsResolved = resolveVectorCanSymbols(vectorcanLibrary());
    if (Q_UNLIKELY(!symbolsResolved)) {
        *errorReason = vectorcanLibrary()->errorString();
        return false;
    }
    return true;
#endif
}

QList<QCanBusDeviceInfo> VectorCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> result;

    if (Q_UNLIKELY(VectorCanBackendPrivate::loadDriver() != XL_SUCCESS))
        return result;

    XLdriverConfig config;
    if (Q_UNLIKELY(::xlGetDriverConfig(&config) != XL_SUCCESS)) {
        VectorCanBackendPrivate::cleanupDriver();
        return result;
    }

    for (uint i = 0; i < config.channelCount; ++i) {
        if (config.channel[i].hwType == XL_HWTYPE_NONE)
            continue;
        if (!(config.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN))
            continue;

        const bool isVirtual = config.channel[i].hwType == XL_HWTYPE_VIRTUAL;
        const bool isFd = config.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_SUPPORT;
        const int channel = config.channel[i].hwChannel;
        const QString name = QStringLiteral("can") + QString::number(i);
        const QString serial = QString::number(config.channel[i].serialNumber);
        const QString description = QLatin1String(config.channel[i].name);
        result.append(createDeviceInfo(QStringLiteral("vectorcan"), name,
                                       serial, description, QString(),
                                       channel, isVirtual, isFd));
    }

    VectorCanBackendPrivate::cleanupDriver();
    return result;
}

static int driverRefCount = 0;

class VectorCanReadNotifier : public QWinEventNotifier
{
    // no Q_OBJECT macro!
public:
    explicit VectorCanReadNotifier(VectorCanBackendPrivate *d, QObject *parent)
        : QWinEventNotifier(parent)
        , dptr(d)
    {
        setHandle(dptr->readHandle);

        connect(this, &QWinEventNotifier::activated, this, [this]() {
            dptr->startRead();
        });
    }

private:
    VectorCanBackendPrivate * const dptr;
};

class VectorCanWriteNotifier : public QTimer
{
    // no Q_OBJECT macro!
public:
    VectorCanWriteNotifier(VectorCanBackendPrivate *d, QObject *parent)
        : QTimer(parent)
        , dptr(d)
    {
        setInterval(0);
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
    VectorCanBackendPrivate * const dptr;
};

XLcanFdConf xlCanFdConfInit(int arbitrationBitRate, int dataBitRate)
{
    XLcanFdConf conf = {};
    const int minArbitrationBitRate = 5000;
    if (arbitrationBitRate < minArbitrationBitRate) {
        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN,
                  "Arbitration bit rate %d is to low (Minimum: %d)",
                  arbitrationBitRate, minArbitrationBitRate);
        return conf;
    }
    if (dataBitRate < arbitrationBitRate) {
        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN,
                  "Data bit rate %d must not be lower than arbitration bit rate %d",
                  dataBitRate, arbitrationBitRate);
        return conf;
    }

    int prescalerArbitration = 1;
    switch (arbitrationBitRate) {
    case 5000: prescalerArbitration = 50; break;
    case 10000: prescalerArbitration = 25; break;
    case 20000: prescalerArbitration = 20; break;
    case 33333: prescalerArbitration = 8; break;
    case 50000: prescalerArbitration = 5; break;
    case 83333: prescalerArbitration = 3; break;
    case 100000: prescalerArbitration = 4; break;
    case 125000: prescalerArbitration = 2; break;
    }
    conf.arbitrationBitRate = arbitrationBitRate;
    const int clock = 80000000;
    double samplePoint = 0.75;
    const int btlCyclesAbr = clock / conf.arbitrationBitRate / prescalerArbitration;
    conf.tseg1Abr = (btlCyclesAbr * samplePoint) - 1;
    conf.tseg2Abr = btlCyclesAbr * (1 - samplePoint);
    conf.sjwAbr = conf.tseg2Abr;

    int prescalerData = 1;
    switch (dataBitRate) {
    case 25000: prescalerData = 20; break;
    case 50000: prescalerData = 10; break;
    case 83333: prescalerData = 6; break;
    case 100000: prescalerData = 5; break;
    case 125000: prescalerData = 4; break;
    case 250000: prescalerData = 2; break;
    case 8000000: samplePoint = 0.70; break;
    }
    conf.dataBitRate = dataBitRate;
    const int btlCyclesDbr = clock / conf.dataBitRate / prescalerData;
    conf.tseg1Dbr = (btlCyclesDbr * samplePoint) - 1;
    conf.tseg2Dbr = btlCyclesDbr * (1 - samplePoint);
    conf.sjwDbr = conf.tseg2Dbr;

    return conf;
}

static qsizetype getDlcFromPayloadSize(const qsizetype size)
{
    if (size <= 8)
        return size;
    if (size <= 12)
        return 9;
    if (size <= 16)
        return 10;
    if (size <= 20)
        return 11;
    if (size <= 24)
        return 12;
    if (size <= 32)
        return 13;
    if (size <= 48)
        return 14;
    return 15;
}

VectorCanBackendPrivate::VectorCanBackendPrivate(VectorCanBackend *q)
    : q_ptr(q)
{
    startupDriver();
}

VectorCanBackendPrivate::~VectorCanBackendPrivate()
{
    cleanupDriver();
}

bool VectorCanBackendPrivate::open()
{
    Q_Q(VectorCanBackend);

    {
        XLdriverConfig config;
        if (Q_UNLIKELY(::xlGetDriverConfig(&config) != XL_SUCCESS)) {
            q->setError(VectorCanBackend::tr("Unable to get driver configuration"),
                        QCanBusDevice::CanBusError::ConnectionError);
            return false;
        }
        channelMask = config.channel[channelIndex].channelMask;
        XLaccess permissionMask = channelMask;
        const quint32 queueSize = usesCanFd ? 8192 : 256;
        const XLstatus status = ::xlOpenPort(&portHandle,
                                             const_cast<char *>(qPrintable(qApp->applicationName())),
                                             channelMask, &permissionMask, queueSize,
                                             usesCanFd ? XL_INTERFACE_VERSION_V4 : XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);

        if (Q_UNLIKELY(status != XL_SUCCESS || portHandle == XL_INVALID_PORTHANDLE)) {
            q->setError(systemErrorString(status), QCanBusDevice::ConnectionError);
            portHandle = XL_INVALID_PORTHANDLE;
            return false;
        }
        if (usesCanFd && arbBitRate != 0) {
            if (permissionMask != 0) {
                XLcanFdConf xlfdconf = xlCanFdConfInit(arbBitRate, dataBitRate);

                const XLstatus statusFd = ::xlCanFdSetConfiguration(portHandle, channelMask, &xlfdconf);
                if (Q_UNLIKELY(statusFd != XL_SUCCESS)) {
                    const QString errorString = systemErrorString(statusFd);
                    if (statusFd == XL_ERR_INVALID_ACCESS) {
                        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Unable to change the configuration: %ls.",
                                  qUtf16Printable(errorString));
                        q->setError(errorString, QCanBusDevice::CanBusError::ConfigurationError);
                    } else {
                        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Connection error: %ls.",
                                  qUtf16Printable(errorString));
                        q->setError(errorString, QCanBusDevice::CanBusError::ConnectionError);
                        return false;
                    }
                }
            } else {
                qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "No init access for channel %d! "
                                                       "Using existing configuration!", channelIndex);
            }
        }
    }

    {
        const XLstatus status = ::xlActivateChannel(portHandle, channelMask,
                                                    XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
        if (Q_UNLIKELY(status != XL_SUCCESS)) {
            q->setError(systemErrorString(status), QCanBusDevice::CanBusError::ConnectionError);
            return false;
        }
    }

    {
        const int queueLevel = 1;
        const XLstatus status = ::xlSetNotification(portHandle, &readHandle, queueLevel);
        if (Q_UNLIKELY(status != XL_SUCCESS)) {
            q->setError(systemErrorString(status), QCanBusDevice::ConnectionError);
            return false;
        }
    }

    readNotifier = new VectorCanReadNotifier(this, q);
    readNotifier->setEnabled(true);

    writeNotifier = new VectorCanWriteNotifier(this, q);

    return true;
}

void VectorCanBackendPrivate::close()
{
    Q_Q(VectorCanBackend);

    delete readNotifier;
    readNotifier = nullptr;

    delete writeNotifier;
    writeNotifier = nullptr;

    // xlClosePort can crash on systems with vxlapi.dll but no device driver installed.
    // Therefore avoid calling any close function when the portHandle is invalid anyway.
    if (portHandle == XL_INVALID_PORTHANDLE)
        return;

    {
        const XLstatus status = ::xlDeactivateChannel(portHandle, channelMask);
        if (Q_UNLIKELY(status != XL_SUCCESS)) {
            q->setError(systemErrorString(status), QCanBusDevice::CanBusError::ConnectionError);
        }
    }

    {
        const XLstatus status = ::xlClosePort(portHandle);
        if (Q_UNLIKELY(status != XL_SUCCESS)) {
            q->setError(systemErrorString(status), QCanBusDevice::ConnectionError);
        }
    }

    portHandle = XL_INVALID_PORTHANDLE;
}

bool VectorCanBackendPrivate::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                                        const QVariant &value)
{
    Q_Q(VectorCanBackend);

    switch (key) {
    case QCanBusDevice::BitRateKey:
        return setBitRate(value.toUInt());
    case QCanBusDevice::ReceiveOwnKey:
        transmitEcho = value.toBool();
        return true;
    case QCanBusDevice::DataBitRateKey:
        return setDataBitRate(value.toUInt());
    case QCanBusDevice::CanFdKey:
    {
        if (value.toBool()) {
            XLdriverConfig config;
            if (Q_LIKELY(::xlGetDriverConfig(&config) == XL_SUCCESS)) {
                if (config.channel[channelIndex].channelCapabilities & XL_CHANNEL_FLAG_CANFD_SUPPORT) {
                    usesCanFd = true;
                    return true;
                }
            }
            q->setError(VectorCanBackend::tr("Unable to set CAN FD"),
                        QCanBusDevice::CanBusError::ConfigurationError);
            return false;
        }
        usesCanFd = false;
        return true;
    }
    default:
        q->setError(VectorCanBackend::tr("Unsupported configuration key: %1").arg(key),
                    QCanBusDevice::ConfigurationError);
        return false;
    }
}

void VectorCanBackendPrivate::setupChannel(const QString &interfaceName)
{
    Q_Q(VectorCanBackend);
    if (Q_LIKELY(interfaceName.startsWith(QStringLiteral("can")))) {
        const QStringView ref = QStringView{interfaceName}.mid(3);
        bool ok = false;
        channelIndex = ref.toInt(&ok);
        if (ok && (channelIndex >= 0 && channelIndex < XL_CONFIG_MAX_CHANNELS)) {
            channelMask = xlGetChannelMask(-1, channelIndex, 0);
            return;
        } else {
            channelIndex = -1;
            q->setError(VectorCanBackend::tr("Unable to setup channel with interface name %1")
                            .arg(interfaceName), QCanBusDevice::CanBusError::ConfigurationError);
        }
    }

    qCCritical(QT_CANBUS_PLUGINS_VECTORCAN, "Unable to parse the channel %ls",
               qUtf16Printable(interfaceName));
}

void VectorCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(VectorCanBackend);

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
}

QString VectorCanBackendPrivate::systemErrorString(int errorCode) const
{
    const char *string = ::xlGetErrorString(errorCode);
    if (Q_LIKELY(string))
        return QString::fromUtf8(string);
    return VectorCanBackend::tr("Unable to retrieve an error string");
}

void VectorCanBackendPrivate::startWrite()
{
    Q_Q(VectorCanBackend);

    if (!q->hasOutgoingFrames()) {
        writeNotifier->stop();
        return;
    }

    const QCanBusFrame frame = q->dequeueOutgoingFrame();
    const QByteArray payload = frame.payload();
    const qsizetype payloadSize = payload.size();

    quint32 eventCount = 1;
    XLstatus status = XL_ERROR;
    if (usesCanFd) {
        XLcanTxEvent event = {};

        event.tag = XL_CAN_EV_TAG_TX_MSG;
        XL_CAN_TX_MSG &msg = event.tagData.canMsg;

        msg.id = frame.frameId();
        if (frame.hasExtendedFrameFormat())
            msg.id |= XL_CAN_EXT_MSG_ID;

        msg.dlc = static_cast<quint8>(getDlcFromPayloadSize(payloadSize));
        if (frame.hasFlexibleDataRateFormat())
            msg.flags = XL_CAN_TXMSG_FLAG_EDL;
        if (frame.hasBitrateSwitch())
            msg.flags |= XL_CAN_TXMSG_FLAG_BRS;
        if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
            msg.flags |= XL_CAN_TXMSG_FLAG_RTR; // we do not care about the payload
        else
            ::memcpy(msg.data, payload.constData(), payloadSize);

        status = ::xlCanTransmitEx(portHandle, channelMask, eventCount, &eventCount, &event);
    } else {
        XLevent event = {};
        event.tag = XL_TRANSMIT_MSG;
        s_xl_can_msg &msg = event.tagData.msg;

        msg.id = frame.frameId();
        if (frame.hasExtendedFrameFormat())
            msg.id |= XL_CAN_EXT_MSG_ID;

        msg.dlc = payloadSize;

        if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
            msg.flags |= XL_CAN_MSG_FLAG_REMOTE_FRAME; // we do not care about the payload
        else if (frame.frameType() == QCanBusFrame::ErrorFrame)
            msg.flags |= XL_CAN_MSG_FLAG_ERROR_FRAME; // we do not care about the payload
        else
            ::memcpy(msg.data, payload.constData(), payloadSize);

        status = ::xlCanTransmit(portHandle, channelMask, &eventCount, &event);
    }
    if (Q_UNLIKELY(status != XL_SUCCESS)) {
        q->setError(systemErrorString(status),
                    QCanBusDevice::WriteError);
    } else {
        emit q->framesWritten(qint64(eventCount));
    }

    if (q->hasOutgoingFrames())
        writeNotifier->start();
}

void VectorCanBackendPrivate::startRead()
{
    Q_Q(VectorCanBackend);

    QList<QCanBusFrame> newFrames;

    for (;;) {
        quint32 eventCount = 1;
        if (usesCanFd) {
            XLcanRxEvent event = {};

            const XLstatus status = ::xlCanReceive(portHandle, &event);
            if (Q_UNLIKELY(status != XL_SUCCESS)) {
                if (status != XL_ERR_QUEUE_IS_EMPTY) {
                    q->setError(systemErrorString(status), QCanBusDevice::ReadError);
                }
                break;
            }
            const bool isReceivedFrame = event.tag == XL_CAN_EV_TAG_RX_OK;
            const bool isEchoFrame = transmitEcho && (event.tag == XL_CAN_EV_TAG_TX_OK);
            if (!isReceivedFrame && !isEchoFrame)
                continue;

            const XL_CAN_EV_RX_MSG &msg = event.tagData.canRxOkMsg;

            int dataLength = msg.dlc;
            switch (msg.dlc) {
            case 9: dataLength = 12; break;
            case 10: dataLength = 16; break;
            case 11: dataLength = 20; break;
            case 12: dataLength = 24; break;
            case 13: dataLength = 32; break;
            case 14: dataLength = 48; break;
            case 15: dataLength = 64; break;
            }

            QCanBusFrame frame(msg.id & ~XL_CAN_EXT_MSG_ID,
                QByteArray(reinterpret_cast<const char *>(msg.data), dataLength));
            frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(event.timeStamp / 1000));
            frame.setExtendedFrameFormat(msg.id & XL_CAN_EXT_MSG_ID);
            frame.setBitrateSwitch(msg.flags & XL_CAN_RXMSG_FLAG_BRS);
            frame.setLocalEcho(isEchoFrame);
            frame.setFrameType((msg.flags & XL_CAN_RXMSG_FLAG_RTR)
                                ? QCanBusFrame::RemoteRequestFrame
                                : (msg.flags & XL_CAN_RXMSG_FLAG_EF)
                                    ? QCanBusFrame::ErrorFrame
                                    : QCanBusFrame::DataFrame);

            newFrames.append(std::move(frame));
        } else {
            XLevent event = {};

            const XLstatus status = ::xlReceive(portHandle, &eventCount, &event);
            if (Q_UNLIKELY(status != XL_SUCCESS)) {
                if (status != XL_ERR_QUEUE_IS_EMPTY) {
                    q->setError(systemErrorString(status),
                        QCanBusDevice::ReadError);
                }
                break;
            }
            if (event.tag != XL_RECEIVE_MSG)
                continue;

            const s_xl_can_msg &msg = event.tagData.msg;

            if ((msg.flags & XL_CAN_MSG_FLAG_TX_COMPLETED) && !transmitEcho)
                continue;

            QCanBusFrame frame(msg.id & ~XL_CAN_EXT_MSG_ID,
                QByteArray(reinterpret_cast<const char *>(msg.data), int(msg.dlc)));
            frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(event.timeStamp / 1000));
            frame.setExtendedFrameFormat(msg.id & XL_CAN_EXT_MSG_ID);
            frame.setLocalEcho(msg.flags & XL_CAN_MSG_FLAG_TX_COMPLETED);
            frame.setFrameType((msg.flags & XL_CAN_MSG_FLAG_REMOTE_FRAME)
                                ? QCanBusFrame::RemoteRequestFrame
                                : (msg.flags & XL_CAN_MSG_FLAG_ERROR_FRAME)
                                   ? QCanBusFrame::ErrorFrame
                                   : QCanBusFrame::DataFrame);

            newFrames.append(std::move(frame));
        }
    }

    q->enqueueReceivedFrames(newFrames);
}

XLstatus VectorCanBackendPrivate::loadDriver()
{
    if (driverRefCount == 0) {
        const XLstatus status = ::xlOpenDriver();
        if (Q_UNLIKELY(status != XL_SUCCESS))
            return status;

    } else if (Q_UNLIKELY(driverRefCount < 0)) {
        qCCritical(QT_CANBUS_PLUGINS_VECTORCAN, "Wrong driver reference counter: %d",
                   driverRefCount);
        return XL_ERR_CANNOT_OPEN_DRIVER;
    }

    ++driverRefCount;
    return XL_SUCCESS;
}

void VectorCanBackendPrivate::startupDriver()
{
    Q_Q(VectorCanBackend);

    const XLstatus status = loadDriver();
    if (Q_UNLIKELY(status != XL_SUCCESS)) {
        q->setError(systemErrorString(status),
                    QCanBusDevice::CanBusError::ConnectionError);
    }
}

void VectorCanBackendPrivate::cleanupDriver()
{
    --driverRefCount;

    if (Q_UNLIKELY(driverRefCount < 0)) {
        qCCritical(QT_CANBUS_PLUGINS_VECTORCAN, "Wrong driver reference counter: %d",
                   driverRefCount);
        driverRefCount = 0;
    } else if (driverRefCount == 0) {
        ::xlCloseDriver();
    }
}

bool VectorCanBackendPrivate::setBitRate(quint32 bitrate)
{
    Q_Q(VectorCanBackend);
    if (!usesCanFd && q->state() != QCanBusDevice::UnconnectedState) {
        const XLstatus status = ::xlCanSetChannelBitrate(portHandle, channelMask, bitrate);
        arbBitRate = bitrate;
        if (Q_UNLIKELY(status != XL_SUCCESS)) {
            q->setError(systemErrorString(status),
                        QCanBusDevice::CanBusError::ConfigurationError);
            return false;
        }
    } else if (arbBitRate != bitrate) {
        arbBitRate = bitrate;
    }

    return true;
}

bool VectorCanBackendPrivate::setDataBitRate(quint32 bitrate)
{
    if (!usesCanFd) {
        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN,
                  "Cannot set data bit rate in CAN 2.0 mode, this is only available with CAN FD");
        return false;
    }
    if (dataBitRate != bitrate) {
        if (bitrate >= 25000) { // Minimum
            dataBitRate = bitrate;
        } else {
            qCWarning(QT_CANBUS_PLUGINS_VECTORCAN,
                      "Cannot set data bit rate to less than 25000 which is the minimum");
            return false;
        }
    }
    return true;
}

VectorCanBackend::VectorCanBackend(const QString &name, QObject *parent)
    : QCanBusDevice(parent)
    , d_ptr(new VectorCanBackendPrivate(this))
{
    Q_D(VectorCanBackend);

    d->setupChannel(name);
    d->setupDefaultConfigurations();
}

VectorCanBackend::~VectorCanBackend()
{
    if (state() == ConnectedState)
        close();

    delete d_ptr;
}

bool VectorCanBackend::open()
{
    Q_D(VectorCanBackend);

    if (!d->open()) {
        close(); // sets UnconnectedState
        return false;
    }

    const auto keys = configurationKeys();
    for (ConfigurationKey key : keys) {
        const QVariant param = configurationParameter(key);
        const bool success = d->setConfigurationParameter(key, param);
        if (!success) {
            qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Cannot apply parameter: %d with value: %ls.",
                      key, qUtf16Printable(param.toString()));
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void VectorCanBackend::close()
{
    Q_D(VectorCanBackend);

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void VectorCanBackend::setConfigurationParameter(ConfigurationKey key, const QVariant &value)
{
    Q_D(VectorCanBackend);

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool VectorCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(VectorCanBackend);

    if (state() != QCanBusDevice::ConnectedState)
        return false;

    if (Q_UNLIKELY(!newData.isValid())) {
        setError(tr("Cannot write invalid QCanBusFrame"),
                 QCanBusDevice::WriteError);
        return false;
    }

    if (Q_UNLIKELY(newData.frameType() != QCanBusFrame::DataFrame
            && newData.frameType() != QCanBusFrame::RemoteRequestFrame
            && newData.frameType() != QCanBusFrame::ErrorFrame)) {
        setError(tr("Unable to write a frame with unacceptable type"),
                 QCanBusDevice::WriteError);
        return false;
    }

    if (!d->usesCanFd && newData.hasFlexibleDataRateFormat()) {
        setError(tr("Unable to write a flexible data rate format frame without CAN FD enabled."),
                 QCanBusDevice::WriteError);
        return false;
    }

    enqueueOutgoingFrame(newData);

    if (!d->writeNotifier->isActive())
        d->writeNotifier->start();

    return true;
}

// TODO: Implement me
QString VectorCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);

    return QString();
}

bool VectorCanBackend::hasBusStatus() const
{
    return true;
}

QCanBusDevice::CanBusStatus VectorCanBackend::busStatus()
{
    Q_D(VectorCanBackend);

    const XLstatus requestStatus = ::xlCanRequestChipState(d->portHandle, d->channelMask);
    if (Q_UNLIKELY(requestStatus != XL_SUCCESS)) {
        const QString errorString = d->systemErrorString(requestStatus);
        qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Can not query CAN bus status: %ls.",
                  qUtf16Printable(errorString));
        setError(errorString, QCanBusDevice::CanBusError::ReadError);
        return QCanBusDevice::CanBusStatus::Unknown;
    }

    quint8 busStatus = 0;
    if (d->usesCanFd) {
        XLcanRxEvent event = {};

        const XLstatus receiveStatus = ::xlCanReceive(d->portHandle, &event);
        if (Q_UNLIKELY(receiveStatus != XL_SUCCESS)) {
            const QString errorString = d->systemErrorString(receiveStatus);
            qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Can not query CAN bus status: %ls.",
                qUtf16Printable(errorString));
            setError(errorString, QCanBusDevice::CanBusError::ReadError);
            return QCanBusDevice::CanBusStatus::Unknown;
        }

        if (Q_LIKELY(event.tag == XL_CAN_EV_TAG_CHIP_STATE))
            busStatus = event.tagData.canChipState.busStatus;

    } else {
        quint32 eventCount = 1;
        XLevent event = {};

        const XLstatus receiveStatus = ::xlReceive(d->portHandle, &eventCount, &event);
        if (Q_UNLIKELY(receiveStatus != XL_SUCCESS)) {
            const QString errorString = d->systemErrorString(receiveStatus);
            qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Can not query CAN bus status: %ls.",
                qUtf16Printable(errorString));
            setError(errorString, QCanBusDevice::CanBusError::ReadError);
            return QCanBusDevice::CanBusStatus::Unknown;
        }

        if (Q_LIKELY(event.tag == XL_CHIP_STATE))
            busStatus = event.tagData.chipState.busStatus;
    }

    switch (busStatus) {
    case XL_CHIPSTAT_BUSOFF:
        return QCanBusDevice::CanBusStatus::BusOff;
    case XL_CHIPSTAT_ERROR_PASSIVE:
        return QCanBusDevice::CanBusStatus::Error;
    case XL_CHIPSTAT_ERROR_WARNING:
        return QCanBusDevice::CanBusStatus::Warning;
    case XL_CHIPSTAT_ERROR_ACTIVE:
        return QCanBusDevice::CanBusStatus::Good;
    }

    qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "Unknown CAN bus status: %u", busStatus);
    return QCanBusDevice::CanBusStatus::Unknown;
}

QCanBusDeviceInfo VectorCanBackend::deviceInfo() const
{
    const QList<QCanBusDeviceInfo> availableDevices = interfaces();
    const int index = d_ptr->channelIndex;
    const QString name = QStringLiteral("can%1").arg(index);

    const auto deviceInfo = std::find_if(availableDevices.constBegin(),
                                         availableDevices.constEnd(),
                                         [name](const QCanBusDeviceInfo &info) {
        return name == info.name();
    });

    if (Q_LIKELY(deviceInfo != availableDevices.constEnd()))
        return *deviceInfo;

    qWarning("%s: Cannot get device info for index %d.", Q_FUNC_INFO, index);
    return QCanBusDevice::deviceInfo();
}

QT_END_NAMESPACE
