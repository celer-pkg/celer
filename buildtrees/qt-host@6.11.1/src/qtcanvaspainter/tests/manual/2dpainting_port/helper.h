// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef HELPER_H
#define HELPER_H

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QWidget>
#include <QCanvasPainter>
#include <QCanvasLinearGradient>

//! [0]
class Helper
{
public:
    Helper();

public:
    void paint(QPainter *painter, QPaintEvent *event, int elapsed);

    void canvasInit(QCanvasPainter *painter);
    void paint(QCanvasPainter *painter, const QRect &rect, int elapsed);

private:
    QBrush background;
    QBrush circleBrush;
    QFont textFont;
    QPen circlePen;
    QPen textPen;

    QCanvasLinearGradient qcCircleBrush;
    QFont qcTextFont;
};
//! [0]

#endif
