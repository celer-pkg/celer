// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PAINTHELPER_H
#define PAINTHELPER_H

#include <algorithm>
#include <QImage>
#include "qcanvaspainter.h"
#include "qcanvasradialgradient.h"
#include "qcanvasimagepattern.h"

inline void paintHelloItem(QCanvasPainter *p, float width, float height, QCanvasImage *heartImage = nullptr, float patternSizeFactor = 0.02f)
{
    float size = std::min(width, height);
    QPointF center(width/2, height/2);
    // Paint the background circle
    QCanvasRadialGradient gradient1(center.x(), center.y() - size*0.1, size*0.6);
    gradient1.setStartColor("#909090");
    gradient1.setEndColor("#404040");
    p->beginPath();
    p->circle(center, size*0.46);
    p->setFillStyle(gradient1);
    p->fill();
    p->setStrokeStyle("#202020");
    p->setLineWidth(size*0.02);
    p->stroke();
    // Hello text
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(size*0.08);
    p->setFont(font1);
    p->setFillStyle("#B0D040");
    p->fillText("HELLO", center.x(), center.y() - size*0.18);
    // QCanvasPainter text
    QFont font2;
    font2.setWeight(QFont::Weight::Thin);
    font2.setPixelSize(size*0.12);
    p->setFont(font2);
    p->fillText("QCanvasPainter", center.x(), center.y() - size*0.08);
    // Paint heart
    static QImage logoImage(":/quitlogo.png");
    // The QCanvasImage must not be static, the underlying texture needs to be
    // recreated if the QRhi (and so the renderer) changes, when reparenting to
    // a new top-level for instance.
    QCanvasImage logo = p->addImage(logoImage, QCanvasPainter::ImageFlag::Repeat);
    float patternSize = size * patternSizeFactor;
    QCanvasImagePattern pattern(heartImage ? *heartImage : logo, center.x(), center.y(), patternSize, patternSize);
    p->setFillStyle(pattern);
    p->setLineCap(QCanvasPainter::LineCap::Round);
    p->setStrokeStyle("#B0D040");
    p->beginPath();
    p->moveTo(center.x(), center.y() + size*0.3);
    p->bezierCurveTo(center.x() - size*0.25, center.y() + size*0.1,
                     center.x() - size*0.05, center.y() + size*0.05,
                     center.x(), center.y() + size*0.15);
    p->bezierCurveTo(center.x() + size*0.05, center.y() + size*0.05,
                     center.x() + size*0.25, center.y() + size*0.1,
                     center.x(), center.y() + size*0.3);
    p->stroke();
    p->fill();
}

#endif // PAINTHELPER_H
