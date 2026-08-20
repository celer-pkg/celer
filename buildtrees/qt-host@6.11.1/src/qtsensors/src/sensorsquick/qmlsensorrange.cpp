// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmlsensorrange_p.h"

QmlSensorRange::QmlSensorRange(QObject *parent)
    : QObject(parent),
      min(0),
      max(0)
{
}

QmlSensorRange::~QmlSensorRange()
{
}

int QmlSensorRange::minimum() const
{
    return min;
}

int QmlSensorRange::maximum() const
{
    return max;
}

QmlSensorOutputRange::QmlSensorOutputRange(QObject *parent)
    : QObject(parent),
      min(0),
      max(0),
      acc(0)
{
}

QmlSensorOutputRange::~QmlSensorOutputRange()
{
}

qreal QmlSensorOutputRange::minimum() const
{
    return min;
}

qreal QmlSensorOutputRange::maximum() const
{
    return max;
}

qreal QmlSensorOutputRange::accuracy() const
{
    return acc;
}
