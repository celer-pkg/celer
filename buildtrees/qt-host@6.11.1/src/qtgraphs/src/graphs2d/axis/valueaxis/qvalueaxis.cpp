// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/QValueAxis>
#include <private/qvalueaxis_p.h>
#include <private/charthelpers_p.h>
#include <QtCore/QtMath>

QT_BEGIN_NAMESPACE
/*!
    \class QValueAxis
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QValueAxis class adds values to a graph's axes.

    A value axis can be set up to show an axis line with tick marks, grid lines, and shades.
    The values on the axis are drawn at the positions of tick marks.
*/
/*!
    \qmltype ValueAxis
    \nativetype QValueAxis
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits AbstractAxis
    \brief Adds values to a graph's axes.

    The ValueAxis type can be set up to show an axis line with tick marks, grid lines, and shades.
    The values on the axis are drawn at the positions of tick marks.

    The following example code illustrates how to use the ValueAxis type:
    \code
    GraphsView {
        axisX: ValueAxis {
            max: 10
            tickInterval: 1
        }
        axisY: ValueAxis {
            min -20
            max: 40
        }
        LineSeries {
            // Add a few XYPoint data...
        }
    }
    \endcode
*/

/*!
  \property QValueAxis::min
  \brief The minimum value on the axis.

  When setting this property, the maximum value is adjusted if necessary, to ensure that
  the range remains valid.
  The default value is 0.0
*/
/*!
  \qmlproperty real ValueAxis::min
  The minimum value on the axis.

  When setting this property, the maximum value is adjusted if necessary, to ensure that
  the range remains valid.
  The default value is 0.0
*/

/*!
  \property QValueAxis::max
  \brief The maximum value on the axis.

  When setting this property, the minimum value is adjusted if necessary, to ensure that
  the range remains valid.
  The default value is 10.0
*/
/*!
  \qmlproperty real ValueAxis::max
  The maximum value on the axis.

  When setting this property, the minimum value is adjusted if necessary, to ensure that
  the range remains valid.
  The default value is 10.0
*/

/*!
  \property QValueAxis::subTickCount
  \brief The number of subticks on the axis. This indicates how many subticks are drawn
  between major lines on the graph. Labels are not drawn for subticks. The default value is 0.
*/
/*!
  \qmlproperty int ValueAxis::subTickCount
  The number of subticks on the axis. This indicates how many subticks are drawn
  between major lines on the graph. Labels are not drawn for subticks. The default value is 0.
*/

/*!
  \property QValueAxis::tickAnchor
  \brief The base value where the dynamically placed tick marks and labels are started from.
  The default value is 0.
 */
/*!
  \qmlproperty real ValueAxis::tickAnchor
  The base value where the dynamically placed tick marks and labels are started from.
  The default value is 0.
 */

/*!
  \property QValueAxis::tickInterval
  \brief The interval between dynamically placed tick marks and labels.
  The default value is 0, which means that intervals are automatically calculated
  based on the min and max range.
*/
/*!
  \qmlproperty real ValueAxis::tickInterval
  The interval between dynamically placed tick marks and labels.
  The default value is 0, which means that intervals are automatically calculated
  based on the min and max range.
*/

/*!
  \property QValueAxis::labelFormat
  \brief The label format of the axis.

  The format string supports the following conversion specifiers, length modifiers, and flags
  provided by \c printf() in the standard C++ library: d, i, o, x, X, f, F, e, E, g, G, c.

  The default value is empty, in which case 'f' format is used.

  \sa QString::asprintf()
*/
/*!
  \qmlproperty string ValueAxis::labelFormat

  The format string supports the following conversion specifiers, length modifiers, and flags
  provided by \c printf() in the standard C++ library: d, i, o, x, X, f, F, e, E, g, G, c.

  The default value is empty, in which case 'f' format is used.

  \sa QString::asprintf()
*/

/*!
  \property QValueAxis::labelDecimals
  \brief The number of decimals used for showing the labels. When set to -1, decimal amount
  is adjusted automatically based on the values range. The default value is -1.
*/
/*!
  \qmlproperty int ValueAxis::labelDecimals
  The number of decimals used for showing the labels. When set to -1, decimal amount
  is adjusted automatically based on the values range. The default value is -1.
*/

/*!
  \property QValueAxis::zoom
  \since 6.9
  \brief The zoom value of the axis.

  The zoom value enlarges or shrinks the axis and thus the graph without affecting intervals
  of grid and labels. The default value is 1.
*/
/*!
  \qmlproperty real ValueAxis::zoom
  \since 6.9
  The zoom value of the axis.

  The zoom value enlarges or shrinks the axis and thus the graph without affecting intervals
  of grid and labels. The default value is 1.
*/

/*!
  \property QValueAxis::pan
  \since 6.9
  \brief The pan value of the axis.

  The pan value moves the center of the axis without affecting intervals
  of grid and labels. The default value is 0.
*/
/*!
  \qmlproperty real ValueAxis::pan
  \since 6.9
  The pan value of the axis.

  The pan value moves the center of the axis without affecting intervals
  of grid and labels. The default value is 0.
*/

/*!
 \property QValueAxis::visualMin
 \readonly
 \since 6.11
 \brief The visual minimum value of the axis.

 This property holds a a visual minimum axis value when axis has been
 panned or zoomed. The default value is \l{QValueAxis::min}
*/
/*!
 \qmlproperty real ValueAxis::visualMin
 \readonly
 \since 6.11
 The visual minimum value of the axis.

 This property holds a a visual minimum axis value when axis has been
 panned or zoomed. The default value is \l min
*/

/*!
 \property QValueAxis::visualMax
 \readonly
 \since 6.11
 \brief The visual maximum value of the axis.

 This property holds a a visual maximum axis value when axis has been
 panned or zoomed. The default value is \l{QValueAxis::max}
*/
/*!
 \qmlproperty real ValueAxis::visualMax
 \readonly
 \since 6.11
 The visual maximum value of the axis.

 This property holds a a visual maximum axis value when axis has been
 panned or zoomed. The default value is \l max
*/

/*!
  \qmlsignal ValueAxis::minChanged(real min)
  This signal is emitted when the minimum value of the axis changes to \a min.
*/

/*!
  \qmlsignal ValueAxis::maxChanged(real max)
  This signal is emitted when the maximum value of the axis changes to \a max.
*/

/*!
  \qmlsignal ValueAxis::subTickCountChanged(int subTickCount)
  This signal is emitted when the number of subticks on the axis, specified by
  \a subTickCount, changes.
*/

/*!
    \qmlsignal ValueAxis::rangeChanged(real min, real max)
  This signal is emitted when the minimum or maximum value of the axis
  changes to \a min and \a max, respectively.
*/

/*!
  \qmlsignal ValueAxis::labelFormatChanged(string format)
  This signal is emitted when the format of axis labels changes to \a format.
*/

/*!
  \qmlsignal ValueAxis::labelDecimalsChanged(int decimals)
  This signal is emitted when the amount of axis label decimals changes to \a decimals.
*/

/*!
  \qmlsignal ValueAxis::tickAnchorChanged(real tickAnchor)
  This signal is emitted when the tick anchoring value changes to \a tickAnchor.
*/

/*!
  \qmlsignal ValueAxis::tickIntervalChanged(real tickInterval)
  This signal is emitted when the tick interval value, changes to
  \a tickInterval.
*/

/*!
    Constructs an axis object that is a child of \a parent.
*/
QValueAxis::QValueAxis(QObject *parent)
    : QAbstractAxis(*(new QValueAxisPrivate), parent)
{}

/*!
    \internal
*/
QValueAxis::QValueAxis(QValueAxisPrivate &d, QObject *parent)
    : QAbstractAxis(d, parent)
{}

/*!
    Destroys the object.
*/
QValueAxis::~QValueAxis()
{
}

void QValueAxis::setMin(qreal min)
{
    Q_D(QValueAxis);
    setRange(min, qMax(d->m_max, min));
}

qreal QValueAxis::min() const
{
    Q_D(const QValueAxis);
    return d->m_min;
}

void QValueAxis::setMax(qreal max)
{
    Q_D(QValueAxis);
    setRange(qMin(d->m_min, max), max);
}

qreal QValueAxis::max() const
{
    Q_D(const QValueAxis);
    return d->m_max;
}

/*!
  Sets the range from \a min to \a max on the axis.
  If \a min is greater than \a max, this function returns without making any changes.
*/
void QValueAxis::setRange(qreal min, qreal max)
{
    Q_D(QValueAxis);
    d->setRange(min,max);
    emit update();
}

void QValueAxis::setSubTickCount(qsizetype count)
{
    Q_D(QValueAxis);
    if (d->m_subTickCount != count && count >= 0) {
        d->m_subTickCount = count;
        emit update();
        emit subTickCountChanged(count);
    }
}

qsizetype QValueAxis::subTickCount() const
{
    Q_D(const QValueAxis);
    return d->m_subTickCount;
}

void QValueAxis::setTickAnchor(qreal anchor)
{
    Q_D(QValueAxis);
    if (d->m_tickAnchor != anchor) {
        d->m_tickAnchor = anchor;
        emit update();
        emit tickAnchorChanged(anchor);
    }
}

qreal QValueAxis::tickAnchor() const
{
    Q_D(const QValueAxis);
    return d->m_tickAnchor;
}

void QValueAxis::setTickInterval(qreal interval)
{
    Q_D(QValueAxis);
    if (d->m_tickInterval != interval) {
        d->m_tickInterval = interval;
        emit update();
        emit tickIntervalChanged(interval);
    }
}

qreal QValueAxis::tickInterval() const
{
    Q_D(const QValueAxis);
    return d->m_tickInterval;
}

void QValueAxis::setLabelFormat(const QString &format)
{
    Q_D(QValueAxis);
    d->m_format = format;
    emit update();
    emit labelFormatChanged(format);
}

QString QValueAxis::labelFormat() const
{
    Q_D(const QValueAxis);
    return d->m_format;
}

void QValueAxis::setLabelDecimals(int decimals)
{
    Q_D(QValueAxis);
    if (d->m_decimals != decimals) {
        d->m_decimals = decimals;
        emit update();
        emit labelDecimalsChanged(decimals);
    } else {
        qCDebug(lcAxis2D, "QValueAxis::setLabelDecimals. Label decimals is already set to: %d",
                decimals);
    }
}

int QValueAxis::labelDecimals() const
{
    Q_D(const QValueAxis);
    return d->m_decimals;
}

void QValueAxis::setZoom(qreal zoom)
{
    Q_D(QValueAxis);
    if (d->m_zoom != zoom) {
        d->m_zoom = zoom;
        d->calculateVisualRange();
        emit update();
        emit zoomChanged(zoom);
    } else {
        qCDebug(lcAxis2D, "QValueAxis::setZoom. Zoom is already set to: %f",
                zoom);
    }
}

qreal QValueAxis::zoom() const
{
    Q_D(const QValueAxis);
    return d->m_zoom;
}

void QValueAxis::setPan(qreal pan)
{
    Q_D(QValueAxis);
    if (d->m_pan != pan) {
        d->m_pan = pan;
        d->calculateVisualRange();
        emit update();
        emit panChanged(pan);
    } else {
        qCDebug(lcAxis2D, "QValueAxis::setPan. Panning is already set to: %f",
                pan);
    }
}

qreal QValueAxis::pan() const
{
    Q_D(const QValueAxis);
    return d->m_pan;
}

qreal QValueAxis::visualMin() const
{
    Q_D(const QValueAxis);
    return d->m_visualMin;
}

qreal QValueAxis::visualMax() const
{
    Q_D(const QValueAxis);
    return d->m_visualMax;
}

/*!
  Returns the type of the axis.
*/
QAbstractAxis::AxisType QValueAxis::type() const
{
    return QAbstractAxis::AxisType::Value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QValueAxisPrivate::QValueAxisPrivate()
    : m_min(0)
    , m_max(10)
    , m_subTickCount(0)
    , m_format()
    , m_decimals(-1)
    , m_tickAnchor(0.0)
    , m_tickInterval(0.0)
{}

QValueAxisPrivate::~QValueAxisPrivate() {}

void QValueAxisPrivate::setMin(const QVariant &min)
{
    Q_Q(QValueAxis);
    bool ok;
    qreal value = min.toReal(&ok);
    if (ok)
        q->setMin(value);
}

void QValueAxisPrivate::setMax(const QVariant &max)
{
    Q_Q(QValueAxis);
    bool ok;
    qreal value = max.toReal(&ok);
    if (ok)
        q->setMax(value);
}

void QValueAxisPrivate::setRange(const QVariant &min, const QVariant &max)
{
    Q_Q(QValueAxis);
    bool ok1;
    bool ok2;
    qreal value1 = min.toReal(&ok1);
    qreal value2 = max.toReal(&ok2);
    if (ok1 && ok2)
        q->setRange(value1, value2);
}

void QValueAxisPrivate::setRange(qreal min, qreal max)
{
    Q_Q(QValueAxis);
    bool changed = false;

    if (min > max) {
        qCWarning(lcAxis2D, "min value is higher than max vaue.");
        return;
    }

    if (!isValidValue(min, max)) {
        qCWarning(lcAxis2D, "attempting to set invalid range for value axis: [%f - %f]", min, max);
        return;
    }

    if (m_min != min) {
        m_min = min;
        changed = true;
        emit q->minChanged(min);
    }

    if (m_max != max) {
        m_max = max;
        changed = true;
        emit q->maxChanged(max);
    }

    if (changed) {
        calculateVisualRange();
        emit q->rangeChanged(min, max);
    }
}

void QValueAxisPrivate::calculateVisualRange()
{
    Q_Q(QValueAxis);
    qreal diff = m_max - m_min;
    qreal center = diff / 2.0f + m_min + m_pan;
    diff /= m_zoom;
    qreal min = center - diff / 2.0f;
    qreal max = center + diff / 2.0f;
    if (!QtPrivate::fuzzyCompare(m_visualMin, min)) {
        m_visualMin = min;
        emit q->visualMinChanged(min);
    }
    if (!QtPrivate::fuzzyCompare(m_visualMax, max)) {
        m_visualMax = max;
        emit q->visualMaxChanged(max);
    }
}

QT_END_NAMESPACE

#include "moc_qvalueaxis.cpp"
