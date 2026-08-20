// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qgeopositioninfosourcefactory_nmea.h"
#include <QtPositioning/QNmeaPositionInfoSource>
#include <QtPositioning/QNmeaSatelliteInfoSource>
#include <QtNetwork/QTcpSocket>
#include <QLoggingCategory>
#include <QSet>
#include <QUrl>
#include <QFile>
#include <QSharedPointer>
#include "qiopipe_p.h"

#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
#  include <QtSerialPort/QSerialPort>
#  include <QtSerialPort/QSerialPortInfo>
#endif

#include <memory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcNmea, "qt.positioning.nmea")

static const auto sourceParameterName = QStringLiteral("nmea.source");
static const auto socketScheme = QStringLiteral("socket:");
static const auto serialScheme = QStringLiteral("serial:");

static const auto baudRateParameterName = QStringLiteral("nmea.baudrate");
static constexpr auto defaultBaudRate = 4800;

#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT

// This class is used only for SerialPort devices, because we can't open the
// same serial port twice.
// In case of files and sockets it's easier to explicitly create a QIODevice for
// each new instance of Nmea*InfoSource.
// Also QFile can't be directly used with QIOPipe, because QFile is not a
// sequential device.
// TcpSocket could be used with QIOPipe, but it complicates error handling
// dramatically, as we would need to somehow forward socket errors through
// QIOPipes to the clients.
class IODeviceContainer
{
public:
    IODeviceContainer() {}
    IODeviceContainer(IODeviceContainer const&) = delete;
    void operator=(IODeviceContainer const&)  = delete;

    QSharedPointer<QIOPipe> serial(const QString &portName, qint32 baudRate)
    {
        if (m_serialPorts.contains(portName)) {
            m_serialPorts[portName].refs++;
            QIOPipe *endPipe = new QIOPipe(m_serialPorts[portName].proxy);
            m_serialPorts[portName].proxy->addChildPipe(endPipe);
            return QSharedPointer<QIOPipe>(endPipe);
        }
        IODevice device;
        QSerialPort *port = new QSerialPort(portName);
        port->setBaudRate(baudRate);
        qCDebug(lcNmea) << "Opening serial port" << portName << "with baudrate" << baudRate;
        if (!port->open(QIODevice::ReadOnly)) {
            qWarning("nmea: Failed to open %s", qPrintable(portName));
            delete port;
            return {};
        }
        qCDebug(lcNmea) << "Opened successfully";
        device.device = port;
        device.refs = 1;
        device.proxy = new QIOPipe(port, QIOPipe::ProxyPipe);
        m_serialPorts[portName] = device;
        QIOPipe *endPipe = new QIOPipe(device.proxy);
        device.proxy->addChildPipe(endPipe);
        return QSharedPointer<QIOPipe>(endPipe);
    }

    void releaseSerial(const QString &portName, QSharedPointer<QIOPipe> &pipe)
    {
        if (!m_serialPorts.contains(portName))
            return;

        pipe.clear(); // make sure to release the pipe returned by getSerial, or else, if there are still refs, data will be leaked through it
        IODevice &device = m_serialPorts[portName];
        if (device.refs > 1) {
            device.refs--;
            return;
        }

        IODevice taken = m_serialPorts.take(portName);
        taken.device->deleteLater();
    }

private:

    struct IODevice {
        QIODevice *device = nullptr;
        QIOPipe *proxy = nullptr; // adding client pipes as children of proxy
                                  // allows to dynamically add clients to one device.
        unsigned int refs = 1;
    };

    QMap<QString, IODevice> m_serialPorts;
};

Q_GLOBAL_STATIC(IODeviceContainer, deviceContainer)

#endif // QT_NMEA_PLUGIN_HAS_SERIALPORT

struct NmeaParameters
{
    explicit NmeaParameters(const QVariantMap &parameters);

    QString source;
    qint32 baudRate = defaultBaudRate;
};

NmeaParameters::NmeaParameters(const QVariantMap &parameters)
{
    source = parameters.value(sourceParameterName).toString();
    bool ok = false;
    const auto br = parameters.value(baudRateParameterName).toInt(&ok);
    // According to QSerialPort::setBaudRate() documentation, we can pick any
    // positive number as a baud rate.
    if (ok && br > 0)
        baudRate = br;
}

// We use a string prefix to distinguish between the different data sources.
// "socket:" means that we use a socket connection
// "serial:" means that we use a serial port connection
// "file:///", "qrc:///" and just plain strings mean that we try to use local
// file.
// Note: if we do not specify anything, or specify "serial:" without specifying
// the port name, then we will try to search for a well-known serial port
// device.
class NmeaSource : public QNmeaPositionInfoSource
{
    Q_OBJECT
public:
    NmeaSource(QObject *parent, const QVariantMap &parameters);
    NmeaSource(QObject *parent, const QString &fileName);
    ~NmeaSource() override;
    bool isValid() const
    {
        return !m_dataSource.isNull() || m_fileSource || m_socket;
    }

private slots:
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void processParameters(const NmeaParameters &parameters);
    void addSerialDevice(const QString &requestedPort, quint32 baudRate);
    void setFileName(const QString &fileName);
    void connectSocket(const QString &source);

    QSharedPointer<QIOPipe> m_dataSource;
    std::unique_ptr<QFile> m_fileSource;
    std::unique_ptr<QTcpSocket> m_socket;
    QString m_sourceName;
};

NmeaSource::NmeaSource(QObject *parent, const QVariantMap &parameters)
    : QNmeaPositionInfoSource(RealTimeMode, parent)
{
    processParameters(NmeaParameters(parameters));
}

NmeaSource::NmeaSource(QObject *parent, const QString &fileName)
    : QNmeaPositionInfoSource(SimulationMode, parent)
{
    setFileName(fileName);
}

NmeaSource::~NmeaSource()
{
#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
    if (deviceContainer.exists())
        deviceContainer->releaseSerial(m_sourceName, m_dataSource);
#endif
}

void NmeaSource::onSocketError(QAbstractSocket::SocketError error)
{
    m_socket->close();

    switch (error) {
    case QAbstractSocket::UnknownSocketError:
        setError(QGeoPositionInfoSource::UnknownSourceError);
        break;
    case QAbstractSocket::SocketAccessError:
        setError(QGeoPositionInfoSource::AccessError);
        break;
    case QAbstractSocket::RemoteHostClosedError:
        setError(QGeoPositionInfoSource::ClosedError);
        break;
    default:
        qWarning() << "Connection failed! QAbstractSocket::SocketError" << error;
        // TODO - introduce new type of error. TransportError?
        setError(QGeoPositionInfoSource::UnknownSourceError);
        break;
    }
}

void NmeaSource::processParameters(const NmeaParameters &parameters)
{
    if (parameters.source.startsWith(socketScheme)) {
        // This is a socket
        connectSocket(parameters.source);
    } else {
        // Last chance - this can be serial device.
        // Note: File is handled in a separate case.
        addSerialDevice(parameters.source, parameters.baudRate);
    }
}

#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
static QString tryFindSerialDevice(const QString &requestedPort)
{
    QString portName;
    if (requestedPort.isEmpty()) {
        const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
        qCDebug(lcNmea) << "Found" << ports.size() << "serial ports";
        if (ports.isEmpty()) {
            qWarning("nmea: No serial ports found");
            return portName;
        }

        // Try to find a well-known device.
        QSet<int> supportedDevices;
        supportedDevices << 0x67b; // GlobalSat (BU-353S4 and probably others)
        supportedDevices << 0xe8d; // Qstarz MTK II
        for (const QSerialPortInfo& port : ports) {
            if (port.hasVendorIdentifier() && supportedDevices.contains(port.vendorIdentifier())) {
                portName = port.portName();
                break;
            }
        }

        if (portName.isEmpty()) {
            qWarning("nmea: No known GPS device found.");
        }
    } else {
        portName = requestedPort;
        if (portName.startsWith(serialScheme))
            portName.remove(0, 7);
    }
    return portName;
}
#endif // QT_NMEA_PLUGIN_HAS_SERIALPORT

void NmeaSource::addSerialDevice(const QString &requestedPort, quint32 baudRate)
{
#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
    m_sourceName = tryFindSerialDevice(requestedPort);
    if (m_sourceName.isEmpty())
        return;

    m_dataSource = deviceContainer->serial(m_sourceName, baudRate);
    if (!m_dataSource)
        return;

    setDevice(m_dataSource.data());
#else
    Q_UNUSED(baudRate);
    // As we are not calling setDevice(), the source will be invalid, so
    // the factory methods will return nullptr.
    qWarning() << "Plugin was built without serialport support!"
               << requestedPort << "cannot be used!";
#endif
}

void NmeaSource::setFileName(const QString &fileName)
{
    m_sourceName = fileName;

    m_fileSource.reset(new QFile(fileName));
    qCDebug(lcNmea) << "Opening file" << fileName;
    if (!m_fileSource->open(QIODevice::ReadOnly)) {
        qWarning("nmea: failed to open file %s", qPrintable(fileName));
        m_fileSource.reset();
    }

    if (!m_fileSource)
        return;

    qCDebug(lcNmea) << "Opened successfully";

    setDevice(m_fileSource.get());
}

void NmeaSource::connectSocket(const QString &source)
{
    const QUrl url(source);
    const QString host = url.host();
    const int port = url.port();
    if (!host.isEmpty() && (port > 0)) {
        m_socket.reset(new QTcpSocket);
        // no need to explicitly connect to connected() signal
        connect(m_socket.get(), &QTcpSocket::errorOccurred, this, &NmeaSource::onSocketError);
        m_socket->connectToHost(host, port, QTcpSocket::ReadOnly);
        m_sourceName = source;

        setDevice(m_socket.get());
    } else {
        qWarning("nmea: incorrect socket parameters %s:%d", qPrintable(host), port);
    }
}

class NmeaSatelliteSource : public QNmeaSatelliteInfoSource
{
    Q_OBJECT
public:
    NmeaSatelliteSource(QObject *parent, const QVariantMap &parameters);
    NmeaSatelliteSource(QObject *parent, const QString &fileName, const QVariantMap &parameters);
    ~NmeaSatelliteSource();

    bool isValid() const { return !m_port.isNull() || m_file || m_socket; }

private slots:
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void processRealtimeParameters(const NmeaParameters &parameters);
    void parseSimulationSource(const QString &localFileName);

    QSharedPointer<QIOPipe> m_port;
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTcpSocket> m_socket;
    QString m_sourceName;
};

NmeaSatelliteSource::NmeaSatelliteSource(QObject *parent, const QVariantMap &parameters)
    : QNmeaSatelliteInfoSource(QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode, parent)
{
    processRealtimeParameters(NmeaParameters(parameters));
}

// We can use a QNmeaSatelliteInfoSource::SimulationUpdateInterval parameter to
// set the file read frequency in simulation mode. We use setBackendProperty()
// for it. The value can't be smaller than minimumUpdateInterval().
// This check is done on the QNmeaSatelliteInfoSource level
NmeaSatelliteSource::NmeaSatelliteSource(QObject *parent, const QString &fileName,
                                         const QVariantMap &parameters)
    : QNmeaSatelliteInfoSource(QNmeaSatelliteInfoSource::UpdateMode::SimulationMode, parent)
{
    bool ok = false;
    const int interval =
            parameters.value(QNmeaSatelliteInfoSource::SimulationUpdateInterval).toInt(&ok);
    if (ok)
        setBackendProperty(QNmeaSatelliteInfoSource::SimulationUpdateInterval, interval);
    parseSimulationSource(fileName);
}

NmeaSatelliteSource::~NmeaSatelliteSource()
{
#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
    if (deviceContainer.exists())
        deviceContainer->releaseSerial(m_sourceName, m_port);
#endif
}

void NmeaSatelliteSource::onSocketError(QAbstractSocket::SocketError error)
{
    m_socket->close();

    switch (error) {
    case QAbstractSocket::UnknownSocketError:
        setError(QGeoSatelliteInfoSource::UnknownSourceError);
        break;
    case QAbstractSocket::SocketAccessError:
        setError(QGeoSatelliteInfoSource::AccessError);
        break;
    case QAbstractSocket::RemoteHostClosedError:
        setError(QGeoSatelliteInfoSource::ClosedError);
        break;
    default:
        qWarning() << "Connection failed! QAbstractSocket::SocketError" << error;
        // TODO - introduce new type of error. TransportError?
        setError(QGeoSatelliteInfoSource::UnknownSourceError);
        break;
    }
}

void NmeaSatelliteSource::processRealtimeParameters(const NmeaParameters &parameters)
{
    const QString source = parameters.source;
    if (source.startsWith(socketScheme)) {
        // This is a socket.
        const QUrl url(source);
        const QString host = url.host();
        const int port = url.port();
        if (!host.isEmpty() && (port > 0)) {
            m_socket.reset(new QTcpSocket);
            // no need to explicitly connect to connected() signal
            connect(m_socket.get(), &QTcpSocket::errorOccurred,
                    this, &NmeaSatelliteSource::onSocketError);
            m_socket->connectToHost(host, port, QTcpSocket::ReadOnly);
            m_sourceName = source;

            setDevice(m_socket.get());
        } else {
            qWarning("nmea: incorrect socket parameters %s:%d", qPrintable(host), port);
        }
    } else {
#ifdef QT_NMEA_PLUGIN_HAS_SERIALPORT
        // Last chance - this can be serial device.
        m_sourceName = tryFindSerialDevice(source);
        if (m_sourceName.isEmpty())
            return;

        m_port = deviceContainer->serial(m_sourceName, parameters.baudRate);
        if (!m_port)
            return;

        setDevice(m_port.data());
#else
        // As we are not calling setDevice(), the source will be invalid, so
        // the factory methods will return nullptr.
        qWarning() << "Plugin was built without serialport support!"
                   << source << "cannot be used!";
#endif // QT_NMEA_PLUGIN_HAS_SERIALPORT
    }
}

void NmeaSatelliteSource::parseSimulationSource(const QString &localFileName)
{
    // This is a text file.
    m_sourceName = localFileName;

    qCDebug(lcNmea) << "Opening file" << localFileName;
    m_file.reset(new QFile(localFileName));
    if (!m_file->open(QIODevice::ReadOnly)) {
        qWarning("nmea: failed to open file %s", qPrintable(localFileName));
        m_file.reset();
        return;
    }
    qCDebug(lcNmea) << "Opened successfully";

    setDevice(m_file.get());
}

/*!
    \internal
    Returns a local file name if \a source represents it.
    The returned value can be different from \a source, as the method tries to
    modify the path
*/
static QString checkSourceIsFile(const QString &source)
{
    if (source.isEmpty())
        return QString();

    QString localFileName = source;

    if (!QFile::exists(localFileName)) {
        if (localFileName.startsWith(QStringLiteral("qrc:///")))
            localFileName.remove(0, 7);
        else if (localFileName.startsWith(QStringLiteral("file:///")))
            localFileName.remove(0, 7);
        else if (localFileName.startsWith(QStringLiteral("qrc:/")))
            localFileName.remove(0, 5);

        if (!QFile::exists(localFileName) && localFileName.startsWith(QLatin1Char('/')))
            localFileName.remove(0, 1);
    }
    if (!QFile::exists(localFileName))
        localFileName.prepend(QLatin1Char(':'));

    const bool isLocalFile = QFile::exists(localFileName);
    return isLocalFile ? localFileName : QString();
}

/*!
    \internal
    Returns a local file name if file exists, or an empty string otherwise
*/
static QString extractLocalFileName(const QVariantMap &parameters)
{
    QString localFileName = parameters.value(sourceParameterName).toString();
    return checkSourceIsFile(localFileName);
}

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryNmea::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    std::unique_ptr<NmeaSource> src = nullptr;

    const QString localFileName = extractLocalFileName(parameters);
    if (localFileName.isEmpty())
        src = std::make_unique<NmeaSource>(parent, parameters); // use RealTimeMode
    else
        src = std::make_unique<NmeaSource>(parent, localFileName); // use SimulationMode

    return (src && src->isValid()) ? src.release() : nullptr;
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryNmea::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    std::unique_ptr<NmeaSatelliteSource> src = nullptr;

    const QString localFileName = extractLocalFileName(parameters);
    if (localFileName.isEmpty()) {
        // use RealTimeMode
        src = std::make_unique<NmeaSatelliteSource>(parent, parameters);
    } else {
        // use SimulationMode
        src = std::make_unique<NmeaSatelliteSource>(parent, localFileName, parameters);
    }
    return (src && src->isValid()) ? src.release() : nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryNmea::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent);
    Q_UNUSED(parameters);
    return nullptr;
}

QT_END_NAMESPACE

#include "moc_qgeopositioninfosourcefactory_nmea.cpp"
#include "qgeopositioninfosourcefactory_nmea.moc"
