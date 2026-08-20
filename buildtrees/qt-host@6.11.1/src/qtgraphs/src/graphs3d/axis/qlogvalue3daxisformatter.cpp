// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qlogvalue3daxisformatter_p.h"
#include "qvalue3daxis_p.h"
#include <QtCore/qmath.h>
#include <qgraphs3dlogging_p.h>

QT_BEGIN_NAMESPACE

/*!
 * \class QLogValue3DAxisFormatter
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QLogValue3DAxisFormatter class provides formatting rules for a
 * logarithmic value axis.
 *
 * When a formatter is attached to a value axis, the axis range
 * cannot include negative values or the zero.
 *
 * \sa QValue3DAxisFormatter
 */

/*!
 * \qmltype LogValue3DAxisFormatter
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QLogValue3DAxisFormatter
 * \inherits Value3DAxisFormatter
 * \brief Provides formatting rules for a logarithmic value axis.
 *
 * When a formatter is attached to a value axis, the axis range
 * cannot include negative values or the zero.
 */

/*!
 * \qmlproperty real LogValue3DAxisFormatter::base
 *
 * The base of the logarithm used to map axis values. If the base is non-zero,
 * the parent axis segment count will be ignored when the grid line and label
 * positions are calculated. If you want the range to be divided into equal
 * segments like a normal value axis, set this property value to zero.
 *
 * The base has to be zero or a positive value and it cannot be equal to one.
 * Defaults to ten.
 *
 * \sa Value3DAxis::segmentCount
 */

/*!
 * \qmlproperty bool LogValue3DAxisFormatter::autoSubGrid
 *
 * Defines whether sub-grid positions are generated automatically.
 *
 * If this property value is set to \c true, the parent axis sub-segment count
 * is ignored when calculating sub-grid line positions. The sub-grid positions
 * are generated automatically according to the \l base property value. The
 * number of sub-grid lines is set to the base value minus one, rounded down.
 * This property is ignored when the base value is zero.
 * Defaults to \c true.
 *
 * \sa base, Value3DAxis::subSegmentCount
 */

/*!
 * \qmlproperty bool LogValue3DAxisFormatter::edgeLabelsVisible
 *
 * Defines whether the first and last label on the axis are visible.
 *
 * When the \l base property value is non-zero, the whole axis range is often
 * not equally divided into
 * segments. The first and last segments are often smaller than the other
 * segments. In extreme cases, this can lead to overlapping labels on the first
 * and last two grid lines. By setting this property to \c false, you can
 * suppress showing the minimum and maximum labels for the axis in cases where
 * the segments do not exactly fit the axis. Defaults to \c true.
 *
 * \sa base, Abstract3DAxis::labels
 */

/*!
    \qmlsignal LogValue3DAxisFormatter::baseChanged(real base)

    This signal is emitted when the base of the logarithm used to map axis
    values changes to \a base.
*/

/*!
    \qmlsignal LogValue3DAxisFormatter::autoSubGridChanged(bool enabled)

    This signal is emitted when the value that specifies whether sub-grid
    positions are generated automatically changes to \a enabled.
*/

/*!
    \qmlsignal LogValue3DAxisFormatter::edgeLabelsVisibleChanged(bool enabled)

    This signal is emitted when the value that specifies whether to show
    the first and last label on the axis changes to \a enabled.
*/

/*!
 * \internal
 */
QLogValue3DAxisFormatter::QLogValue3DAxisFormatter(QLogValue3DAxisFormatterPrivate &d,
                                                   QObject *parent)
    : QValue3DAxisFormatter(d, parent)
{
    setAllowNegatives(false);
    setAllowZero(false);
}

/*!
 * Constructs a new logarithmic value 3D axis formatter with the optional
 * parent \a parent.
 */
QLogValue3DAxisFormatter::QLogValue3DAxisFormatter(QObject *parent)
    : QValue3DAxisFormatter(*(new QLogValue3DAxisFormatterPrivate()), parent)
{
    setAllowNegatives(false);
    setAllowZero(false);
}

/*!
 * Deletes the logarithmic value 3D axis formatter.
 */
QLogValue3DAxisFormatter::~QLogValue3DAxisFormatter() {}

/*!
 * \property QLogValue3DAxisFormatter::base
 *
 * \brief The base of the logarithm used to map axis values.
 *
 * If the base is non-zero, the parent axis
 * segment count will be ignored when the grid line and label positions are
 * calculated. If you want the range to be divided into equal segments like a
 * normal value axis, set this property value to zero.
 *
 * The base has to be zero or a positive value and it cannot be equal to one.
 * Defaults to ten.
 *
 * \sa QValue3DAxis::segmentCount
 */
void QLogValue3DAxisFormatter::setBase(qreal base)
{
    Q_D(QLogValue3DAxisFormatter);
    if (base < 0.0f || base == 1.0f) {
        qCWarning(lcAProperties3D,
                  "%s the logarithm base must be greater than 0 and not equal to 1, attempted: %.1f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)), base);
        return;
    }
    if (qFuzzyCompare(d->m_base + 1, base + 1)) {
        qCDebug(lcAProperties3D,
                "%s Base value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), base);
        return;
    }
    d->m_base = base;
    markDirty(true);
    emit baseChanged(base);
}

qreal QLogValue3DAxisFormatter::base() const
{
    Q_D(const QLogValue3DAxisFormatter);
    return d->m_base;
}

/*!
 * \property QLogValue3DAxisFormatter::autoSubGrid
 *
 * \brief Whether sub-grid positions are generated automatically.
 *
 * If this property value is set to \c true, the parent axis sub-segment count
 * is ignored when calculating sub-grid line positions. The sub-grid positions
 * are generated automatically according to the \l base property value. The
 * number of sub-grid lines is set to the base value minus one, rounded down.
 * This property is ignored when the base value is zero. Defaults to \c true.
 *
 * \sa base, QValue3DAxis::subSegmentCount
 */
void QLogValue3DAxisFormatter::setAutoSubGrid(bool enabled)
{
    Q_D(QLogValue3DAxisFormatter);
    if (d->m_autoSubGrid == enabled) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "auto subgrid value is already set to:" << enabled;
        return;
    }
    d->m_autoSubGrid = enabled;
    markDirty(false);
    emit autoSubGridChanged(enabled);

}

bool QLogValue3DAxisFormatter::autoSubGrid() const
{
    Q_D(const QLogValue3DAxisFormatter);
    return d->m_autoSubGrid;
}

/*!
 * \property QLogValue3DAxisFormatter::edgeLabelsVisible
 *
 * \brief Whether the first and last label on the axis are visible.
 *
 * When the \l base property value is non-zero, the whole axis range is often
 * not equally divided into
 * segments. The first and last segments are often smaller than the other
 * segments. In extreme cases, this can lead to overlapping labels on the first
 * and last two grid lines. By setting this property to \c false, you can
 * suppress showing the minimum and maximum labels for the axis in cases where
 * the segments do not exactly fit the axis. Defaults to \c true.
 *
 * \sa base, QAbstract3DAxis::labels
 */
void QLogValue3DAxisFormatter::setEdgeLabelsVisible(bool enabled)
{
    Q_D(QLogValue3DAxisFormatter);
    if (d->m_edgeLabelsVisible == enabled) {
        qCDebug(lcAProperties3D) << __FUNCTION__
            << "edge labels visibility is already set to:" << enabled;
        return;
    }

    d->m_edgeLabelsVisible = enabled;
    markDirty(true);
    emit edgeLabelsVisibleChanged(enabled);

}

bool QLogValue3DAxisFormatter::edgeLabelsVisible() const
{
    Q_D(const QLogValue3DAxisFormatter);
    return d->m_edgeLabelsVisible;
}

/*!
 * \internal
 */
QValue3DAxisFormatter *QLogValue3DAxisFormatter::createNewInstance() const
{
    return new QLogValue3DAxisFormatter();
}

/*!
 * \internal
 */
void QLogValue3DAxisFormatter::recalculate()
{
    Q_D(QLogValue3DAxisFormatter);
    d->recalculate();
}

/*!
 * \internal
 */
float QLogValue3DAxisFormatter::positionAt(float value) const
{
    Q_D(const QLogValue3DAxisFormatter);
    return d->positionAt(value);
}

/*!
 * \internal
 */
float QLogValue3DAxisFormatter::valueAt(float position) const
{
    Q_D(const QLogValue3DAxisFormatter);
    return d->valueAt(position);
}

/*!
 * \internal
 */
void QLogValue3DAxisFormatter::populateCopy(QValue3DAxisFormatter &copy)
{
    Q_D(QLogValue3DAxisFormatter);
    QValue3DAxisFormatter::populateCopy(copy);
    d->populateCopy(copy);
}

// QLogValue3DAxisFormatterPrivate
QLogValue3DAxisFormatterPrivate::QLogValue3DAxisFormatterPrivate()
    : m_base(10.0)
    , m_logMin(0.0)
    , m_logMax(0.0)
    , m_logRangeNormalizer(0.0)
    , m_autoSubGrid(true)
    , m_edgeLabelsVisible(true)
    , m_evenMinSegment(true)
    , m_evenMaxSegment(true)
{}

QLogValue3DAxisFormatterPrivate::~QLogValue3DAxisFormatterPrivate() {}

void QLogValue3DAxisFormatterPrivate::recalculate()
{
    Q_Q(QLogValue3DAxisFormatter);

    // When doing position/value mappings, base doesn't matter, so just use
    // natural logarithm
    m_logMin = qLn(qreal(m_min));
    m_logMax = qLn(qreal(m_max));
    m_logRangeNormalizer = m_logMax - m_logMin;

    int subGridCount = m_axis->subSegmentCount() - 1;
    int segmentCount = m_axis->segmentCount();
    QString labelFormat = m_axis->labelFormat();
    qreal segmentStep;
    if (m_base > 0.0) {
        // Update parent axis segment counts
        qreal logMin = qLn(qreal(m_min)) / qLn(m_base);
        qreal logMax = qLn(qreal(m_max)) / qLn(m_base);
        qreal logRangeNormalizer = logMax - logMin;

        qreal minDiff = qCeil(logMin) - logMin;
        qreal maxDiff = logMax - qFloor(logMax);

        m_evenMinSegment = qFuzzyIsNull(minDiff);
        m_evenMaxSegment = qFuzzyIsNull(maxDiff);

        segmentCount = qRound(logRangeNormalizer - minDiff - maxDiff);

        if (!m_evenMinSegment)
            segmentCount++;
        if (!m_evenMaxSegment)
            segmentCount++;

        segmentStep = 1.0 / logRangeNormalizer;

        if (m_autoSubGrid) {
            subGridCount = qCeil(m_base) - 2; // -2 for subgrid because subsegment count is base - 1
            if (subGridCount < 0)
                subGridCount = 0;
        }

        m_gridPositions.resize(segmentCount + 1);
        m_subGridPositions.resize(segmentCount * subGridCount);
        m_labelPositions.resize(segmentCount + 1);
        m_labelStrings.clear();
        m_labelStrings.reserve(segmentCount + 1);

        // Calculate segment positions
        int index = 0;
        if (!m_evenMinSegment) {
            m_gridPositions[0] = 0.0f;
            m_labelPositions[0] = 0.0f;
            if (m_edgeLabelsVisible)
                m_labelStrings << q->stringForValue(qreal(m_min), labelFormat);
            else
                m_labelStrings << QString(hiddenLabelTag);
            index++;
        }
        for (int i = 0; i < segmentCount; i++) {
            float gridValue = float((minDiff + qreal(i)) / qreal(logRangeNormalizer));
            m_gridPositions[index] = gridValue;
            m_labelPositions[index] = gridValue;
            if (i != 0 || m_edgeLabelsVisible) {
                m_labelStrings << q->stringForValue(qPow(m_base, minDiff + qreal(i) + logMin),
                                                    labelFormat);
            } else {
                m_labelStrings << QString(hiddenLabelTag);
            }
            index++;
        }
        // Ensure max value doesn't suffer from any rounding errors
        m_gridPositions[segmentCount] = 1.0f;
        m_labelPositions[segmentCount] = 1.0f;
        QString finalLabel = QString(hiddenLabelTag);
        if (m_edgeLabelsVisible || m_evenMaxSegment)
            finalLabel = q->stringForValue(qreal(m_max), labelFormat);

        if (m_labelStrings.size() > segmentCount)
            m_labelStrings.replace(segmentCount, finalLabel);
        else
            m_labelStrings << finalLabel;
    } else {
        // Grid lines and label positions are the same as the parent class, so call
        // parent impl first to populate those
        QValue3DAxisFormatterPrivate::doRecalculate();

        // Label string list needs to be repopulated
        segmentStep = 1.0 / qreal(segmentCount);

        m_labelStrings << q->stringForValue(qreal(m_min), labelFormat);
        for (int i = 1; i < m_labelPositions.size() - 1; i++)
            m_labelStrings[i] = q->stringForValue(qExp(segmentStep * qreal(i) * m_logRangeNormalizer
                                                       + m_logMin),
                                                  labelFormat);
        m_labelStrings << q->stringForValue(qreal(m_max), labelFormat);

        m_evenMaxSegment = true;
        m_evenMinSegment = true;
    }

    // Subgrid line positions are logarithmically spaced
    if (subGridCount > 0) {
        float oneSegmentRange = valueAt(float(segmentStep)) - m_min;
        float subSegmentStep = oneSegmentRange / float(subGridCount + 1);

        // Since the logarithm has the same curvature across whole axis range, we
        // can just calculate subgrid positions for the first segment and replicate
        // them to other segments.
        QList<float> actualSubSegmentSteps(subGridCount);

        for (int i = 0; i < subGridCount; i++) {
            float currentSubPosition = positionAt(m_min + ((i + 1) * subSegmentStep));
            actualSubSegmentSteps[i] = currentSubPosition;
        }

        float firstPartialSegmentAdjustment = float(segmentStep) - m_gridPositions.at(1);
        for (int i = 0; i < segmentCount; i++) {
            for (int j = 0; j < subGridCount; j++) {
                float position = m_gridPositions.at(i) + actualSubSegmentSteps.at(j);
                if (!m_evenMinSegment && i == 0)
                    position -= firstPartialSegmentAdjustment;
                if (position > 1.0f)
                    position = 1.0f;
                if (position < 0.0f)
                    position = 0.0f;
                m_subGridPositions[i * subGridCount + j] = position;
            }
        }
    }
}

void QLogValue3DAxisFormatterPrivate::populateCopy(QValue3DAxisFormatter &copy) const
{
    QLogValue3DAxisFormatter *logFormatter = static_cast<QLogValue3DAxisFormatter *>(&copy);
    QLogValue3DAxisFormatterPrivate *priv = logFormatter->d_func();

    priv->m_base = m_base;
    priv->m_logMin = m_logMin;
    priv->m_logMax = m_logMax;
    priv->m_logRangeNormalizer = m_logRangeNormalizer;
}

float QLogValue3DAxisFormatterPrivate::positionAt(float value) const
{
    qreal logValue = qLn(qreal(value));
    float retval = float((logValue - m_logMin) / m_logRangeNormalizer);

    return retval;
}

float QLogValue3DAxisFormatterPrivate::valueAt(float position) const
{
    qreal logValue = (qreal(position) * m_logRangeNormalizer) + m_logMin;
    return float(qExp(logValue));
}

QT_END_NAMESPACE

#include "moc_qlogvalue3daxisformatter.cpp"
