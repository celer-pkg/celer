// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qvalue3daxis_p.h"
#include "qvalue3daxisformatter_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QValue3DAxisFormatter
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QValue3DAxisFormatter class is a base class for 3D value axis
 * formatters.
 *
 * This class provides formatting rules for a linear value 3D axis. Subclass it
 * if you want to implement custom value axes.
 *
 * The base class has no public API beyond constructors and destructors. It is
 * meant to be only used internally. However, subclasses may implement public
 * properties as needed.
 *
 * \sa QValue3DAxis, QLogValue3DAxisFormatter
 */

/*!
 * \qmltype Value3DAxisFormatter
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QValue3DAxisFormatter
 * \brief A base type for 3D value axis formatters.
 *
 * This type provides formatting rules for a linear value 3D axis.
 * This type is the default type for Value3DAxis and thus never needs to be
 * explicitly created. This type has no public functionality.
 *
 * \sa Value3DAxis
 */

/*!
 * \internal
 */
QValue3DAxisFormatter::QValue3DAxisFormatter(QValue3DAxisFormatterPrivate &d, QObject *parent)
    : QObject(d, parent)
{}

/*!
 * Constructs a new value 3D axis formatter with the optional parent \a parent.
 */
QValue3DAxisFormatter::QValue3DAxisFormatter(QObject *parent)
    : QObject(*(new QValue3DAxisFormatterPrivate()), parent)
{}

/*!
 * Deletes the value 3D axis formatter.
 */
QValue3DAxisFormatter::~QValue3DAxisFormatter() {}

/*!
 * Allows the parent axis to have negative values if \a allow is \c true.
 */
void QValue3DAxisFormatter::setAllowNegatives(bool allow)
{
    Q_D(QValue3DAxisFormatter);
    if (d->m_allowNegatives == allow) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to: " << allow;
        return;
    }

    d->m_allowNegatives = allow;
}

/*!
 * Returns \c true if negative values are valid values for the parent axis.
 * The default implementation always returns \c true.
 */
bool QValue3DAxisFormatter::allowNegatives() const
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_allowNegatives;
}

/*!
 * Allows the parent axis to have a zero value if \a allow is \c true.
 */
void QValue3DAxisFormatter::setAllowZero(bool allow)
{
    Q_D(QValue3DAxisFormatter);
    if (d->m_allowZero == allow) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "value is already set to: " << allow;
        return;
    }

    d->m_allowZero = allow;
}

/*!
 * Returns \c true if zero is a valid value for the parent axis.
 * The default implementation always returns \c true.
 */
bool QValue3DAxisFormatter::allowZero() const
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_allowZero;
}

/*!
 * \internal
 */
void QValue3DAxisFormatter::setAxis(QValue3DAxis *axis)
{
    Q_ASSERT(axis);
    Q_D(QValue3DAxisFormatter);

    // These signals are all connected to markDirtyNoLabelChange slot, even though
    // most of them do require labels to be regenerated. This is because the label
    // regeneration is triggered elsewhere in these cases.
    connect(axis,
            &QValue3DAxis::segmentCountChanged,
            this,
            &QValue3DAxisFormatter::markDirtyNoLabelChange);
    connect(axis,
            &QValue3DAxis::subSegmentCountChanged,
            this,
            &QValue3DAxisFormatter::markDirtyNoLabelChange);
    connect(axis,
            &QValue3DAxis::labelFormatChanged,
            this,
            &QValue3DAxisFormatter::markDirtyNoLabelChange);
    connect(axis,
            &QAbstract3DAxis::rangeChanged,
            this,
            &QValue3DAxisFormatter::markDirtyNoLabelChange);

    d->setAxis(axis);
}

/*!
 * \internal
 */
void QValue3DAxisFormatter::markDirtyNoLabelChange()
{
    Q_D(QValue3DAxisFormatter);
    d->markDirty(false);
}

/*!
 * Creates a new empty value 3D axis formatter. Must be reimplemented in a
 * subclass.
 *
 * Returns the new formatter. The renderer uses this method to cache a copy of
 * the formatter. The ownership of the new copy is transferred to the caller.
 */
QValue3DAxisFormatter *QValue3DAxisFormatter::createNewInstance() const
{
    return new QValue3DAxisFormatter();
}

/*!
 * Resizes and populates the label and grid line position arrays and the label
 * strings array, as well as calculates any values needed to map a value to its
 * position. The parent axis can be accessed from inside this function.
 *
 * This method must be reimplemented in a subclass if the default array contents
 * are not suitable.
 *
 * See gridPositions(), subGridPositions(), labelPositions(), and labelStrings()
 * methods for documentation about the arrays that need to be resized and
 * populated.
 *
 * \sa gridPositions(), subGridPositions(), labelPositions(), labelStrings(),
 * axis()
 */
void QValue3DAxisFormatter::recalculate()
{
    Q_D(QValue3DAxisFormatter);
    d->doRecalculate();
}

/*!
 * Returns the formatted label string using the specified \a value and
 * \a format.
 *
 * Reimplement this method in a subclass to resolve the formatted string for a
 * given \a value if the default formatting rules specified for
 * QValue3DAxis::labelFormat property are not sufficient.
 *
 * \sa recalculate(), labelStrings(), QValue3DAxis::labelFormat
 */
QString QValue3DAxisFormatter::stringForValue(qreal value, const QString &format)
{
    Q_D(QValue3DAxisFormatter);
    return d->stringForValue(value, format);
}

/*!
 * Returns the normalized position along the axis for the given \a value.
 * The returned value should be between \c 0.0 (the minimum value) and
 * \c 1.0 (the maximum value), inclusive, if the value is within the parent
 * axis range.
 *
 * Reimplement this method if the position cannot be resolved by linear
 * interpolation between the parent axis minimum and maximum values.
 *
 * \sa recalculate(), valueAt()
 */
float QValue3DAxisFormatter::positionAt(float value) const
{
    Q_D(const QValue3DAxisFormatter);
    return d->positionAt(value);
}

/*!
 * Returns the value at the normalized \a position along the axis.
 * The \a position value should be between \c 0.0 (the minimum value) and
 * \c 1.0 (the maximum value), inclusive, to obtain values within the parent
 * axis range.
 *
 * Reimplement this method if the value cannot be resolved by linear
 * interpolation between the parent axis minimum and maximum values.
 *
 * \sa recalculate(), positionAt()
 */
float QValue3DAxisFormatter::valueAt(float position) const
{
    Q_D(const QValue3DAxisFormatter);
    return d->valueAt(position);
}

/*!
 * Copies all the values necessary for resolving positions, values, and strings
 * with this formatter to the \a copy of the formatter. When reimplementing
 * this method in a subclass, call the superclass version at some point.
 * The renderer uses this method to cache a copy of the formatter.
 *
 * Returns the new copy. The ownership of the new copy transfers to the caller.
 */
void QValue3DAxisFormatter::populateCopy(QValue3DAxisFormatter &copy)
{
    Q_D(QValue3DAxisFormatter);
    d->doPopulateCopy(*(copy.d_func()));
}

/*!
 * Marks this formatter as dirty, prompting the renderer to make a new copy of its
 * cache on the next renderer synchronization. This method should be called by a
 * subclass whenever the formatter is changed in a way that affects the resolved
 * values. Set \a labelsChange to \c true if the change requires regenerating
 * the parent axis label strings.
 */
void QValue3DAxisFormatter::markDirty(bool labelsChange)
{
    Q_D(QValue3DAxisFormatter);
    d->markDirty(labelsChange);
}

/*!
 * Returns the parent axis. The parent axis must only be accessed in the
 * recalculate() method to maintain thread safety in environments using a
 * threaded renderer.
 *
 * \sa recalculate()
 */
QValue3DAxis *QValue3DAxisFormatter::axis() const
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_axis;
}

/*!
 * Returns a reference to the array of normalized grid line positions.
 * The default array size is equal to the segment count of the parent axis plus
 * one, but a subclassed implementation of the recalculate() method may resize
 * the array differently.
 *
 * \sa QValue3DAxis::segmentCount, recalculate()
 */
const QList<float> &QValue3DAxisFormatter::gridPositions() const &
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_gridPositions;
}

/*!
 * Returns an array of normalized grid line positions by value.
 * The default array size is equal to the segment count of the parent axis plus
 * one, but a subclassed implementation of the recalculate() method may resize
 * the array differently.
 *
 * \sa QValue3DAxis::segmentCount, recalculate()
 */
QList<float> QValue3DAxisFormatter::gridPositions() &&
{
    Q_D(QValue3DAxisFormatter);
    return std::move(d->m_gridPositions);
}

/*!
 * Sets a list of new grid positions from \a gridPositions.
 * The values should be between \c 0.0 (the minimum value) and \c 1.0 (the
 * maximum value), inclusive.
 *
 * \sa QValue3DAxis::segmentCount, recalculate(), gridPositions
 */
void QValue3DAxisFormatter::setGridPoitions(QList<float> gridPositions)
{
    Q_D(QValue3DAxisFormatter);
    d->m_gridPositions = gridPositions;
}

/*!
 * Returns a reference to the array of normalized sub-grid line positions.
 * The default array size is equal to the segment count of the parent axis times
 * the sub-segment count of the parent axis minus one, but a subclassed
 * implementation of the recalculate() method may resize the array differently.
 *
 * \sa QValue3DAxis::segmentCount, QValue3DAxis::subSegmentCount, recalculate()
 */
const QList<float> &QValue3DAxisFormatter::subGridPositions() const &
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_subGridPositions;
}

/*!
 * Returns an array of normalized sub-grid line positions by value.
 * The default array size is equal to the segment count of the parent axis times
 * the sub-segment count of the parent axis minus one, but a subclassed
 * implementation of the recalculate() method may resize the array differently.
 *
 * \sa QValue3DAxis::segmentCount, QValue3DAxis::subSegmentCount, recalculate()
 */
QList<float> QValue3DAxisFormatter::subGridPositions() &&
{
    Q_D(QValue3DAxisFormatter);
    return std::move(d->m_subGridPositions);
}

/*!
 * Sets a list of new sub-grid positions from \a subGridPositions.
 * The values should be between \c 0.0 (the minimum value) and \c 1.0 (the
 * maximum value), inclusive.
 *
 * \sa QValue3DAxis::segmentCount, QValue3DAxis::subSegmentCount, recalculate(),
 * subGridPositions
 */
void QValue3DAxisFormatter::setSubGridPositions(QList<float> subGridPositions)
{
    Q_D(QValue3DAxisFormatter);
    d->m_subGridPositions = subGridPositions;
}

/*!
 * Returns a reference to the array of normalized label positions.
 * The default array size is equal to the segment count of the parent axis plus
 * one, but a subclassed implementation of the recalculate() method may resize
 * the array differently. By default, the label at the index zero corresponds to
 * the minimum value of the axis.
 *
 * \sa QValue3DAxis::segmentCount, QAbstract3DAxis::labels, recalculate()
 */
const QList<float> &QValue3DAxisFormatter::labelPositions() const &
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_labelPositions;
}

/*!
 * Returns an array of normalized label positions by value.
 * The default array size is equal to the segment count of the parent axis plus
 * one, but a subclassed implementation of the recalculate() method may resize
 * the array differently. By default, the label at the index zero corresponds to
 * the minimum value of the axis.
 *
 * \sa QValue3DAxis::segmentCount, QAbstract3DAxis::labels, recalculate()
 */
QList<float> QValue3DAxisFormatter::labelPositions() &&
{
    Q_D(QValue3DAxisFormatter);
    return std::move(d->m_labelPositions);
}

/*!
 * Sets a list of new label positions from \a labelPositions.
 * The values should be between \c 0.0 (the minimum value) and
 * \c 1.0 (the maximum value), inclusive.
 *
 * \sa QValue3DAxis::segmentCount, QAbstract3DAxis::labels, recalculate(),
 * labelPositions()
 */
void QValue3DAxisFormatter::setlabelPositions(QList<float> labelPositions)
{
    Q_D(QValue3DAxisFormatter);
    d->m_labelPositions = labelPositions;
}

/*!
 * Returns a reference to the string list containing formatter label strings.
 *
 * \sa labelPositions()
 */
const QStringList &QValue3DAxisFormatter::labelStrings() const &
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_labelStrings;
}

/*!
 * Returns a string list by value containing formatter label strings.
 *
 * \sa labelPositions()
 */
QStringList QValue3DAxisFormatter::labelStrings() &&
{
    Q_D(QValue3DAxisFormatter);
    return std::move(d->m_labelStrings);
}

/*!
 * Sets a list of new label strings from \a labelStrings.
 * The array size must be equal to the size of the label positions array, which
 * the indexes also correspond to.
 * \sa labelPositions(), labelStrings()
 */
void QValue3DAxisFormatter::setLabelStrings(QStringList labelStrings)
{
    Q_D(QValue3DAxisFormatter);
    d->m_labelStrings = labelStrings;
}

/*!
 * Sets the \a locale that this formatter uses.
 * The graph automatically sets the formatter's locale to a graph's locale
 * whenever the parent axis is set as an active axis of the graph, the axis
 * formatter is set to an axis attached to the graph, or the graph's locale
 * changes.
 *
 * \sa locale(), Q3DGraphsWidgetItem::locale
 */
void QValue3DAxisFormatter::setLocale(const QLocale &locale)
{
    Q_D(QValue3DAxisFormatter);
    d->m_cLocaleInUse = (locale == QLocale::c());
    d->m_locale = locale;
    markDirty(true);
}
/*!
 * Returns the current locale this formatter is using.
 */
QLocale QValue3DAxisFormatter::locale() const
{
    Q_D(const QValue3DAxisFormatter);
    return d->m_locale;
}

// QValue3DAxisFormatterPrivate
QValue3DAxisFormatterPrivate::QValue3DAxisFormatterPrivate()
    : m_needsRecalculate(true)
    , m_min(0.0f)
    , m_max(0.0f)
    , m_rangeNormalizer(0.0f)
    , m_axis(0)
    , m_preparsedParamType(Utils::ParamType::Unknown)
    , m_allowNegatives(true)
    , m_allowZero(true)
    , m_formatPrecision(6)
    , // 6 and 'g' are defaults in Qt API for format precision and spec
    m_formatSpec('g')
    , m_cLocaleInUse(true)
{}

QValue3DAxisFormatterPrivate::~QValue3DAxisFormatterPrivate() {}

void QValue3DAxisFormatterPrivate::recalculate()
{
    Q_Q(QValue3DAxisFormatter);
    // Only recalculate if we need to and have m_axis pointer. If we do not have
    // m_axis, either we are not attached to an axis or this is a renderer cache.
    if (m_axis && m_needsRecalculate) {
        m_min = m_axis->min();
        m_max = m_axis->max();
        m_rangeNormalizer = (m_max - m_min);

        q->recalculate();
        m_needsRecalculate = false;
    }
}

void QValue3DAxisFormatterPrivate::doRecalculate()
{
    Q_Q(QValue3DAxisFormatter);
    int segmentCount = m_axis->segmentCount();
    int subGridCount = m_axis->subSegmentCount() - 1;
    QString labelFormat = m_axis->labelFormat();

    m_gridPositions.resize(segmentCount + 1);
    m_subGridPositions.resize(segmentCount * subGridCount);

    m_labelPositions.resize(segmentCount + 1);
    m_labelStrings.clear();
    m_labelStrings.reserve(segmentCount + 1);

    // Use qreals for intermediate calculations for better accuracy on label
    // values
    qreal segmentStep = 1.0 / qreal(segmentCount);
    qreal subSegmentStep = 0;
    if (subGridCount > 0)
        subSegmentStep = segmentStep / qreal(subGridCount + 1);

    // Calculate positions
    qreal rangeNormalizer = qreal(m_max - m_min);
    for (int i = 0; i < segmentCount; i++) {
        qreal gridValue = segmentStep * qreal(i);
        m_gridPositions[i] = float(gridValue);
        m_labelPositions[i] = float(gridValue);
        m_labelStrings << q->stringForValue(gridValue * rangeNormalizer + qreal(m_min), labelFormat);
        if (m_subGridPositions.size()) {
            for (int j = 0; j < subGridCount; j++)
                m_subGridPositions[i * subGridCount + j] = gridValue + subSegmentStep * (j + 1);
        }
    }

    // Ensure max value doesn't suffer from any rounding errors
    m_gridPositions[segmentCount] = 1.0f;
    m_labelPositions[segmentCount] = 1.0f;
    m_labelStrings << q->stringForValue(qreal(m_max), labelFormat);
}

void QValue3DAxisFormatterPrivate::populateCopy(QValue3DAxisFormatter &copy)
{
    Q_Q(QValue3DAxisFormatter);
    recalculate();
    q->populateCopy(copy);
}

void QValue3DAxisFormatterPrivate::doPopulateCopy(QValue3DAxisFormatterPrivate &copy)
{
    copy.m_min = m_min;
    copy.m_max = m_max;
    copy.m_rangeNormalizer = m_rangeNormalizer;

    copy.m_gridPositions = m_gridPositions;
    copy.m_labelPositions = m_labelPositions;
    copy.m_subGridPositions = m_subGridPositions;
}

QString QValue3DAxisFormatterPrivate::stringForValue(qreal value, const QString &format)
{
    if (m_previousLabelFormat.compare(format)) {
        // Format string different than the previous one used, reparse it
        m_labelFormatArray = format.toUtf8();
        m_previousLabelFormat = format;
        m_preparsedParamType = Utils::preParseFormat(format,
                                                     m_formatPreStr,
                                                     m_formatPostStr,
                                                     m_formatPrecision,
                                                     m_formatSpec);
    }

    if (m_cLocaleInUse) {
        return Utils::formatLabelSprintf(m_labelFormatArray, m_preparsedParamType, value);
    } else {
        return Utils::formatLabelLocalized(m_preparsedParamType,
                                           value,
                                           m_locale,
                                           m_formatPreStr,
                                           m_formatPostStr,
                                           m_formatPrecision,
                                           m_formatSpec,
                                           m_labelFormatArray);
    }
}

float QValue3DAxisFormatterPrivate::positionAt(float value) const
{
    return ((value - m_min) / m_rangeNormalizer);
}

float QValue3DAxisFormatterPrivate::valueAt(float position) const
{
    return ((position * m_rangeNormalizer) + m_min);
}

void QValue3DAxisFormatterPrivate::setAxis(QValue3DAxis *axis)
{
    Q_ASSERT(axis);
    m_axis = axis;
}

void QValue3DAxisFormatterPrivate::markDirty(bool labelsChange)
{
    m_needsRecalculate = true;
    if (m_axis) {
        if (labelsChange)
            m_axis->d_func()->emitLabelsChanged();
        if (m_axis && m_axis->orientation() != QAbstract3DAxis::AxisOrientation::None)
            m_axis->d_func()->emitFormatterDirty();
    }
}

QT_END_NAMESPACE

#include "moc_qvalue3daxisformatter.cpp"
