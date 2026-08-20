// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "barsetgenerator.h"
#include <QtGraphs/QBarSet>

BarSetGenerator::BarSetGenerator(QObject *parent)
    : QObject(parent)
{}

QBarSet *BarSetGenerator::createNewBarSet()
{
    auto set = new QBarSet();
    const int valueCount = 6;
    for (int i = 0; i < valueCount; ++i)
        set->append(i + 1);
    return set;
}
