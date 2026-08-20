// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CANDLESTICK_H
#define CANDLESTICK_H

#include <QtGraphs/qcustomseries.h>

class Candlestick : public QCustomSeries
{
    Q_OBJECT
    QML_ELEMENT

    struct CandlestickData
    {
        qreal open = 0;
        qreal close = 0;
        qreal high = 0;
        qreal low = 0;
    };

    QList<CandlestickData> m_data;

protected:
    void updateDelegate(QQuickItem *item, qsizetype index) override;

public:
    Candlestick(QObject *parent = nullptr);
};

#endif // CANDLESTICK_H
