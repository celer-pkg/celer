// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qdebug.h>
#include "qabstract3daxis_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QAbstract3DAxis
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QAbstract3DAxis class is a base class for the axes of a 3D graph.
 *
 * This class specifies the enumerations, properties, and functions shared by
 * graph axes. It should not be used directly, but one of its subclasses should
 * be used instead.
 *
 * \sa QCategory3DAxis, QValue3DAxis
 */

/*!
 * \qmltype Abstract3DAxis
 * \qmlabstract
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QAbstract3DAxis
 * \brief A base type for the axes of a 3D graph.
 * For Abstract3DAxis enums, see \l QAbstract3DAxis::AxisOrientation and
 * \l{QAbstract3DAxis::AxisType}.
 */

/*!
 * \qmlproperty string Abstract3DAxis::title
 * The title for the axis.
 *
 * \sa titleVisible, titleFixed
 */

/*!
 * \qmlproperty list Abstract3DAxis::labels
 * The labels for the axis.
 * \note Setting this property for Value3DAxis does nothing, as it generates
 * labels automatically.
 */

/*!
 * \qmlproperty Abstract3DAxis.AxisOrientation Abstract3DAxis::orientation
 * The orientation of the axis.
 */

/*!
 * \qmlproperty Abstract3DAxis.AxisType Abstract3DAxis::type
 * The type of the axis.
 */

/*!
 * \qmlproperty real Abstract3DAxis::min
 *
 * The minimum value on the axis.
 * When setting this property, the maximum value is adjusted if necessary, so
 * the range remains valid.
 */

/*!
 * \qmlproperty real Abstract3DAxis::max
 *
 * The maximum value on the axis.
 * When setting this property, the minimum value is adjusted if necessary, so
 * the range remains valid.
 */

/*!
 * \qmlproperty bool Abstract3DAxis::autoAdjustRange
 *
 * Defines whether the axis will automatically adjust the range so that all data
 * fits in it.
 */

/*!
 * \qmlproperty real Abstract3DAxis::labelAutoAngle
 *
 * The maximum angle the labels can autorotate when the camera angle changes.
 * The angle can be between 0 and 90, inclusive. The default value is 0.
 * If the value is 0, axis labels do not automatically rotate.
 * If the value is greater than zero, labels attempt to orient themselves toward
 * the camera, up to the specified angle.
 */

/*!
 * \qmlproperty bool Abstract3DAxis::titleVisible
 *
 * Defines whether the axis title is visible in the primary graph view.
 *
 * The default value is \c{false}.
 *
 * \sa title, titleFixed
 */

/*!
 * \qmlproperty bool Abstract3DAxis::labelsVisible
 *
 * Defines whether the axis labels are visible in the primary graph view.
 *
 * The default value is \c{true}.
 *
 */

/*!
 * \qmlproperty bool Abstract3DAxis::titleFixed
 *
 * The rotation of axis titles.
 *
 * If \c{false}, axis titles in the primary graph view will be rotated towards
 * the camera similarly to the axis labels. If \c{true}, axis titles are only
 * rotated around their axis but are not otherwise oriented towards the camera.
 * This property does not have any effect if the labelAutoAngle property
 * value is zero.
 * Default value is \c{true}.
 *
 * \sa labelAutoAngle, title, titleVisible
 */

/*!
 * \qmlproperty real Abstract3DAxis::titleOffset
 *
 * The position of the axis title on the axis.
 * The value must be between \c -1.0f and \c 1.0f
 *
 * Default value is \c{0}.
 *
 * \sa title, titleVisible
 */

/*!
 * \qmlproperty bool Abstract3DAxis::scaleLabelsByCount
 * \since 6.9
 *
 * Whether the labels will automatically adjust their size based on the total amount of labels.
 *
 * \sa labelSize
 */

/*!
 * \qmlproperty real Abstract3DAxis::labelSize
 * \since 6.9
 *
 * The size of the label.
 * The default for this value is \c 1.0.
 *
 * \sa scaleLabelsByCount
 */

/*!
    \qmlsignal Abstract3DAxis::titleChanged(string newTitle)

    This signal is emitted when \l title changes to \a newTitle.
*/
/*!
    \qmlsignal Abstract3DAxis::labelsChanged()

    This signal is emitted when axis labels change.
*/
/*!
    \qmlsignal Abstract3DAxis::orientationChanged(AxisOrientation orientation)

    This signal is emitted when axis orientation changes to \a orientation.
*/
/*!
    \qmlsignal Abstract3DAxis::minChanged(real value)

    This signal is emitted when the minimum value of the axis changes
    to \a value.
*/
/*!
    \qmlsignal Abstract3DAxis::maxChanged(real value)

    This signal is emitted when the maximum value of the axis changes
    to \a value.
*/
/*!
    \qmlsignal Abstract3DAxis::rangeChanged(real min, real max)

    This signal is emitted when the axis range changes. \a min and \a max are
    the min and max of the new range.
*/
/*!
    \qmlsignal Abstract3DAxis::autoAdjustRangeChanged(bool autoAdjust)

    This signal is emitted when the \l autoAdjustRange property value changes
    to \a autoAdjust.
*/
/*!
    \qmlsignal Abstract3DAxis::labelAutoAngleChanged(real angle)

    This signal is emitted when the angle of label rotation changes to
    \a angle.
*/
/*!
    \qmlsignal Abstract3DAxis::titleVisibilityChanged(bool visible)

    This signal is emitted when the title visibility changes to \a visible.
*/
/*!
    \qmlsignal Abstract3DAxis::labelVisibilityChanged(bool visible)

    This signal is emitted when the label visibility changes to \a visible.
*/
/*!
    \qmlsignal Abstract3DAxis::titleFixedChanged(bool fixed)

    This signal is emitted when the titleFixed property value changes to
    \a fixed.
*/
/*!
    \qmlsignal Abstract3DAxis::titleOffsetChanged(real offset)

    This signal is emitted when the titleOffset property value changes to
    \a offset.
*/
/*!
    \qmlsignal Abstract3DAxis::scaleLabelsByCountChanged(bool adjust)

    This signal is emitted when the scaleLabelsByCount property value changes to
    \a adjust.
*/
/*!
    \qmlsignal Abstract3DAxis::labelSizeChanged(real size)

    This signal is emitted when the labelSize property value changes to
    \a size.
*/

/*!
 * \enum QAbstract3DAxis::AxisOrientation
 *
 * The orientation of the axis object.
 *
 * \value None
 * \value X
 * \value Y
 * \value Z
 */

/*!
 * \enum QAbstract3DAxis::AxisType
 *
 * The type of the axis object.
 *
 * \value None
 * \value Category
 * \value Value
 */

/*!
 * \internal
 */
QAbstract3DAxis::QAbstract3DAxis(QAbstract3DAxisPrivate &d, QObject *parent)
    : QObject(d, parent)

{}

/*!
 * Destroys QAbstract3DAxis.
 */
QAbstract3DAxis::~QAbstract3DAxis() {}

/*!
 * \property QAbstract3DAxis::orientation
 *
 * \brief The orientation of the axis.
 *
 * The value is one of AxisOrientation values.
 */
QAbstract3DAxis::AxisOrientation QAbstract3DAxis::orientation() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_orientation;
}

/*!
 * \property QAbstract3DAxis::type
 *
 * \brief The type of the axis.
 *
 * The value is one of AxisType values.
 */
QAbstract3DAxis::AxisType QAbstract3DAxis::type() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_type;
}

/*!
 * \property QAbstract3DAxis::title
 *
 * \brief The title for the axis.
 *
 * \sa titleVisible, titleFixed
 */
void QAbstract3DAxis::setTitle(const QString &title)
{
    Q_D(QAbstract3DAxis);
    if (d->m_title == title) {
        qCDebug(lcAProperties3D, "%s title is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(title));
        return;
    }

    d->m_title = title;
    emit titleChanged(title);
}

QString QAbstract3DAxis::title() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_title;
}

/*!
 * \property QAbstract3DAxis::labels
 *
 * \brief The labels for the axis.
 * \note Setting this property for QValue3DAxis does nothing, as it generates
 * labels automatically.
 */
void QAbstract3DAxis::setLabels(const QStringList &labels)
{
    Q_UNUSED(labels);
}

QStringList QAbstract3DAxis::labels() const
{
    Q_D(const QAbstract3DAxis);
    const_cast<QAbstract3DAxisPrivate *>(d)->updateLabels();
    return const_cast<QAbstract3DAxisPrivate *>(d)->m_labels;
}

/*!
 * Sets the value range of the axis from \a min to \a max.
 * When setting the range, the maximum value is adjusted if necessary, to ensure
 * that the range remains valid.
 * \note For QCategory3DAxis, specifies the index range of rows or columns to
 * show.
 */
void QAbstract3DAxis::setRange(float min, float max)
{
    Q_D(QAbstract3DAxis);
    d->setRange(min, max);
    setAutoAdjustRange(false);
}

/*!
 * \property QAbstract3DAxis::labelAutoAngle
 *
 * \brief The maximum angle the labels can autorotate when the camera angle
 * changes.
 *
 * The angle can be between 0 and 90, inclusive. The default value is 0.
 * If the value is 0, axis labels do not automatically rotate.
 * If the value is greater than zero, labels attempt to orient themselves toward
 * the camera, up to the specified angle.
 */
void QAbstract3DAxis::setLabelAutoAngle(float degree)
{
    Q_D(QAbstract3DAxis);
    if (degree < 0.0f) {
        qCWarning(lcAProperties3D, "%s invalid angle degree."
                  "degree has been set to 0.0f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        degree = 0.0f;
    }

    if (degree > 90.0f) {
        qCWarning(lcAProperties3D, "%s invalid angle degree."
                  "degree has been set to 90.0f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        degree = 90.0f;
    }

    if (qFuzzyCompare(d->m_labelAutoAngle + 1, degree + 1)) {
        qCDebug(lcAProperties3D, "%s angle value is already: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), degree);
        return;
    }

    d->m_labelAutoAngle = degree;
    emit labelAutoAngleChanged(degree);
}

float QAbstract3DAxis::labelAutoAngle() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_labelAutoAngle;
}

/*!
 * \property QAbstract3DAxis::titleVisible
 *
 * \brief Whether the axis title is visible in the primary graph view.
 *
 * The default value is \c{false}.
 *
 * \sa title, titleFixed
 */
void QAbstract3DAxis::setTitleVisible(bool visible)
{
    Q_D(QAbstract3DAxis);
    if (d->m_titleVisible == visible) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "visibility is already set to:" << visible;
        return;
    }

    d->m_titleVisible = visible;
    emit titleVisibleChanged(visible);
}

bool QAbstract3DAxis::isTitleVisible() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_titleVisible;
}

/*!
 * \property QAbstract3DAxis::labelsVisible
 *
 * \brief Whether the axis labels are visible in the primary graph view.
 *
 * The default value is \c{true}.
 *
 */
void QAbstract3DAxis::setLabelsVisible(bool visible)
{
    Q_D(QAbstract3DAxis);
    if (d->m_labelsVisible == visible) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "visibility is already set to:" << visible;
        return;
    }

    d->m_labelsVisible = visible;
    emit labelVisibleChanged(visible);
}

bool QAbstract3DAxis::labelsVisible() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_labelsVisible;
}

/*!
 * \property QAbstract3DAxis::titleFixed
 *
 * \brief The rotation of the axis titles.
 *
 * If \c{false}, axis titles in the primary graph view will be rotated towards
 * the camera similarly to the axis labels. If \c{true}, axis titles are only
 * rotated around their axis but are not otherwise oriented towards the camera.
 * This property does not have any effect if the labelAutoAngle property
 * value is zero.
 * Default value is \c{true}.
 *
 * \sa labelAutoAngle, title, titleVisible
 */
void QAbstract3DAxis::setTitleFixed(bool fixed)
{
    Q_D(QAbstract3DAxis);
    if (d->m_titleFixed == fixed) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to:" << fixed;
        return;
    }

    d->m_titleFixed = fixed;
    emit titleFixedChanged(fixed);
}

bool QAbstract3DAxis::isTitleFixed() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_titleFixed;
}

/*!
 * \property QAbstract3DAxis::min
 *
 * \brief The minimum value on the axis.
 *
 * When setting this property, the maximum value is adjusted if necessary, to
 * ensure that the range remains valid.
 * \note For QCategory3DAxis, specifies the index of the first row or column to
 * show.
 */
void QAbstract3DAxis::setMin(float min)
{
    Q_D(QAbstract3DAxis);
    d->setMin(min);
    setAutoAdjustRange(false);
}

/*!
 * \property QAbstract3DAxis::max
 *
 * \brief The maximum value on the axis.
 *
 * When setting this property, the minimum value is adjusted if necessary, to
 * ensure that the range remains valid.
 * \note For QCategory3DAxis, specifies the index of the last row or column to
 * show.
 */
void QAbstract3DAxis::setMax(float max)
{
    Q_D(QAbstract3DAxis);
    d->setMax(max);
    setAutoAdjustRange(false);
}

float QAbstract3DAxis::min() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_min;
}

float QAbstract3DAxis::max() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_max;
}

/*!
 * \property QAbstract3DAxis::autoAdjustRange
 *
 * \brief Whether the axis will automatically adjust the range so that all data
 * fits in it.
 *
 * \sa setRange(), setMin(), setMax()
 */
void QAbstract3DAxis::setAutoAdjustRange(bool autoAdjust)
{
    Q_D(QAbstract3DAxis);
    if (d->m_autoAdjust == autoAdjust) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to:" << autoAdjust;
        return;
    }

    d->m_autoAdjust = autoAdjust;
    emit autoAdjustRangeChanged(autoAdjust);
}

bool QAbstract3DAxis::isAutoAdjustRange() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_autoAdjust;
}

/*!
 * \property QAbstract3DAxis::scaleLabelsByCount
 * \since 6.9
 * \brief Whether the labels will automatically adjust their size
 * based on the total amount of labels
 *
 * \sa labelSize()
 */
void QAbstract3DAxis::setScaleLabelsByCount(bool adjust)
{
    Q_D(QAbstract3DAxis);
    if (d->m_scaleLabelsByCount == adjust) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to:" << adjust;
        return;
    }

    d->m_scaleLabelsByCount = adjust;
    emit scaleLabelsByCountChanged(adjust);

}

bool QAbstract3DAxis::isScaleLabelsByCount() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_scaleLabelsByCount;
}

/*!
 * \property QAbstract3DAxis::labelSize
 * \since 6.9
 * \brief Size of the label
 *
 * The size of the label.
 * The default for this value is \c 1.0.
 *
 * \sa scaleLabelsByCount
 */
void QAbstract3DAxis::setLabelSize(qreal size)
{
    Q_D(QAbstract3DAxis);
    if (QtPrivate::fuzzyCompare(d->m_labelSize, size)) {
        qCDebug(lcAProperties3D, "%s Value is already set to: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), size);
        return;
    }

    d->m_labelSize = size;
    emit labelSizeChanged(size);
}

qreal QAbstract3DAxis::labelSize() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_labelSize;
}

/*!
 * \property QAbstract3DAxis::titleOffset
 *
 * The position of the axis title on the axis.
 * The value must be between \c -1.0f and \c 1.0f
 *
 * \sa title, titleFixed
 */
void QAbstract3DAxis::setTitleOffset(float offset)
{
    Q_D(QAbstract3DAxis);
    if (QtPrivate::fuzzyCompare(d->m_titleOffset, offset)) {
        qCDebug(lcAProperties3D, "%s offset value is already set to: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), offset);
        return;
    } else if (offset < -1.0f || offset > 1.0f) {
        qCWarning(lcAProperties3D, "%s invalid value. Valid range for title offset is between "
                 "-1.0f and 1.0f", qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    d->m_titleOffset = offset;
    emit titleOffsetChanged(offset);
}

float QAbstract3DAxis::titleOffset() const
{
    Q_D(const QAbstract3DAxis);
    return d->m_titleOffset;
}

/*!
 * \fn QAbstract3DAxis::rangeChanged(float min, float max)
 *
 * Emits the minimum and maximum values of the range, \a min and \a max, when
 * the range changes.
 */

// QAbstract3DAxisPrivate
QAbstract3DAxisPrivate::QAbstract3DAxisPrivate(QAbstract3DAxis::AxisType type)
    : m_orientation(QAbstract3DAxis::AxisOrientation::None)
    , m_type(type)
    , m_isDefaultAxis(false)
    , m_min(0.0f)
    , m_max(10.0f)
    , m_autoAdjust(true)
    , m_scaleLabelsByCount(false)
    , m_labelSize(1.0)
    , m_labelAutoAngle(0.0f)
    , m_titleOffset(0.0f)
    , m_titleVisible(false)
    , m_labelsVisible(true)
    , m_titleFixed(true)

{}

QAbstract3DAxisPrivate::~QAbstract3DAxisPrivate() {}

void QAbstract3DAxisPrivate::setOrientation(QAbstract3DAxis::AxisOrientation orientation)
{
    Q_Q(QAbstract3DAxis);
    if (m_orientation == QAbstract3DAxis::AxisOrientation::None) {
        m_orientation = orientation;
        emit q->orientationChanged(orientation);
    } else {
        Q_ASSERT("Attempted to reset axis orientation.");
    }
}

void QAbstract3DAxisPrivate::updateLabels()
{
    // Default implementation does nothing
}

void QAbstract3DAxisPrivate::setRange(float min, float max, bool suppressWarnings)
{
    Q_Q(QAbstract3DAxis);
    bool adjusted = false;
    if (!allowNegatives()) {
        if (allowZero()) {
            if (min < 0.0f) {
                min = 0.0f;
                adjusted = true;
            }
            if (max < 0.0f) {
                max = 0.0f;
                adjusted = true;
            }
        } else {
            if (min <= 0.0f) {
                min = 1.0f;
                adjusted = true;
            }
            if (max <= 0.0f) {
                max = 1.0f;
                adjusted = true;
            }
        }
    }
    // If min >= max, we adjust ranges so that
    // m_max becomes (min + 1.0f)
    // as axes need some kind of valid range.
    bool minDirty = false;
    bool maxDirty = false;
    if (m_min != min) {
        m_min = min;
        minDirty = true;
    }
    if (m_max != max || min > max || (!allowMinMaxSame() && min == max)) {
        if (min > max || (!allowMinMaxSame() && min == max)) {
            m_max = min + 1.0f;
            adjusted = true;
        } else {
            m_max = max;
        }
        maxDirty = true;
    }

    if (minDirty || maxDirty) {
        if (adjusted && !suppressWarnings) {
            qCWarning(lcAProperties3D, "%s tried to set invalid range for axis. Range automatically adjusted "
                     "to a valid one: %f - %f --> %f - %f",
                     qUtf8Printable(QLatin1String(__FUNCTION__)),
                     min,
                     max,
                     m_min,
                     m_max);
        }
        emit q->rangeChanged(m_min, m_max);
    }

    if (minDirty)
        emit q->minChanged(m_min);
    if (maxDirty)
        emit q->maxChanged(m_max);
}

void QAbstract3DAxisPrivate::setMin(float min)
{
    Q_Q(QAbstract3DAxis);
    if (!allowNegatives()) {
        if (allowZero()) {
            if (min < 0.0f) {
                min = 0.0f;
                qCWarning(lcAProperties3D, "%s tried to set negative minimum for an axis that only"
                         "supports positive values and zero: %f",
                         qUtf8Printable(QLatin1String(__FUNCTION__)),
                         min);
            }
        } else {
            if (min <= 0.0f) {
                min = 1.0f;
                qCWarning(lcAProperties3D, "%s tried to set negative or zero minimum for an "
                         "axis that only"
                         "supports positive values: %f",
                         qUtf8Printable(QLatin1String(__FUNCTION__)),
                         min);
            }
        }
    }

    if (m_min != min) {
        bool maxChanged = false;
        if (min > m_max || (!allowMinMaxSame() && min == m_max)) {
            float oldMax = m_max;
            m_max = min + 1.0f;
            qCWarning(lcAProperties3D, "%s tried to set minimum to equal or larger than maximum for"
                     " value axis. Maximum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__FUNCTION__)),
                     oldMax,
                     m_max);
            maxChanged = true;
        }
        m_min = min;

        emit q->rangeChanged(m_min, m_max);
        emit q->minChanged(m_min);
        if (maxChanged)
            emit q->maxChanged(m_max);
    }
}

void QAbstract3DAxisPrivate::setMax(float max)
{
    Q_Q(QAbstract3DAxis);
    if (!allowNegatives()) {
        if (allowZero()) {
            if (max < 0.0f) {
                max = 0.0f;
                qCWarning(lcAProperties3D, "%s tried to set negative maximum for an axis that only"
                         " supports positive values and zero: %f",
                         qUtf8Printable(QLatin1String(__FUNCTION__)),
                         max);
            }
        } else {
            if (max <= 0.0f) {
                max = 1.0f;
                qCWarning(lcAProperties3D, "%s tried to set negative or zero maximum for an "
                         "axis that only"
                         " supports positive values: %f",
                         qUtf8Printable(QLatin1String(__FUNCTION__)),
                         max);
            }
        }
    }

    if (m_max != max) {
        bool minChanged = false;
        if (m_min > max || (!allowMinMaxSame() && m_min == max)) {
            float oldMin = m_min;
            m_min = max - 1.0f;
            if (!allowNegatives() && m_min < 0.0f) {
                if (allowZero())
                    m_min = 0.0f;
                else
                    m_min = max / 2.0f; // Need some positive value smaller than max

                if (!allowMinMaxSame() && max == 0.0f) {
                    m_min = oldMin;
                    qCWarning(lcAProperties3D, "%s unable to set maximum value to zero.",
                              qUtf8Printable(QLatin1String(__FUNCTION__)));
                    return;
                }
            }
            qCWarning(lcAProperties3D, "%s tried to set maximum to equal or smaller than minimum "
                     "for value axis. Minimum automatically adjusted to a valid one: %f --> %f",
                     qUtf8Printable(QLatin1String(__FUNCTION__)),
                     oldMin,
                     m_min);
            minChanged = true;
        }
        m_max = max;
        emit q->rangeChanged(m_min, m_max);
        emit q->maxChanged(m_max);
        if (minChanged)
            emit q->minChanged(m_min);
    }
}

QT_END_NAMESPACE

#include "moc_qabstract3daxis.cpp"
