// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QDataStream>
#include <private/qcustomseriesdata_p.h>

QT_BEGIN_NAMESPACE

QCustomSeriesData::QCustomSeriesData(QObject *parent)
    : QObject(parent)
{}

QVariantMap QCustomSeriesData::data() const
{
    return m_data;
}

void QCustomSeriesData::setData(const QVariantMap &newData)
{
    m_data = newData;
}

QT_END_NAMESPACE

#include "moc_qcustomseriesdata_p.cpp"
