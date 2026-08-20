// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "candlestick.h"
#include <QQuickItem>
#include <QRandomGenerator>

void Candlestick::updateDelegate(QQuickItem *item, qsizetype index)
{
    auto &data = m_data[index];

    auto &&items = item->childItems();

    auto wick = items[0];
    wick->setHeight(qAbs(mapY(data.high) - mapY(data.low)));
    wick->setX(mapX(index + 1) - wick->width() / 2 + 1);
    wick->setY(mapY((data.low + data.high) / 2) - wick->height() / 2);

    auto candle = items[1];
    candle->setHeight(qAbs(mapY(data.open) - mapY(data.close)));
    candle->setProperty("color", data.open < data.close ? "lightgreen" : "salmon");
    candle->setX(mapX(index + 1) - candle->width() / 2 + 1);
    candle->setY(mapY((data.close + data.open) / 2) - candle->height() / 2);
}

Candlestick::Candlestick(QObject *parent)
    : QCustomSeries(parent)
{
    for (int i = 0; i < 9; i++) {
        CandlestickData d;
        d.open = QRandomGenerator::global()->generateDouble() * 80.0f + 10.0f;
        d.close = QRandomGenerator::global()->generateDouble() * 80.0f + 10.0f;
        d.high = qMax(d.open, d.close) + QRandomGenerator::global()->generateDouble() * 10.0f;
        d.low = qMin(d.open, d.close) - QRandomGenerator::global()->generateDouble() * 10.0f;

        m_data.append(d);
        append();
    }
}
