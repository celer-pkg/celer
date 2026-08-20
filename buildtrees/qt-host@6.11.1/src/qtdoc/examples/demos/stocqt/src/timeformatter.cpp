// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "timeformatter.h"
#include <QtGraphs/qvalue3daxis.h>
#include <QtQml/qqmlextensionplugin.h>

TimeFormatter::TimeFormatter(QObject *parent)
    : QValue3DAxisFormatter(parent)
{
    qRegisterMetaType<QValue3DAxisFormatter *>();
}

TimeFormatter::~TimeFormatter() {}

QValue3DAxisFormatter *TimeFormatter::createNewInstance() const
{
    return new TimeFormatter();
}

void TimeFormatter::populateCopy(QValue3DAxisFormatter &copy)
{
    QValue3DAxisFormatter::populateCopy(copy);
    TimeFormatter *timeFormatter = static_cast<TimeFormatter *>(&copy);
    timeFormatter->m_selectionFormat = m_selectionFormat;
}

QString TimeFormatter::stringForValue(qreal value, const QString &format)
{
    Q_UNUSED(format);
    qreal epoch = value;
    switch (m_epochFormat) {
    case EpochFormat::Ms:
        epoch += m_epochOffset;
        return QDateTime::fromMSecsSinceEpoch(epoch).toString(m_selectionFormat);
        break;
    case EpochFormat::S:
        epoch += m_epochOffset / 1000;
        return QDateTime::fromSecsSinceEpoch(epoch).toString(m_selectionFormat);
        break;
    case EpochFormat::Day:
        epoch += m_epochOffset / (1000 * 86400);
        return QDateTime::fromSecsSinceEpoch(epoch * 86400).toString(m_selectionFormat);
        break;
    default:
        return "";
        break;
    }
}

QString TimeFormatter::selectionFormat() const
{
    return m_selectionFormat;
}

TimeFormatter::EpochFormat TimeFormatter::epochFormat() const
{
    return m_epochFormat;
}

long long TimeFormatter::epochOffset() const
{
    return m_epochOffset;
}

void TimeFormatter::setSelectionFormat(const QString &format)
{
    if (m_selectionFormat != format) {
        m_selectionFormat = format;
        markDirty(true);
        emit selectionFormatChanged(format);
    }
}

void TimeFormatter::setEpochOffset(const long long offset)
{
    m_epochOffset = offset;
}

void TimeFormatter::setEpochFormat(const EpochFormat format)
{
    m_epochFormat = format;
}
