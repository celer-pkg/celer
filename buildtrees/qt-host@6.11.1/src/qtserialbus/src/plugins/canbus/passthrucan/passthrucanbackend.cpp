// Copyright (C) 2017 Ford Motor Company.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "passthrucanbackend.h"
#include "passthrucanio.h"

#include <QEventLoop>
#include <QSettings>

namespace {

#ifdef Q_OS_WIN32

static inline QString registryPath()
{
    return QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\PassThruSupport.04.04");
}

static QString canAdapterName(const QSettings &entries)
{
    const int supportsCan = entries.value(QStringLiteral("CAN")).toInt();
    if (supportsCan)
        return entries.value(QStringLiteral("Name")).toString();
    return {};
}

static QString libraryForAdapter(const QString &adapterName)
{
    QString library;
    QSettings entries (registryPath(), QSettings::NativeFormat);
    const QStringList groups = entries.childGroups();

    for (const auto &group : groups) {
        entries.beginGroup(group);

        const QString name = canAdapterName(entries);
        if (!name.isEmpty() && (adapterName.isEmpty() ||
                                name.compare(adapterName, Qt::CaseInsensitive) == 0))
            library = entries.value(QStringLiteral("FunctionLibrary")).toString();

        entries.endGroup();

        if (!library.isEmpty())
            break;
    }
    return library;
}

#else // !Q_OS_WIN32

static QString libraryForAdapter(const QString &adapterName)
{
    // Insert system-specific device name to J2534 library name mapping here.
    // For now, allow the path to the J2534 library to be specified directly
    // as the adapter name.
    return adapterName;
}

#endif // !Q_OS_WIN32

} // anonymous namespace

PassThruCanBackend::PassThruCanBackend(const QString &name, QObject *parent)
    : QCanBusDevice(parent)
    , m_deviceName (name)
    , m_canIO (new PassThruCanIO())
{
    m_canIO->moveToThread(&m_ioThread);

    // Signals emitted by the I/O thread, to be queued.
    connect(m_canIO, &PassThruCanIO::errorOccurred,
            this, &PassThruCanBackend::setError);
    connect(m_canIO, &PassThruCanIO::openFinished,
            this, &PassThruCanBackend::ackOpenFinished);
    connect(m_canIO, &PassThruCanIO::closeFinished,
            this, &PassThruCanBackend::ackCloseFinished);
    connect(m_canIO, &PassThruCanIO::messagesReceived,
            this, &PassThruCanBackend::enqueueReceivedFrames);
    connect(m_canIO, &PassThruCanIO::messagesSent,
            this, &QCanBusDevice::framesWritten);
}

PassThruCanBackend::~PassThruCanBackend()
{
    if (state() != UnconnectedState) {
        // If the I/O thread is still active at this point, we will have to
        // wait for it to finish.
        QEventLoop loop;
        connect(&m_ioThread, &QThread::finished, &loop, &QEventLoop::quit);

        if (state() != ClosingState)
            disconnectDevice();

        while (!m_ioThread.isFinished())
            loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    m_canIO->deleteLater();
}

void PassThruCanBackend::setConfigurationParameter(ConfigurationKey key, const QVariant &value)
{
    QCanBusDevice::setConfigurationParameter(key, value);

    if (state() == ConnectedState)
        applyConfig(key, value);
}

bool PassThruCanBackend::writeFrame(const QCanBusFrame &frame)
{
    if (state() != ConnectedState) {
        setError(tr("Device is not connected"), WriteError);
        return false;
    }
    if (!frame.isValid()) {
        setError(tr("Invalid CAN bus frame"), WriteError);
        return false;
    }
    if (frame.frameType() != QCanBusFrame::DataFrame) {
        setError(tr("Unsupported CAN frame type"), WriteError);
        return false;
    }
    // Push the frame directly to the write queue of the worker thread,
    // bypassing the QCanBusDevice output queue. Despite the duplicated
    // queue, things are cleaner this way as it avoids a reverse dependency
    // from the worker object on the QCanBusDevice object.
    return m_canIO->enqueueMessage(frame);
}

QString PassThruCanBackend::interpretErrorFrame(const QCanBusFrame &)
{
    // J2534 Pass-thru v04.04 does not seem to support error frames.
    return {};
}

QList<QCanBusDeviceInfo> PassThruCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> list;
#ifdef Q_OS_WIN32
    QSettings entries (registryPath(), QSettings::NativeFormat);
    const QStringList groups = entries.childGroups();

    for (const auto &group : groups) {
        entries.beginGroup(group);

        const QString name = canAdapterName(entries);
        if (!name.isEmpty())
            list.append(createDeviceInfo(QStringLiteral("passthrucan"), name, false, false));
        entries.endGroup();
    }
#endif
    return list;
}

QCanBusDeviceInfo PassThruCanBackend::deviceInfo() const
{
    return createDeviceInfo(QStringLiteral("passthrucan"), m_deviceName, false, false);
}

bool PassThruCanBackend::open()
{
    if (Q_UNLIKELY(state() != ConnectingState)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Unexpected state on open");
        return false;
    }
    // Support a special "adapter%subdevice" syntax to allow control of the
    // device name passed to the J2534 library's PassThruOpen() function.
    // If the "%subdevice" suffix is not used, the J2534 interface library
    // will choose a default or ask the user.
    const int splitPos = m_deviceName.indexOf(QChar::fromLatin1('%'));
    const QString adapter = m_deviceName.left(splitPos);
    QByteArray subDev;

    if (splitPos >= 0)
      subDev = QStringView{m_deviceName}.mid(splitPos + 1).toLatin1();

    const QString library = libraryForAdapter(adapter);
    if (library.isEmpty()) {
        setError(tr("Adapter not found: %1").arg(adapter), ConnectionError);
        return false;
    }
    bool ok = false;
    uint bitRate = configurationParameter(BitRateKey).toUInt(&ok);
    if (!ok) {
        bitRate = 500*1000; // default initial bit rate
        setConfigurationParameter(BitRateKey, bitRate);
    }
    m_ioThread.start();

    return QMetaObject::invokeMethod(m_canIO, [this, library, subDev, bitRate] {
                                        m_canIO->open(library, subDev, bitRate);
                                     }, Qt::QueuedConnection);
}

void PassThruCanBackend::close()
{
    if (Q_UNLIKELY(state() != ClosingState)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Unexpected state on close");
        return;
    }
    QMetaObject::invokeMethod(m_canIO, &PassThruCanIO::close, Qt::QueuedConnection);
}

void PassThruCanBackend::ackOpenFinished(bool success)
{
    // Do not transition to connected state if close() has been called
    // in the meantime.
    if (state() != ConnectingState)
        return;

    if (success) {
        const QVariant loopback = configurationParameter(LoopbackKey);
        if (loopback.toBool())
            applyConfig(LoopbackKey, loopback);

        QVariant filters = configurationParameter(RawFilterKey);
        if (!filters.isValid()) {
            // Configure default match-all filter.
            filters = QVariant::fromValue(QList<Filter>{Filter{}});
            setConfigurationParameter(RawFilterKey, filters);
        }
        applyConfig(RawFilterKey, filters);

        QMetaObject::invokeMethod(m_canIO, &PassThruCanIO::listen, Qt::QueuedConnection);

        setState(ConnectedState);
    } else {
        setState(UnconnectedState);
    }
}

void PassThruCanBackend::ackCloseFinished()
{
    m_ioThread.exit(0);
    m_ioThread.wait();

    setState(UnconnectedState);
}

void PassThruCanBackend::applyConfig(QCanBusDevice::ConfigurationKey key, const QVariant &value)
{
    QMetaObject::invokeMethod(m_canIO,
                              [this, key, value] { m_canIO->applyConfig(key, value); },
                              Qt::QueuedConnection);
}
