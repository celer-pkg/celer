// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlsensor_p.h"
#include <QtSensors/QSensor>
#include <QDebug>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QmlSensorPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlSensor)
public:

    QList<QmlSensorRange *> availableRanges;
    QList<QmlSensorOutputRange *> outputRanges;
};

template<typename Item>
qsizetype readonlyListCount(QQmlListProperty<Item> *p)
{
    return static_cast<const QList<Item *> *>(p->data)->size();
}

template<typename Item>
Item *readonlyListAt(QQmlListProperty<Item> *p, qsizetype idx)
{
    return static_cast<const QList<Item *> *>(p->data)->at(idx);
};

template<typename Item>
QQmlListProperty<Item> readonlyListProperty(const QObject *o, const QList<Item *> *list)
{
    // Unfortunately QQmlListProperty won't accept a const object, even on the readonly ctor.
    return QQmlListProperty<Item>(const_cast<QObject *>(o), const_cast<QList<Item *> *>(list),
                                  readonlyListCount<Item>, readonlyListAt<Item>);
}

/*!
    \qmltype Sensor
//!    \nativetype QmlSensor
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \brief The Sensor element serves as a base type for sensors.

    The Sensor element serves as a base type for sensors.

    This element wraps the QSensor class. Please see the documentation for
    QSensor for details.

    This element cannot be directly created. Please use one of the sub-classes instead.
*/

QmlSensor::QmlSensor(QObject *parent)
    : QObject(*(new QmlSensorPrivate), parent)
{
}

QmlSensor::~QmlSensor()
{
}

/*!
    \qmlproperty string Sensor::identifier
    This property holds the backend identifier for the sensor.

    Please see QSensor::identifier for information about this property.
*/

QByteArray QmlSensor::identifier() const
{
    return sensor()->identifier();
}

void QmlSensor::setIdentifier(const QByteArray &identifier)
{
    sensor()->setIdentifier(identifier);
}

/*!
    \qmlproperty string Sensor::type
    This property holds the type of the sensor.
*/

QByteArray QmlSensor::type() const
{
    return sensor()->type();
}

/*!
    \qmlproperty bool Sensor::connectedToBackend
    This property holds a value indicating if the sensor has connected to a backend.

    Please see QSensor::connectedToBackend for information about this property.
*/

bool QmlSensor::isConnectedToBackend() const
{
    return sensor()->isConnectedToBackend();
}

/*!
    \qmlproperty bool Sensor::busy
    This property holds a value to indicate if the sensor is busy.

    Please see QSensor::busy for information about this property.
*/

bool QmlSensor::isBusy() const
{
    return sensor()->isBusy();
}

/*!
    \qmlproperty bool Sensor::active
    This property holds a value to indicate if the sensor is active.

    Please see QSensor::active for information about this property.
*/

void QmlSensor::setActive(bool active)
{
    if (!m_componentComplete) {
        m_activateOnComplete = active;
        return;
    }
    if (active)
        sensor()->start();
    else
        sensor()->stop();
}

bool QmlSensor::isActive() const
{
    return sensor()->isActive();
}

/*!
    \qmlproperty bool Sensor::alwaysOn
    This property holds a value to indicate if the sensor should remain running when the screen is off.

    Please see QSensor::alwaysOn for information about this property.
*/

bool QmlSensor::isAlwaysOn() const
{
    return sensor()->isAlwaysOn();
}

void QmlSensor::setAlwaysOn(bool alwaysOn)
{
    sensor()->setAlwaysOn(alwaysOn);
}

/*!
    \qmlproperty bool Sensor::skipDuplicates
    \since QtSensors 5.1

    This property indicates whether duplicate reading values should be omitted.

    Please see QSensor::skipDuplicates for information about this property.
*/

bool QmlSensor::skipDuplicates() const
{
    return sensor()->skipDuplicates();
}

void QmlSensor::setSkipDuplicates(bool skipDuplicates)
{
    sensor()->setSkipDuplicates(skipDuplicates);
}

/*!
    \qmlproperty list<Range> Sensor::availableDataRates
    This property holds the data rates that the sensor supports.

    Please see QSensor::availableDataRates for information about this property.
*/
QQmlListProperty<QmlSensorRange> QmlSensor::availableDataRates() const
{
    Q_D(const QmlSensor);
    return readonlyListProperty<QmlSensorRange>(this, &d->availableRanges);
}

/*!
    \qmlproperty int Sensor::dataRate
    This property holds the data rate that the sensor should be run at.

    Please see QSensor::dataRate for information about this property.
*/

int QmlSensor::dataRate() const
{
    return sensor()->dataRate();
}

void QmlSensor::setDataRate(int rate)
{
    if (rate != dataRate()) {
      sensor()->setDataRate(rate);
      Q_EMIT dataRateChanged();
    }
}

/*!
    \qmlproperty list<OutputRange> Sensor::outputRanges
    This property holds a list of output ranges the sensor supports.

    Please see QSensor::outputRanges for information about this property.
*/

QQmlListProperty<QmlSensorOutputRange> QmlSensor::outputRanges() const
{
    Q_D(const QmlSensor);
    return readonlyListProperty<QmlSensorOutputRange>(this, &d->outputRanges);
}

/*!
    \qmlproperty int Sensor::outputRange
    This property holds the output range in use by the sensor.

    Please see QSensor::outputRange for information about this property.
*/

int QmlSensor::outputRange() const
{
    return sensor()->outputRange();
}

void QmlSensor::setOutputRange(int index)
{
    int oldRange = outputRange();
    if (oldRange == index) return;
    sensor()->setOutputRange(index);
    if (sensor()->outputRange() == index)
        Q_EMIT outputRangeChanged();
}

/*!
    \qmlproperty string Sensor::description
    This property holds a descriptive string for the sensor.
*/

QString QmlSensor::description() const
{
    return sensor()->description();
}

/*!
    \qmlproperty int Sensor::error
    This property holds the last error code set on the sensor.
*/

int QmlSensor::error() const
{
    return sensor()->error();
}

/*!
    \qmlproperty SensorReading Sensor::reading
    This property holds the reading class.

    Please see QSensor::reading for information about this property.
    \sa {QML Reading types}
*/

QmlSensorReading *QmlSensor::reading() const
{
    return m_reading;
}

QBindable<QmlSensorReading*> QmlSensor::bindableReading() const
{
    return &m_reading;
}

/*!
    \qmlmethod bool Sensor::isFeatureSupported(feature)
    \since QtSensors 6.7
    Checks if a specific feature is supported by the backend.
    Returns \c true if the \a feature is supported, and \c false otherwise.
    For feature descriptions see \l {QSensor::Feature}.

    Please see QSensor::isFeatureSupported for information.
*/

bool QmlSensor::isFeatureSupported(Feature feature) const
{
    return sensor()->isFeatureSupported(static_cast<QSensor::Feature>(feature));
}

/*!
    \qmlproperty Sensor::AxesOrientationMode Sensor::axesOrientationMode
    \since QtSensors 5.1
    This property holds the mode that affects how the screen orientation changes reading values.

    Please see QSensor::axesOrientationMode for information about this property.
*/

QmlSensor::AxesOrientationMode QmlSensor::axesOrientationMode() const
{
    return static_cast<QmlSensor::AxesOrientationMode>(sensor()->axesOrientationMode());
}

void QmlSensor::setAxesOrientationMode(QmlSensor::AxesOrientationMode axesOrientationMode)
{
    sensor()->setAxesOrientationMode(static_cast<QSensor::AxesOrientationMode>(axesOrientationMode));
}

/*!
    \qmlproperty int Sensor::currentOrientation
    \since QtSensors 5.1
    This property holds the current orientation that is used for rotating the reading values.

    Please see QSensor::currentOrientation for information about this property.
*/

int QmlSensor::currentOrientation() const
{
    return sensor()->currentOrientation();
}

/*!
    \qmlproperty int Sensor::userOrientation
    \since QtSensors 5.1
    This property holds the angle used for rotating the reading values in the UserOrientation mode.

    Please see QSensor::userOrientation for information about this property.
*/

int QmlSensor::userOrientation() const
{
    return sensor()->userOrientation();
}

void QmlSensor::setUserOrientation(int userOrientation)
{
    sensor()->setUserOrientation(userOrientation);
}

/*!
    \qmlproperty int Sensor::maxBufferSize
    \since QtSensors 5.1
    This property holds the maximum buffer size.

    Please see QSensor::maxBufferSize for information about this property.
*/

int QmlSensor::maxBufferSize() const
{
    return sensor()->maxBufferSize();
}

/*!
    \qmlproperty int Sensor::efficientBufferSize
    \since QtSensors 5.1
    The property holds the most efficient buffer size.

    Please see QSensor::efficientBufferSize for information about this property.
*/

int QmlSensor::efficientBufferSize() const
{
    return sensor()->efficientBufferSize();
}

/*!
    \qmlproperty int Sensor::bufferSize
    \since QtSensors 5.1
    This property holds the size of the buffer.

    Please see QSensor::bufferSize for information about this property.
*/

int QmlSensor::bufferSize() const
{
    return sensor()->bufferSize();
}

void QmlSensor::setBufferSize(int bufferSize)
{
    sensor()->setBufferSize(bufferSize);
}

/*!
    \qmlmethod bool Sensor::start()
    Start retrieving values from the sensor. Returns true if the sensor
    was started, false otherwise.

    Please see QSensor::start() for information.
*/

bool QmlSensor::start()
{
    return sensor()->start();
}

/*!
    \qmlmethod bool Sensor::stop()
    Stop retrieving values from the sensor.
    Returns true if the sensor was stopped, false otherwise.

    Please see QSensor::stop() for information.
*/

void QmlSensor::stop()
{
    setActive(false);
}

void QmlSensor::classBegin()
{
}

void QmlSensor::componentComplete()
{
    m_componentComplete = true;

    connect(sensor(), SIGNAL(sensorError(int)), this, SIGNAL(errorChanged()));
    connect(sensor(), SIGNAL(activeChanged()), this, SIGNAL(activeChanged()));
    connect(sensor(), SIGNAL(alwaysOnChanged()), this, SIGNAL(alwaysOnChanged()));
    connect(sensor(), SIGNAL(skipDuplicatesChanged(bool)), this, SIGNAL(skipDuplicatesChanged(bool)));
    connect(sensor(), SIGNAL(axesOrientationModeChanged(AxesOrientationMode)),
            this, SIGNAL(axesOrientationModeChanged(AxesOrientationMode)));
    connect(sensor(), SIGNAL(userOrientationChanged(int)), this, SIGNAL(userOrientationChanged(int)));
    connect(sensor(), SIGNAL(currentOrientationChanged(int)), this, SIGNAL(currentOrientationChanged(int)));
    connect(sensor(), SIGNAL(bufferSizeChanged(int)), this, SIGNAL(bufferSizeChanged(int)));
    connect(sensor(), SIGNAL(maxBufferSizeChanged(int)), this, SIGNAL(maxBufferSizeChanged(int)));
    connect(sensor(), SIGNAL(efficientBufferSizeChanged(int)), this, SIGNAL(efficientBufferSizeChanged(int)));
    connect(sensor(), &QSensor::busyChanged, this, &QmlSensor::busyChanged);
    connect(sensor(), &QSensor::identifierChanged, this, &QmlSensor::identifierChanged);

    // These can change!
    int oldDataRate = dataRate();
    int oldOutputRange = outputRange();

    if (sensor()->connectToBackend())
        Q_EMIT connectedToBackendChanged();

    m_reading.setValueBypassingBindings(createReading());
    m_reading->setParent(this);
    if (oldDataRate != dataRate())
        Q_EMIT dataRateChanged();
    if (oldOutputRange != outputRange())
        Q_EMIT outputRangeChanged();

    Q_D(QmlSensor);
    const auto available = sensor()->availableDataRates();
    d->availableRanges.reserve(available.size());
    for (const qrange &r : available) {
        auto *range = new QmlSensorRange(this);
        range->setMinumum(r.first);
        range->setMaximum(r.second);
        d->availableRanges.append(range);
    }
    const auto output = sensor()->outputRanges();
    d->outputRanges.reserve(output.size());
    for (const qoutputrange &r : output) {
        auto *range = new QmlSensorOutputRange(this);
        range->setMinimum(r.minimum);
        range->setMaximum(r.maximum);
        range->setAccuracy(r.accuracy);
        d->outputRanges.append(range);
    }

    // meta-data should become non-empty
    if (!description().isEmpty())
        Q_EMIT descriptionChanged();
    if (available.size())
        Q_EMIT availableDataRatesChanged();
    if (output.size())
        Q_EMIT outputRangesChanged();

    connect(sensor(), SIGNAL(readingChanged()), this, SLOT(updateReading()));
    if (m_activateOnComplete)
        start();
}

void QmlSensor::updateReading()
{
    if (m_reading) {
        m_reading->update();
        m_reading.notify();
        Q_EMIT readingChanged();
    }
}

/*!
    \qmltype SensorReading
//!    \nativetype QmlSensorReading
    \inqmlmodule QtSensors
    \since QtSensors 5.0
    \brief The SensorReading element serves as a base type for sensor readings.

    The SensorReading element serves as a base type for sensor readings.

    This element wraps the QSensorReading class. Please see the documentation for
    QSensorReading for details.

    This element cannot be directly created.
*/

/*!
    \qmlproperty quint64 SensorReading::timestamp
    A timestamp for the reading.

    Please see QSensorReading::timestamp for information about this property.
*/

quint64 QmlSensorReading::timestamp() const
{
    return m_timestamp;
}

QBindable<quint64> QmlSensorReading::bindableTimestamp() const
{
    return &m_timestamp;
}


void QmlSensorReading::update()
{
    m_timestamp = reading()->timestamp();
    readingUpdate();
}

QT_END_NAMESPACE
