// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "helper.h"

#include <QPainter>
#include <QPaintEvent>
#include <QWidget>

//! [0]
Helper::Helper()
{
    QLinearGradient gradient(QPointF(50, -20), QPointF(80, 20));
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(1.0, QColor(0xa6, 0xce, 0x39));

    background = QBrush(QColor(64, 32, 64));
    circleBrush = QBrush(gradient);
    circlePen = QPen(Qt::black);
    circlePen.setWidth(1);
    textPen = QPen(Qt::white);
    textFont.setPixelSize(50);
}
//! [0]

//! [1]
void Helper::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{
    painter->fillRect(event->rect(), background);
    painter->translate(100, 100);
//! [1]

//! [2]
    painter->save();
    painter->setBrush(circleBrush);
    painter->setPen(circlePen);
    painter->rotate(elapsed * 0.030);

    qreal r = elapsed / 1000.0;
    int n = 30;
    for (int i = 0; i < n; ++i) {
        painter->rotate(30);
        qreal factor = (i + r) / n;
        qreal radius = 0 + 120.0 * factor;
        qreal circleRadius = 1 + factor * 20;
        painter->drawEllipse(QRectF(radius, -circleRadius,
                                    circleRadius * 2, circleRadius * 2));
    }
    painter->restore();
//! [2]

//! [3]
    painter->setPen(textPen);
    painter->setFont(textFont);
    painter->drawText(QRect(-50, -50, 100, 100), Qt::AlignCenter, QStringLiteral("Qt"));
}
//! [3]

void Helper::canvasInit(QCanvasPainter *painter)
{
    qcCircleBrush = QCanvasLinearGradient(QPointF(50, -20), QPointF(80, 20));
    qcCircleBrush.setColorAt(0.0, Qt::white);
    qcCircleBrush.setColorAt(1.0, QColor(0xa6, 0xce, 0x39));

    qcTextFont.setPixelSize(50);
}

void Helper::paint(QCanvasPainter *painter, const QRect &rect, int elapsed)
{
    painter->setFillStyle(QColor(64, 32, 64));
    painter->fillRect(rect);

    painter->translate(100, 100);

    painter->save();
    painter->rotate(qDegreesToRadians(elapsed * 0.030));
    painter->setFillStyle(qcCircleBrush);
    painter->setStrokeStyle(Qt::black);
    qreal r = elapsed / 1000.0;
    int n = 30;
    for (int i = 0; i < n; ++i) {
        painter->rotate(qDegreesToRadians(30));
        qreal factor = (i + r) / n;
        qreal radius = 0 + 120.0 * factor;
        qreal circleRadius = 1 + factor * 20;
        painter->beginPath();
        painter->ellipse(QRectF(radius, -circleRadius, circleRadius * 2, circleRadius * 2));
        painter->stroke();
        painter->fill();
    }
    painter->restore();

    painter->setFont(qcTextFont);
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    painter->setFillStyle(Qt::white);
    painter->fillText(QStringLiteral("Qt"), QRect(-50, -50, 100, 100));
}
