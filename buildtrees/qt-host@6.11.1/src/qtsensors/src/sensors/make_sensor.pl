#!/usr/bin/perl
# Copyright (C) 2021 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# About this script
#
# The make_sensor.pl creates new sensor frontend placeholder implementations.
# Both C++ and QML classes are generated. As the script makes assumptions on the
# naming conventions and source file locations, the main
# intended use case is adding new sensor frontends to the QtSensors module.
#
# To run the script, provide the sensor name starting with 'Q' as an argument
# (here "QFlow"):
#
# perl make_sensor.pl QFlow
#
# This will generate the following files:
# ../sensorsquick/qmlflow_p.h
# ../sensorsquick/qmlflow.cpp
# qflow_p.h
# qflow.h
# qflow.cpp

use strict;
use warnings;

use Carp;
local $Carp::CarpLevel;# = 1;

my $sensor = get_arg();
my $sensorbase = $sensor;
$sensorbase =~ s/Sensor$//;
my $reading = $sensorbase.'Reading';
my $reading_private = $reading.'Private';
my $filter = $sensorbase.'Filter';
my $no_q_sensor = $sensor;
$no_q_sensor =~ s/^.//;
my $qmlsensor = "Qml".$no_q_sensor;
my $qmlsensorbase = $qmlsensor;
$qmlsensorbase =~ s/Sensor$//;
my $qmlreading = $qmlsensorbase."Reading";
my $no_q_reading = $no_q_sensor;
$no_q_reading =~ s/Sensor$//;
$no_q_reading = $no_q_reading."Reading";

my $filebase;
eval {
    $filebase = get_arg();
};
if ($@) {
    $filebase = lc($sensor);
}

my $qmlfilebase = $filebase;
$qmlfilebase =~ s/^.//;
$qmlfilebase = "qml".$qmlfilebase;

my $pheader = $filebase."_p.h";
my $header = $filebase.".h";
my $source = $filebase.".cpp";
my $qmlsource = "../sensorsquick/".$qmlfilebase.".cpp";
my $qmlheader = "../sensorsquick/".$qmlfilebase."_p.h";

my $pguard = uc($pheader);
$pguard =~ s/\./_/g;

my $guard = uc($header);
$guard =~ s/\./_/g;

my $qmlguard = "QML".uc($no_q_sensor)."_H";

if (! -e $qmlheader) {
    print "Creating $qmlheader\n";
    open OUT, ">$qmlheader" or die $!;
    print OUT '
#ifndef '.$qmlguard.'
#define '.$qmlguard.'

#include "qmlsensor_p.h"

QT_BEGIN_NAMESPACE

class '.$sensor.';

class Q_SENSORSQUICK_EXPORT '.$qmlsensor.' : public QmlSensor
{
    Q_OBJECT
    QML_NAMED_ELEMENT('.$no_q_sensor.')
    QML_ADDED_IN_VERSION(6,2) // CHANGE VERSION
public:
    explicit '.$qmlsensor.'(QObject *parent = 0);
    ~'.$qmlsensor.'();

    QSensor *sensor() const override;

private:
    QmlSensorReading *createReading() const override;

    '.$sensor.' *m_sensor;
};

class Q_SENSORSQUICK_EXPORT '.$qmlreading.' : public QmlSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal myprop READ myprop NOTIFY mypropChanged BINDABLE bindableMyprop)
    QML_NAMED_ELEMENT('.$no_q_reading.')
    QML_UNCREATABLE("Cannot create '.$no_q_reading.'")
    QML_ADDED_IN_VERSION(6,2) // CHANGE VERSION
public:
    explicit '.$qmlreading.'('.$sensor.' *sensor);
    ~'.$qmlreading.'();

    qreal myprop() const;
    QBindable<qreal> bindableMyprop() const;

Q_SIGNALS:
    void mypropChanged();

private:
    QSensorReading *reading() const override;
    void readingUpdate() override;

    '.$sensor.' *m_sensor;
    Q_OBJECT_BINDABLE_PROPERTY('.$qmlreading.', qreal,
                               m_myprop, &'.$qmlreading.'::mypropChanged);
};

QT_END_NAMESPACE
#endif
';
    close OUT;
}

if (! -e $qmlsource) {
    print "Creating $qmlsource\n";
    open OUT, ">$qmlsource" or die $!;
    print OUT '
#include "qml'.lc($no_q_sensor).'_p.h"
#include <'.$sensor.'>

/*!
    \qmltype '.$no_q_sensor.'
    \instantiates '.$qmlsensor.'
    \ingroup qml-sensors_type
    \inqmlmodule QtSensors
    \since QtSensors 6.[INSERT VERSION HERE]
    \inherits Sensor
    \brief The '.$no_q_sensor.' element reports on fubbleness.

    The '.$no_q_sensor.' element reports on fubbleness.

    This element wraps the '.$sensor.' class. Please see the documentation for
    '.$sensor.' for details.

    \sa '.$no_q_reading.'
*/

'.$qmlsensor.'::'.$qmlsensor.'(QObject *parent)
    : QmlSensor(parent)
    , m_sensor(new '.$sensor.'(this))
{
}

'.$qmlsensor.'::~'.$qmlsensor.'()
{
}

QmlSensorReading *'.$qmlsensor.'::createReading() const
{
    return new '.$qmlreading.'(m_sensor);
}

QSensor *'.$qmlsensor.'::sensor() const
{
    return m_sensor;
}

/*!
    \qmltype '.$no_q_reading.'
    \instantiates '.$qmlreading.'
    \ingroup qml-sensors_reading
    \inqmlmodule QtSensors
    \since QtSensors 6.[INSERT VERSION HERE]
    \inherits SensorReading
    \brief The '.$no_q_reading.' element holds the most recent '.$no_q_sensor.' reading.

    The '.$no_q_reading.' element holds the most recent '.$no_q_sensor.' reading.

    This element wraps the '.$reading.' class. Please see the documentation for
    '.$reading.' for details.

    This element cannot be directly created.
*/

'.$qmlreading.'::'.$qmlreading.'('.$sensor.' *sensor)
    : m_sensor(sensor)
{
}

'.$qmlreading.'::~'.$qmlreading.'()
{
}

/*!
    \qmlproperty real '.$no_q_reading.'::myprop
    This property holds the fubble of the device.

    Please see '.$reading.'::myprop for information about this property.
*/

qreal '.$qmlreading.'::myprop() const
{
    return m_myprop;
}

QBindable<qreal> '.$qmlreading.'::bindableMyprop() const
{
    return &m_myprop;
}

QSensorReading *'.$qmlreading.'::reading() const
{
    return m_sensor->reading();
}

void '.$qmlreading.'::readingUpdate()
{
    m_myprop = m_sensor->reading()->myprop();
}
';
    close OUT;
}

if (! -e $pheader) {
    print "Creating $pheader\n";
    open OUT, ">$pheader" or die $!;
    print OUT '
#ifndef '.$pguard.'
#define '.$pguard.'

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class '.$reading_private.'
{
public:
    '.$reading_private.'()
        : myprop(0)
    {
    }

    /*
     * Note that this class is copied so you may need to implement
     * a copy constructor if you have complex types or pointers
     * as values.
     */

    qreal myprop;
};

QT_END_NAMESPACE

#endif
';
    close OUT;
}

if (! -e $header) {
    print "Creating $header\n";
    open OUT, ">$header" or die $!;
    print OUT '
#ifndef '.$guard.'
#define '.$guard.'

#include <QtSensors/qsensor.h>

QT_BEGIN_NAMESPACE

class '.$reading_private.';

class Q_SENSORS_EXPORT '.$reading.' : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(qreal myprop READ myprop)
    DECLARE_READING('.$reading.')
public:
    qreal myprop() const;
    void setMyprop(qreal myprop);
};

class Q_SENSORS_EXPORT '.$filter.' : public QSensorFilter
{
public:
    virtual bool filter('.$reading.' *reading) = 0;
private:
    bool filter(QSensorReading *reading) override;
};

class Q_SENSORS_EXPORT '.$sensor.' : public QSensor
{
    Q_OBJECT
public:
    explicit '.$sensor.'(QObject *parent = 0);
    ~'.$sensor.'();
    '.$reading.' *reading() const;
    static char const * const sensorType;

private:
    Q_DISABLE_COPY('.$sensor.')
};

QT_END_NAMESPACE

#endif
';
    close OUT;
}

if (! -e $source) {
    print "Creating $source\n";
    open OUT, ">$source" or die $!;
    print OUT '
#include <'.$header.'>
#include "'.$pheader.'"

QT_BEGIN_NAMESPACE

IMPLEMENT_READING('.$reading.')

/*!
    \class '.$reading.'
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 6.[INSERT VERSION HERE]

    \brief The '.$reading.' class holds readings from the [X] sensor.

    [Fill this out]

    \section2 '.$reading.' Units

    [Fill this out]
*/

/*!
    \property '.$reading.'::myprop
    \brief [what does it hold?]

    [What are the units?]
    \sa {'.$reading.' Units}
*/

qreal '.$reading.'::myprop() const
{
    return d->myprop;
}

/*!
    Sets [what?] to \a myprop.
*/
void '.$reading.'::setMyprop(qreal myprop)
{
    d->myprop = myprop;
}

// =====================================================================

/*!
    \class '.$filter.'
    \ingroup sensors_filter
    \inmodule QtSensors
    \since 6.[INSERT VERSION HERE]

    \brief The '.$filter.' class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to '.$reading.'
    instead of QSensorReading.
*/

/*!
    \fn '.$filter.'::filter('.$reading.' *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
*/

bool '.$filter.'::filter(QSensorReading *reading)
{
    return filter(static_cast<'.$reading.'*>(reading));
}

char const * const '.$sensor.'::sensorType("'.$sensor.'");

/*!
    \class '.$sensor.'
    \ingroup sensors_type
    \inmodule QtSensors
    \since 6.[INSERT VERSION HERE]

    \brief The '.$sensor.' class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a '.$reading.' instead of a QSensorReading.

    For details about how the sensor works, see \l '.$reading.'.

    \sa '.$reading.'
*/

/*!
    Construct the sensor as a child of \a parent.
*/
'.$sensor.'::'.$sensor.'(QObject *parent)
    : QSensor('.$sensor.'::sensorType, parent)
{
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
*/
'.$sensor.'::~'.$sensor.'()
{
}

/*!
    \fn '.$sensor.'::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
*/

'.$reading.' *'.$sensor.'::reading() const
{
    return static_cast<'.$reading.'*>(QSensor::reading());
}

#include "moc_'.$source.'"
QT_END_NAMESPACE
';
    close OUT;
}

exit 0;


sub get_arg
{
    if (scalar(@ARGV) == 0) {
        croak "Missing sensor name argument (e.g. 'QFlow')";
    }
    return shift(@ARGV);
}
