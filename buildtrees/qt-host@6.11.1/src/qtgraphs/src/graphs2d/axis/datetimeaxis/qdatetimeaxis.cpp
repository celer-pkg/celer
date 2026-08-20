// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qdatetimeaxis.h>
#include <QtGraphs/private/qdatetimeaxis_p.h>
#include <QtCore/qobject.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE
/*!
 \class QDateTimeAxis
 \inmodule QtGraphs
 \ingroup graphs_2D
 \brief The QDateTimeAxis adds support for DateTime values to be added to a graph's axis.

 A DateTime Axis can be used to display DateTime representations with tick marks and grid lines.
 The DateTime items on the axis are displayed at the position of the ticks.
*/

/*!
 \qmltype DateTimeAxis
 \nativetype QDateTimeAxis
 \inqmlmodule QtGraphs
 \ingroup graphs_qml_2D
 \inherits AbstractAxis
 \brief Adds DateTime items to a graph's axis.

 A DateTime Axis can be used to display DateTime representations with tick marks and grid lines.
 The DateTime items on the axis are displayed at the position of the ticks.

 The following example code illustrates how to use the DateTimeAxis type:
 \code
 GraphsView {
     axisX: DateTimeAxis {
         min: new Date(2000,1,1)
         max: new Date(1970,1,1)
     }
     LineSeries {
         // Add a few XYPoint data...
     }
 }
 \endcode
*/

/*!
 \property QDateTimeAxis::min
 \brief The minimum value on the axis

 This value can be lower or higher than the maximum.
 The DateTime is stored as UTC internally.
 The default value is new Date(1970,1,1)
*/
/*!
 \qmlproperty real DateTimeAxis::min
 The minimum value on the axis.

 This value can be lower or higher than the maximum.
 The DateTime is stored as UTC internally.
 The default value is new Date(1970,1,1)
*/

/*!
 \property QDateTimeAxis::max
 \brief The maximum value on the axis

 This value can be lower or higher than the minimum.
 The DateTime is stored as UTC internally.
 The default value is new Date(1980,1,1)
*/
/*!
 \qmlproperty real DateTimeAxis::max
 The maximum value on the axis.

 This value can be lower or higher than the minimum.
 The DateTime is stored as UTC internally.
 The default value is new Date(1980,1,1)
*/
/*!
 \property QDateTimeAxis::subTickCount
 \brief The number of subticks on the axis. This indicates how many subticks are drawn
 between major lines on the graph. Labels are not drawn for subticks. The default value is 0.
*/
/*!
 \qmlproperty int DateTimeAxis::subTickCount
 The number of subticks on the axis. This indicates how many subticks are drawn
 between major lines on the graph. Labels are not drawn for subticks. The default value is 0.
*/
/*!
 \property QDateTimeAxis::tickInterval
 \brief The amount of major ticks are placed on an axis. If value is 0 or lower, axis will
 automatically calculate appropriate amount of ticks. The default value is 0.
*/
/*!
 \qmlproperty real DateTimeAxis::tickInterval
 \brief The amount of major ticks are placed on an axis. If value is 0 or lower, axis will
 automatically calculate appropriate amount of ticks. The default value is 0.
 */
/*!
 \property QDateTimeAxis::timeZone
 \brief The time zone that will be used to display labels of the axis.
 The accepted values are based on IANA time zone IDs. The default time zone is UTC.
 \since 6.11
*/
/*!
 \qmlproperty QTimeZone DateTimeAxis::timeZone
 The time zone that will be used to display labels of the axis.
 The accepted values are based on IANA time zone IDs. The default time zone is UTC.
 \since 6.11
*/

/*!
 \property QDateTimeAxis::labelFormat
 \brief The format of the DateTime labels on the axis.
 The format property allows to signify the visual representation of the DateTime object, in days,
  months, and years. The default value is dd-MMMM-yy.
*/

/*!
 \qmlproperty string DateTimeAxis::labelFormat
 The format of the DateTime labels on the axis
 The format property allows to signify the visual representation of the DateTime object, in days,
  months, and years. The default value is dd-MMMM-yy.
 */

/*!
 \property QDateTimeAxis::zoom
 \since 6.11
 \brief The zoom value of the axis.
 The zoom value enlarges or shrinks the axis and thus the graph without affecting intervals
 of grid and labels. The default value is 1.
*/

/*!
 \qmlproperty real DateTimeAxis::zoom
 The zoom value of the axis.
 The zoom value enlarges or shrinks the axis and thus the graph without affecting intervals
 of grid and labels. The default value is 1.
 \since 6.11
*/

/*!
 \property QDateTimeAxis::pan
 \since 6.11
 \brief The pan value of the axis.

 The pan value moves the center of the axis without affecting intervals
 of grid and labels. The default value is 0.
*/

/*!
 \qmlproperty real DateTimeAxis::pan
 \since 6.11
 The pan value of the axis.

 The pan value moves the center of the axis without affecting intervals
 of grid and labels. The default value is 0.
*/

/*!
 \property QDateTimeAxis::visualMin
 \readonly
 \since 6.11
 \brief The visual minimum value of the axis.

 This property holds a a visual minimum axis value when axis has been
 panned or zoomed. The default value is \l{QDateTimeAxis::min}
*/
/*!
 \qmlproperty real DateTimeAxis::visualMin
 \readonly
 \since 6.11
 The visual minimum value of the axis.

 This property holds a a visual minimum axis value when axis has been
 panned or zoomed. The default value is \l min
*/

/*!
 \property QDateTimeAxis::visualMax
 \readonly
 \since 6.11
 \brief The visual maximum value of the axis.

 This property holds a a visual maximum axis value when axis has been
 panned or zoomed. The default value is \l{QDateTimeAxis::max}
*/
/*!
 \qmlproperty real DateTimeAxis::visualMax
 \readonly
 \since 6.11
 The visual maximum value of the axis.

 This property holds a a visual maximum axis value when axis has been
 panned or zoomed. The default value is \l max
*/

/*!
 \qmlmethod QTimeZone DateTimeAxis::timeZoneFromString(string zoneId)
 \since 6.11
 Returns the QTimeZone corresponding to IANA based \a zoneId.
 \note This is the converter method for setting the \l {timeZone}.
 \sa QDateTimeAxis::timeZone, timeZoneAsString
*/
/*!
 \qmlmethod string DateTimeAxis::timeZoneAsString()
 \since 6.11
 Returns the \l {QDateTimeAxis::timeZone}'s IANA based zone ID as a string.
 \sa timeZone
*/

/*!
 \qmlsignal DateTimeAxis::minChanged(DateTime min)
 This signal is emitted when the minimum value of the axis, specified by \a min, changes.
*/
/*!
 \qmlsignal DateTimeAxis::maxChanged(DateTime max)
 This signal is emitted when the maximum value of the axis, specified by \a max, changes.
*/
/*!
 \qmlsignal DateTimeAxis::subTickCountChanged(int subTickCount)
 This signal is emitted when the number of subticks on the axis, specified by
 \a subTickCount, changes.
*/
/*!
 \qmlsignal DateTimeAxis::rangeChanged(real min, real max)
 This signal is emitted when the minimum or maximum value of the axis, specified by \a min
 and \a max, changes.
*/
/*!
 \qmlsignal DateTimeAxis::labelFormatChanged(string format)
 This signal is emitted when the \a format of axis labels changes.
*/
/*!
 \qmlsignal DateTimeAxis::tickIntervalChanged(real tickInterval)
 This signal is emitted when the tick interval value, specified by
 \a tickInterval, changes.
*/
/*!
 \qmlsignal DateTimeAxis::timeZoneChanged()
 This signal is emitted when the time zone is changed. It represents a string value for the
 IANA time zone ID that was set.
 \since 6.11
*/
/*!
 \qmlsignal DateTimeAxis::zoomChanged()
 This signal is emmited when the zoom value changes.
 \since 6.11
*/

QDateTimeAxis::QDateTimeAxis(QObject *parent)
    : QAbstractAxis(*(new QDateTimeAxisPrivate), parent)
{}

QDateTimeAxis::~QDateTimeAxis() {}

QAbstractAxis::AxisType QDateTimeAxis::type() const
{
    return QAbstractAxis::AxisType::DateTime;
}

void QDateTimeAxis::setMin(const QDateTime &min)
{
    Q_D(QDateTimeAxis);
    if (min.isValid()) {
        d->setRange(min.toMSecsSinceEpoch(), d->m_max);
        emit update();
    }
}

QDateTime QDateTimeAxis::min() const
{
    Q_D(const QDateTimeAxis);
    return QDateTime::fromMSecsSinceEpoch(d->m_min, d->m_timeZone);
}

void QDateTimeAxis::setMax(const QDateTime &max)
{
    Q_D(QDateTimeAxis);
    if (max.isValid()) {
        d->setRange(d->m_min, max.toMSecsSinceEpoch());
        emit update();
    }
}

QDateTime QDateTimeAxis::max() const
{
    Q_D(const QDateTimeAxis);
    return QDateTime::fromMSecsSinceEpoch(d->m_max, d->m_timeZone);
}

void QDateTimeAxis::setLabelFormat(const QString &format)
{
    Q_D(QDateTimeAxis);
    if (d->m_format != format) {
        d->m_format = format;
        emit labelFormatChanged(format);
        emit update();
    }
}

QString QDateTimeAxis::labelFormat() const
{
    Q_D(const QDateTimeAxis);
    return d->m_format;
}

qreal QDateTimeAxis::tickInterval() const
{
    Q_D(const QDateTimeAxis);
    return d->m_tickInterval;
}

void QDateTimeAxis::setTickInterval(qreal newTickInterval)
{
    Q_D(QDateTimeAxis);

    if (newTickInterval < 0.0)
        newTickInterval = 0.0;

    if (qFuzzyCompare(d->m_tickInterval + 1, newTickInterval + 1))
        return;
    d->m_tickInterval = newTickInterval;
    emit tickIntervalChanged();
    emit update();
}

int QDateTimeAxis::subTickCount() const
{
    Q_D(const QDateTimeAxis);
    return d->m_subTickCount;
}

void QDateTimeAxis::setSubTickCount(int newSubTickCount)
{
    Q_D(QDateTimeAxis);

    if (newSubTickCount < 0.0)
        newSubTickCount = 0.0;

    if (d->m_subTickCount == newSubTickCount)
        return;
    d->m_subTickCount = newSubTickCount;
    emit subTickCountChanged();
    emit update();
}

QTimeZone QDateTimeAxis::timeZone() const
{
#if QT_CONFIG(timezone)
    Q_D(const QDateTimeAxis);

    return d->m_timeZone;
#else
    return QTimeZone(QTimeZone::Initialization::UTC);
#endif
}

void QDateTimeAxis::setTimeZone(const QTimeZone &timeZone)
{
    Q_D(QDateTimeAxis);

    if (!timeZone.isValid())
        return;

    d->m_timeZone = timeZone;
    emit timeZoneChanged(timeZone);
    emit update();
}

QTimeZone QDateTimeAxis::timeZoneFromString(const QString &zoneId) const
{

#if QT_CONFIG(timezone)
    auto zone = QTimeZone(zoneId.toUtf8());
#else
    auto zone = QTimeZone(QTimeZone::Initialization::UTC);
#endif

    return zone;
}

QString QDateTimeAxis::timeZoneAsString() const
{
#if QT_CONFIG(timezone)
    Q_D(const QDateTimeAxis);
    return QString::fromLatin1(d->m_timeZone.id());
#else
    return QString::fromLatin1("UTC");
#endif
}

void QDateTimeAxis::setZoom(qreal zoom)
{
    Q_D(QDateTimeAxis);

    if (QtPrivate::fuzzyCompare(d->m_zoom, zoom)) {
        qCDebug(lcAxis2D, "axis already zoomed to value of %f", d->m_zoom);
        return;
    }

    d->m_zoom = zoom;
    d->calculateVisualRange();
    emit zoomChanged(zoom);
    emit update();
}

qreal QDateTimeAxis::zoom() const
{
    Q_D(const QDateTimeAxis);
    return d->m_zoom;
}

void QDateTimeAxis::setPan(qreal pan)
{
    Q_D(QDateTimeAxis);

    if (QtPrivate::fuzzyCompare(d->m_pan, pan)) {
        qCDebug(lcAxis2D, "panning already set to: %f", d->m_pan);
        return;
    }

    d->m_pan = pan;
    d->calculateVisualRange();
    emit panChanged(pan);
    emit update();
}

qreal QDateTimeAxis::pan() const
{
    Q_D(const QDateTimeAxis);
    return d->m_pan;
}

QDateTime QDateTimeAxis::visualMin() const
{
    Q_D(const QDateTimeAxis);
    return QDateTime::fromMSecsSinceEpoch(d->m_visualMin, d->m_timeZone);
}

QDateTime QDateTimeAxis::visualMax() const
{
    Q_D(const QDateTimeAxis);
    return QDateTime::fromMSecsSinceEpoch(d->m_visualMax, d->m_timeZone);
}

///////////////////////////////////////////////////////////////////////////////

QDateTimeAxisPrivate::QDateTimeAxisPrivate() {}

QDateTimeAxisPrivate::~QDateTimeAxisPrivate() {}

void QDateTimeAxisPrivate::setMin(const QVariant &min)
{
    Q_Q(QDateTimeAxis);
    if (min.canConvert<QDateTime>())
        q->setMin(min.toDateTime());
}

void QDateTimeAxisPrivate::setMax(const QVariant &max)
{
    Q_Q(QDateTimeAxis);
    if (max.canConvert<QDateTime>())
        q->setMax(max.toDateTime());
}

void QDateTimeAxisPrivate::setRange(const QVariant &min, const QVariant &max)
{
    Q_Q(QDateTimeAxis);
    if (min.canConvert<QDateTime>() && max.canConvert<QDateTime>())
        q->setRange(min.toDateTime(), max.toDateTime());
}

void QDateTimeAxisPrivate::setRange(qreal min, qreal max)
{
    Q_Q(QDateTimeAxis);

    bool changed = false;

    if (m_min != min) {
        m_min = min;
        changed = true;
        emit q->minChanged(QDateTime::fromMSecsSinceEpoch(min, QTimeZone::UTC));
    }

    if (m_max != max) {
        m_max = max;
        changed = true;
        emit q->maxChanged(QDateTime::fromMSecsSinceEpoch(max, QTimeZone::UTC));
    }

    if (changed) {
        calculateVisualRange();
        emit q->rangeChanged(min, max);
    }
}

void QDateTimeAxisPrivate::calculateVisualRange()
{
    Q_Q(QDateTimeAxis);
    qreal diff = m_max - m_min;
    qreal center = diff / 2.0f + m_min + m_pan;
    diff /= m_zoom;
    qreal max = center + diff / 2.0f;
    qreal min = center - diff / 2.0f;

    if (!QtPrivate::fuzzyCompare(m_visualMin, min)) {
        m_visualMin = min;
        emit q->visualMinChanged(QDateTime::fromMSecsSinceEpoch(min, m_timeZone));
    }

    if (!QtPrivate::fuzzyCompare(m_visualMax, max)) {
        m_visualMax = max;
        emit q->visualMaxChanged(QDateTime::fromMSecsSinceEpoch(max, m_timeZone));
    }
}

QT_END_NAMESPACE

#include "moc_qdatetimeaxis.cpp"
