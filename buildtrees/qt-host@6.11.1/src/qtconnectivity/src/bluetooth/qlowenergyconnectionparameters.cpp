// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergyconnectionparameters.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QLowEnergyConnectionParameters)

class QLowEnergyConnectionParametersPrivate : public QSharedData
{
public:
    QLowEnergyConnectionParametersPrivate()
        : minInterval(7.5)
        , maxInterval(4000)
        , latency(0)
        , timeout(32000)
    {
    }

    double minInterval;
    double maxInterval;
    int latency;
    int timeout;
};

/*!
    \since 5.7
    \class QLowEnergyConnectionParameters
    \brief The QLowEnergyConnectionParameters class is used when requesting or reporting
           an update of the parameters of a Bluetooth LE connection.

    The connection parameters influence how often a master and a slave device synchronize
    with each other. In general, a lower connection interval and latency means faster communication,
    but also higher power consumption. How these criteria should be weighed against each other
    is highly dependent on the concrete use case.

    Android only indirectly permits the adjustment of this parameter set.
    The platform separates the connection parameters into three categories (hight, low & balanced
    priority). Each category implies a predefined set of values for \l minimumInterval(),
    \l maximumInterval() and \l latency(). Additionally, the value ranges of each category can vary
    from one Android device to the next. Qt uses the \l minimumInterval() to determine the target
    category as follows:

    \table
    \header
        \li minimumInterval()
        \li Android priority
    \row
        \li interval < 30
        \li CONNECTION_PRIORITY_HIGH
    \row
        \li 30 <= interval <= 100
        \li CONNECTION_PRIORITY_BALANCED
    \row
        \li interval > 100
        \li CONNECTION_PRIORITY_LOW_POWER
    \endtable

    The \l supervisionTimeout() cannot be changed on Android and is therefore ignored.


    \inmodule QtBluetooth
    \ingroup shared

    \sa QLowEnergyController::requestConnectionUpdate
    \sa QLowEnergyController::connectionUpdated
*/


/*!
   Constructs a new object of this class. All values are initialized to valid defaults.
 */
QLowEnergyConnectionParameters::QLowEnergyConnectionParameters()
    : d(new QLowEnergyConnectionParametersPrivate)
{
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyConnectionParameters::QLowEnergyConnectionParameters(const QLowEnergyConnectionParameters &other)
    : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyConnectionParameters::~QLowEnergyConnectionParameters()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyConnectionParameters &QLowEnergyConnectionParameters::operator=(const QLowEnergyConnectionParameters &other)
{
    d = other.d;
    return *this;
}

/*!
   Sets the range in which the connection interval should be. The actual value will be decided by
   the controller. Both \a minimum and \a maximum are given in milliseconds.
   If \a maximum is smaller than \a minimum, it will be set to the value of \a minimum.
   The smallest possible connection interval is 7.5 milliseconds, the largest one is
   4000 milliseconds.
   \sa minimumInterval(), maximumInterval()
 */
void QLowEnergyConnectionParameters::setIntervalRange(double minimum, double maximum)
{
    d->minInterval = minimum;
    d->maxInterval = qMax(minimum, maximum);
}

/*!
   Returns the minimum connection interval in milliseconds. The default is 7.5.
   \note If this object was emitted via \l QLowEnergyController::connectionUpdated(), then
         this value is the same as \l maximumInterval() and refers to the actual
         connection interval.
   \sa setIntervalRange()
 */
double QLowEnergyConnectionParameters::minimumInterval() const
{
    return d->minInterval;
}

/*!
   Returns the maximum connection interval in milliseconds. The default is 4000.
   \note If this object was emitted via \l QLowEnergyController::connectionUpdated(), then
         this value is the same as \l minimumInterval() and refers to the actual
         connection interval.
   \sa setIntervalRange()
 */
double QLowEnergyConnectionParameters::maximumInterval() const
{
    return d->maxInterval;
}

/*!
  Sets the slave latency of the connection (that is, the number of connection events that a slave
  device is allowed to ignore) to \a latency. The minimum value is 0, the maximum is 499.
  \sa latency()
 */
void QLowEnergyConnectionParameters::setLatency(int latency)
{
    d->latency = latency;
}

/*!
 Returns the slave latency of the connection.
 \sa setLatency()
*/
int QLowEnergyConnectionParameters::latency() const
{
    return d->latency;
}

/*!
  Sets the link supervision timeout to \a timeout milliseconds.
  There are several constraints on this value: It must be in the range [100,32000] and it must be
  larger than (1 + \l latency()) * 2 * \l maximumInterval().

  On Android, this timeout is not adjustable and therefore ignored.

  \sa supervisionTimeout()
 */
void QLowEnergyConnectionParameters::setSupervisionTimeout(int timeout)
{
    d->timeout = timeout;
}

/*!
  Returns the link supervision timeout of the connection in milliseconds.
  \sa setSupervisionTimeout()
*/
int QLowEnergyConnectionParameters::supervisionTimeout() const
{
    return d->timeout;
}

/*!
   \fn void QLowEnergyConnectionParameters::swap(QLowEnergyConnectionParameters &other)
   Swaps this object with \a other.
 */

/*!
    \brief Returns \a true if \a a and \a b are equal with respect to their public state,
    otherwise returns false.
    \internal
 */
bool QLowEnergyConnectionParameters::equals(const QLowEnergyConnectionParameters &a,
                                            const QLowEnergyConnectionParameters &b)
{
    if (a.d == b.d)
        return true;
    return a.minimumInterval() == b.minimumInterval() && a.maximumInterval() == b.maximumInterval()
            && a.latency() == b.latency() && a.supervisionTimeout() == b.supervisionTimeout();
}

/*!
   \fn bool QLowEnergyConnectionParameters::operator!=(
                                    const QLowEnergyConnectionParameters &p1,
                                    const QLowEnergyConnectionParameters &p2)
   \brief Returns \c true if \a p1 and \a p2 are not equal with respect to their public state,
   otherwise returns \c false.
 */

/*!
    \fn bool QLowEnergyConnectionParameters::operator==(
                                    const QLowEnergyConnectionParameters &p1,
                                    const QLowEnergyConnectionParameters &p2)
    \brief Returns \c true if \a p1 and \a p2 are equal with respect to their public state,
    otherwise returns \c false.
 */

QT_END_NAMESPACE
