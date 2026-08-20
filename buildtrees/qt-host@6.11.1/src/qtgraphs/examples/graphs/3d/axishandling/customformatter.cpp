// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "customformatter.h"
#include <QtGraphs/qvalue3daxis.h>
#include <QtQml/qqmlextensionplugin.h>

static const qreal oneDayMs = 60.0 * 60.0 * 24.0 * 1000.0;

CustomFormatter::CustomFormatter(QObject *parent)
    : QValue3DAxisFormatter(parent)
{
    qRegisterMetaType<QValue3DAxisFormatter *>();
}

CustomFormatter::~CustomFormatter() {}

//! [1]
QValue3DAxisFormatter *CustomFormatter::createNewInstance() const
{
    return new CustomFormatter();
}

void CustomFormatter::populateCopy(QValue3DAxisFormatter &copy)
{
    QValue3DAxisFormatter::populateCopy(copy);

    CustomFormatter *customFormatter = static_cast<CustomFormatter *>(&copy);
    customFormatter->m_originDate = m_originDate;
    customFormatter->m_selectionFormat = m_selectionFormat;
}
//! [1]

//! [2]
void CustomFormatter::recalculate()
{
    // We want our axis to always have gridlines at date breaks

    // Convert range into QDateTimes
    QDateTime minTime = valueToDateTime(qreal(axis()->min()));
    QDateTime maxTime = valueToDateTime(qreal(axis()->max()));

    // Find out the grid counts
    QTime midnight(0, 0);
    QDateTime minFullDate(minTime.date(), midnight);
    int gridCount = 0;
    if (minFullDate != minTime)
        minFullDate = minFullDate.addDays(1);
    QDateTime maxFullDate(maxTime.date(), midnight);

    gridCount += minFullDate.daysTo(maxFullDate) + 1;
    int subGridCount = axis()->subSegmentCount() - 1;

    QList<float> gridPositions;
    QList<float> subGridPositions;
    QList<float> labelPositions;
    QStringList labelStrings;

    // Reserve space for position arrays and label strings
    gridPositions.resize(gridCount);
    subGridPositions.resize((gridCount + 1) * subGridCount);
    labelPositions.resize(gridCount);
    labelStrings.reserve(gridCount);

    // Calculate positions and format labels
    qint64 startMs = minTime.toMSecsSinceEpoch();
    qint64 endMs = maxTime.toMSecsSinceEpoch();
    qreal dateNormalizer = endMs - startMs;
    qreal firstLineOffset = (minFullDate.toMSecsSinceEpoch() - startMs) / dateNormalizer;
    qreal segmentStep = oneDayMs / dateNormalizer;
    qreal subSegmentStep = 0;
    if (subGridCount > 0)
        subSegmentStep = segmentStep / qreal(subGridCount + 1);

    for (int i = 0; i < gridCount; i++) {
        qreal gridValue = firstLineOffset + (segmentStep * qreal(i));
        gridPositions[i] = float(gridValue);
        labelPositions[i] = float(gridValue);
        labelStrings << minFullDate.addDays(i).toString(axis()->labelFormat());
    }

    for (int i = 0; i <= gridCount; i++) {
        if (subGridPositions.size()) {
            for (int j = 0; j < subGridCount; j++) {
                float position;
                if (i)
                    position = gridPositions.at(i - 1) + subSegmentStep * (j + 1);
                else
                    position = gridPositions.at(0) - segmentStep + subSegmentStep * (j + 1);
                if (position > 1.0f || position < 0.0f)
                    position = gridPositions.at(0);
                subGridPositions[i * subGridCount + j] = position;
            }
        }
    }
    setGridPoitions(gridPositions);
    setSubGridPositions(subGridPositions);
    setlabelPositions(labelPositions);
    setLabelStrings(labelStrings);
}
//! [2]

//! [3]
QString CustomFormatter::stringForValue(qreal value, const QString &format)
{
    Q_UNUSED(format);

    return valueToDateTime(value).toString(m_selectionFormat);
}
//! [3]

QDate CustomFormatter::originDate() const
{
    return m_originDate;
}

QString CustomFormatter::selectionFormat() const
{
    return m_selectionFormat;
}

void CustomFormatter::setOriginDate(const QDate &date)
{
    if (m_originDate != date) {
        m_originDate = date;
        markDirty(true);
        emit originDateChanged(date);
    }
}

void CustomFormatter::setSelectionFormat(const QString &format)
{
    if (m_selectionFormat != format) {
        m_selectionFormat = format;
        markDirty(true); // Necessary to regenerate already visible selection label
        emit selectionFormatChanged(format);
    }
}

//! [0]
QDateTime CustomFormatter::valueToDateTime(qreal value) const
{
    return m_originDate.startOfDay().addMSecs(qint64(oneDayMs * value));
}
//! [0]
