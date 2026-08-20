// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwsensorbase.h"


SensorManagerInterface* SensorfwSensorBase::m_remoteSensorManager = 0;

//According to wikipedia link http://en.wikipedia.org/wiki/Standard_gravity
const float SensorfwSensorBase::GRAVITY_EARTH_THOUSANDTH = 0.00980665;
const int SensorfwSensorBase::KErrNotFound=-1;
const int SensorfwSensorBase::KErrInUse=-14;
QStringList SensorfwSensorBase::m_bufferingSensors = QStringList()
        <<"sensorfw.accelerometer"<<"sensorfw.magnetometer"
       <<"sensorfw.gyroscope"<<"sensorfw.rotationsensor";

SensorfwSensorBase::SensorfwSensorBase(QSensor *sensor)
    : QSensorBackend(sensor),
      m_sensorInterface(0),
      m_bufferSize(-1),
      reinitIsNeeded(false),
      m_prevOutputRange(0),
      m_efficientBufferSize(1),
      m_maxBufferSize(1),
      m_available(false),
      running(false),
      m_attemptRestart(false)

{
    watcher = new QDBusServiceWatcher("com.nokia.SensorService",QDBusConnection::systemBus(),
            QDBusServiceWatcher::WatchForRegistration |
            QDBusServiceWatcher::WatchForUnregistration, this);

    connect(watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(connectToSensord()));
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(sensordUnregistered()));

    connect(sensor, SIGNAL(alwaysOnChanged()),this,SLOT(standyOverrideChanged()));

    m_available = QDBusConnection::systemBus().interface()->isServiceRegistered("com.nokia.SensorService");
    if (m_available)
        connectToSensord();
}

SensorfwSensorBase::~SensorfwSensorBase()
{
    if (m_sensorInterface) {
        stop();
        delete m_sensorInterface, m_sensorInterface = 0;
    }
}

void SensorfwSensorBase::start()
{
    if (m_sensorInterface) {
        // dataRate
        QByteArray type = sensor()->type();
        if (type != QTapSensor::sensorType && type != QProximitySensor::sensorType) {
            int dataRate = sensor()->dataRate();
            int interval = dataRate > 0 ? 1000 / dataRate : 0;
            // for testing maximum speed
            //interval = 1;
            //dataRate = 1000;
            m_sensorInterface->setInterval(interval);
        }

        // outputRange
        int currentRange = sensor()->outputRange();
        int l = sensor()->outputRanges().size();
        if (l > 1) {
            if (currentRange != m_prevOutputRange) {
//#ifdef Q_WS_MAEMO_6
                bool isOk = m_sensorInterface->setDataRangeIndex(currentRange); //NOTE THAT THE CHANGE MIGHT NOT SUCCEED, FIRST COME FIRST SERVED
                if (!isOk) sensorError(KErrInUse);
                else m_prevOutputRange = currentRange;
//#else
//                // TODO: remove when sensord integrated, in sensorfw env there is a delay
//                qoutputrange range = sensor()->outputRanges().at(currentRange);
//                qreal correction = 1/correctionFactor();
//                DataRange range1(range.minimum*correction, range.maximum*correction, range.accuracy*correction);
//                m_sensorInterface->requestDataRange(range1);
//                m_prevOutputRange = currentRange;
//#endif
            }
        }

        // always on
        bool alwaysOn = sensor()->isAlwaysOn();
        m_sensorInterface->setStandbyOverride(alwaysOn);

        // connects after buffering checks
        doConnectAfterCheck();

        int returnCode = m_sensorInterface->start().error().type();
        if (returnCode == 0) {
            running = true;
            return;
        } else if (returnCode == QDBusError::ServiceUnknown) {
            m_attemptRestart = true;
            qWarning() << "m_sensorInterface did not start, DBus service unknown. Waiting for service registration and retrying.";
        } else {
            qWarning() << "m_sensorInterface did not start, error code:" << returnCode;
        }
    }
    sensorStopped();
}

void SensorfwSensorBase::stop()
{
    if (m_sensorInterface)
        m_sensorInterface->stop();
    running = false;
    m_attemptRestart = false;
}

void SensorfwSensorBase::setRanges(qreal correctionFactor)
{
    if (!m_sensorInterface) return;

    QList<DataRange> ranges = m_sensorInterface->getAvailableDataRanges();

    for (int i = 0, l = ranges.size(); i < l; i++) {
        DataRange range = ranges.at(i);
        qreal rangeMin = range.min * correctionFactor;
        qreal rangeMax = range.max * correctionFactor;
        qreal resolution = range.resolution * correctionFactor;
        addOutputRange(rangeMin, rangeMax, resolution);
    }
}


bool SensorfwSensorBase::doConnectAfterCheck()
{
    if (!m_sensorInterface) return false;

    // buffer size
    int size = bufferSize();

    if (size == m_bufferSize) return true;

    if (m_bufferingSensors.contains(sensor()->identifier()))
        m_sensorInterface->setBufferSize(size);
    else
        size = 1;

    // if multiple->single or single->multiple or if uninitialized
    if ((m_bufferSize > 1 && size == 1) || (m_bufferSize == 1 && size > 1) || m_bufferSize == -1) {
        m_bufferSize = size;
        disconnect(this);
        if (!doConnect()) {
            qWarning() << "Unable to connect "<< sensorName();
            return false;
        }
        return true;
    }
    m_bufferSize = size;
    return true;
}

int SensorfwSensorBase::bufferSize() const
{
    int bufferSize = sensor()->bufferSize();
    if (bufferSize == 1)
        return 1;

    // otherwise check validit
    if (bufferSize < 1) {
        qWarning() << "bufferSize cannot be " << bufferSize << ", must be a positive number >= 1";
        return 1;
    }
    if (bufferSize > m_maxBufferSize) {
        qWarning() << "bufferSize cannot be " << bufferSize << ", MAX value is " << m_maxBufferSize;
        return m_maxBufferSize;
    }
    return bufferSize;
}

qreal SensorfwSensorBase::correctionFactor() const
{
    return 1;
}

void SensorfwSensorBase::connectToSensord()
{
    m_remoteSensorManager = &SensorManagerInterface::instance();
    if (!m_remoteSensorManager->isValid()) {
        qWarning() << "SensorManagerInterface is invalid";
        m_remoteSensorManager = 0;
        return;
    }
    if (running || m_attemptRestart) {
        stop();
        reinitIsNeeded = true;
        start();
        reinitIsNeeded = false;
    }
}

void SensorfwSensorBase::sensordUnregistered()
{
    m_bufferSize = -1;
    reinitIsNeeded = true;
}

bool SensorfwSensorBase::initSensorInterface(QString const &name)
{
    if (!m_sensorInterface) {
        sensorError(KErrNotFound);
        return false;
    }

    //metadata
    const QList<DataRange> intervals = m_sensorInterface->getAvailableIntervals();

    for (int i = 0, l = intervals.size(); i < l; i++) {
        qreal intervalMax = intervals.at(i).max;
        qreal intervalMin = intervals.at(i).min;

        if (intervalMin == 0 && intervalMax == 0) {
            // 0 interval has different meanings in e.g. magge/acce
            // magge -> best-effort
            // acce -> lowest possible
            // in Qt API setting 0 means default
            continue;
        }

        qreal rateMin = intervalMax < 1 ? 1 : 1 / intervalMax * 1000;
        rateMin = rateMin < 1 ? 1 : rateMin;

        intervalMin = intervalMin < 1 ? 10: intervalMin;     // do not divide with 0
        qreal rateMax = 1 / intervalMin * 1000;
        addDataRate(rateMin, rateMax);
    }

    //bufferSizes
    if (m_bufferingSensors.contains(sensor()->identifier())) {

        IntegerRangeList sizes = m_sensorInterface->getAvailableBufferSizes();
        for (int i = 0; i < sizes.size(); i++) {
            int second = sizes.at(i).second;
            m_maxBufferSize = second > m_bufferSize ? second : m_maxBufferSize;
        }
        m_maxBufferSize = m_maxBufferSize < 0 ? 1 : m_maxBufferSize;
        //SensorFW guarantees to provide the most efficient size first
        //TODO: remove from comments
        //m_efficientBufferSize  = m_sensorInterface->hwBuffering()? (l>0?sizes.at(0).first:1) : 1;
    } else {
        m_maxBufferSize = 1;
    }

    sensor()->setMaxBufferSize(m_maxBufferSize);
    sensor()->setEfficientBufferSize(m_efficientBufferSize);

    // TODO deztructor: Leaking abstraction detected. Just copied code
    // from initSensor<>() here, need to
    QByteArray type = sensor()->type();
    if ((type == QAmbientLightSensor::sensorType) // SensorFW returns lux values, plugin enumerated values
        || (type == QIRProximitySensor::sensorType) // SensorFW returns raw reflectance values, plugin % of max reflectance
        || (name == "accelerometersensor") // SensorFW returns milliGs, plugin m/s^2
        || (name == "magnetometersensor") // SensorFW returns nanoTeslas, plugin Teslas
        || (name == "gyroscopesensor")) // SensorFW returns DSPs, plugin milliDSPs
        return true;

    setDescription(m_sensorInterface->description());

    if (name == "tapsensor") return true;
    setRanges();
    return true;
}

void SensorfwSensorBase::standyOverrideChanged()
{
    if (m_sensorInterface)
        m_sensorInterface->setStandbyOverride(sensor()->isAlwaysOn());
}

bool SensorfwSensorBase::isFeatureSupported(QSensor::Feature feature) const
{
    switch (feature) {
    case QSensor::AlwaysOn:
        return true;
    case QSensor::AxesOrientation:
    case QSensor::Buffering:
    case QSensor::AccelerationMode:
    case QSensor::SkipDuplicates:
    case QSensor::PressureSensorTemperature:
    case QSensor::GeoValues:
    case QSensor::Reserved:
    case QSensor::FieldOfView:
        return false;
        break;
    };

    return false;
}
