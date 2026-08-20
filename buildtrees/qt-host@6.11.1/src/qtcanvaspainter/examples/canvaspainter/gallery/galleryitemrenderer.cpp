// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "galleryitemrenderer.h"
#include "galleryitem.h"
#include "qcanvasradialgradient.h"
#include "qcanvasboxgradient.h"
#include "qcanvasconicalgradient.h"
#include "qcanvaslineargradient.h"
#include "qcanvasimagepattern.h"
#include "qcanvasboxshadow.h"
#include "qcanvasgridpattern.h"
#include "qcanvaspath.h"
#include <math.h>
#include <QFontDatabase>
#include <QImage>

GalleryItemRenderer::GalleryItemRenderer()
{
    QFontDatabase::addApplicationFont(":/fonts/Pacifico.ttf");

    m_radGrad.setStops({{ 0.0f, QColor(255, 0, 0, 255) },
                        { 0.1f, QColor(255, 255, 0, 255) },
                        { 0.5f, QColor(0, 255, 0, 255) },
                        { 1.0f, QColor(0, 0, 0, 0) }
                       });
}

GalleryItemRenderer::~GalleryItemRenderer()
{
}

// TODO: Currently all resources are added to all views.
// Consider instead adding only when m_viewIndex matches.
//![initialize]
void GalleryItemRenderer::initializeResources(QCanvasPainter *painter)
{
    QCanvasPainter::ImageFlags flags = QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps;
    m_patternImage = painter->addImage(QImage(":/images/pattern1.png"), flags);
    m_patternImage2 = painter->addImage(QImage(":/images/pattern2.png"), flags);
    m_patternImage3 = painter->addImage(QImage(":/images/pattern3.png"), flags);
    m_testImage = painter->addImage(QImage(":/images/qt_development_white.png"));
    image3Gray = painter->addImage(QImage(":/images/face-smile-bw.png"));
    image3Plain = painter->addImage(QImage(":/images/pattern2.png"));
    image3Nearest = painter->addImage(QImage(":/images/pattern2.png"),
                                        QCanvasPainter::ImageFlag::Nearest);
    image3Mips = painter->addImage(QImage(":/images/pattern2.png"),
                                     QCanvasPainter::ImageFlag::GenerateMipmaps);
    image3NearestMips = painter->addImage(QImage(":/images/pattern2.png"),
                                            QCanvasPainter::ImageFlag::Nearest | QCanvasPainter::ImageFlag::GenerateMipmaps);
    m_customBrush.setFragmentShader(":/qcgalleryexample/brush1.frag.qsb");
    m_customBrush2.setFragmentShader(":/qcgalleryexample/brush2.frag.qsb");
    m_customBrush3.setFragmentShader(":/qcgalleryexample/brush3.frag.qsb");
    m_customBrush3.setVertexShader(":/qcgalleryexample/brush3.vert.qsb");
    m_customBrush4.setFragmentShader(":/qcgalleryexample/brush4.frag.qsb");
    // Enable iTime animations
    m_customBrush.setTimeRunning(true);
    m_customBrush2.setTimeRunning(true);
    m_customBrush3.setTimeRunning(true);
    m_customBrush4.setTimeRunning(true);
}
//![initialize]

//![synchronize]
void GalleryItemRenderer::synchronizeData(QCanvasPainterItem *item)
{
    // Setting values here synchronized
    GalleryItem *realItem = static_cast<GalleryItem*>(item);
    if (realItem) {
        m_animationTime = realItem->animationTime();
        m_animationSine = realItem->animationSine();
        m_animState = realItem->animState();
        m_viewIndex = realItem->galleryView();
        if (!qFuzzyCompare(m_previousWidth, width()) ||
            !qFuzzyCompare(m_previousHeight, height())) {
            m_sizeChanged = true;
            m_previousWidth = width();
            m_previousHeight = height();
        } else {
            m_sizeChanged = false;
        }
    }
}
//![synchronize]

//![paint]
void GalleryItemRenderer::paint(QCanvasPainter *painter)
{
    Q_UNUSED(painter)

    // Views not currently centered to have reduced
    // alpha and saturation.
    m_viewAlpha = 0.2 + 0.8 * m_animState;
    m_viewSaturate = m_animState;
    painter->setGlobalAlpha(m_viewAlpha);
    painter->setGlobalSaturate(m_viewSaturate);
    m_topMargin = height() * 0.02f;
    switch (m_viewIndex) {
    case 0:
        drawRectsWithLinearGradient();
        drawRectsWithRadialGradient();
        drawRectsWithBoxGradient();
        drawRectsWithConicalGradients();
        drawRectsWithImagePattern();
        drawRectsWithBrushStroke();
        break;
    case 1:
        drawImages();
        break;
    case 2:
        drawGridPatterns();
        break;
//![paint]
    case 3:
        drawRectangularShadows();
        break;
    case 4:
        drawCustomBrushes();
        break;
    case 5:
        drawPaths();
        break;
    case 6:
        drawPainterPaths();
        break;
    case 7:
        drawTransforms();
        break;
    case 8:
        drawAntialiasing();
        break;
    case 9:
        drawCompositeModes();
        break;
    case 10:
        drawColorEffects();
        break;
    case 11:
        drawTextsFonts();
        break;
    case 12:
        drawTextsBrushes();
        break;
    case 13:
        drawTextsAlignments();
        break;
    case 14:
        drawTextsWrapping();
        break;
    default:
        break;
    }
}

void GalleryItemRenderer::drawRectsWithLinearGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = m_topMargin;

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCanvasLinearGradient(0, rect1.y(), 0, rect1.y()+rect1.height()));
    painter()->fillRect(rect1);
    posX += w + margin;

    QCanvasLinearGradient g1(0, 0, 0, 0);
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setStartPosition(posX + (w*0.4f * m_animationSine), 0);
    g1.setEndPosition(posX + w - (w*0.4f * m_animationSine), 0);
    painter()->setFillStyle(g1);
    painter()->fillRect(posX,posY,w,w);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCanvasLinearGradient g2(rect2.x(), rect2.y(), rect2.x()+rect2.width(), rect2.y()+rect2.height());
    g2.setStartColor(QColor(m_animationSine*255, 255, 255));
    g2.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g2);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCanvasLinearGradient g3(rect3.x(), rect3.y(), rect3.x(), rect3.y()+rect3.height());
    g3.setColorAt(0.0f, QColorConstants::Black);
    g3.setColorAt(0.2f, QColorConstants::Red);
    g3.setColorAt(0.4f, QColorConstants::Green);
    g3.setColorAt(0.6f, QColorConstants::Blue);
    g3.setColorAt(0.8f, QColorConstants::White);
    g3.setColorAt(1.0f, QColorConstants::Transparent);
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
}

void GalleryItemRenderer::drawRectsWithRadialGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = m_topMargin + (w+margin);

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCanvasRadialGradient(rect1.x(), rect1.y(), rect1.width()));
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCanvasRadialGradient g1;
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setCenterPosition(rect2.x() + rect2.width()/2, rect2.y() + rect2.height()/2);
    g1.setOuterRadius(0.5*w);
    g1.setInnerRadius(0.4*w*m_animationSine);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCanvasRadialGradient g3(rect3.x() + rect3.width()/2, rect3.y() + rect3.height()/2, w/2, w/4);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    m_radGrad.setCenterPosition(rect4.x() + rect4.width()/2, rect4.y() + rect4.height()/2);
    m_radGrad.setOuterRadius(w/2);
    m_radGrad.setInnerRadius(w/16);
    auto stops = m_radGrad.stops();
    stops[2].position = 0.2 + 0.6 * m_animationSine;
    stops[0].color = QColor(255, m_animationSine*255, 0, 255-m_animationSine*255);
    m_radGrad.setStops(stops);
    painter()->setFillStyle(m_radGrad);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithBoxGradient() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = m_topMargin + 2*(w+margin);

    QRectF rect1(posX,posY,w,w);
    painter()->setFillStyle(QCanvasBoxGradient(rect1, w/2));
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCanvasBoxGradient g1;
    g1.setStartColor(QColor(0,255,0,255));
    g1.setEndColor(QColor(255,0,0,255));
    g1.setRect(rect2);
    g1.setRadius(w/4);
    g1.setFeather(w/2 - m_animationSine*w/3);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCanvasBoxGradient g3(rect3, w/4, w/4);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    QCanvasBoxGradient g4(rect4, w/4, w/3);
    g4.setColorAt(0.0f, QColor(255, 255, 255, 255));
    g4.setColorAt(0.2f, QColor(0, 0, 0, 255));
    g4.setColorAt(0.4f, QColor(255, m_animationSine*255, 0, 255));
    g4.setColorAt(0.6f, QColor(0, 0, 0, 255));
    g4.setColorAt(0.8f, QColor(255, 255, 255, 255));
    g4.setColorAt(1.0f, QColor(0, 0, 0, 0));
    painter()->setFillStyle(g4);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithConicalGradients() {
    int rects = 4;
    float margin = width()*0.02f;
    float w = width() / (rects+1) - margin;
    float posX = w*0.5 + margin;
    float posY = m_topMargin + 3*(w+margin);

    QRectF rect1(posX,posY,w,w);
    QCanvasConicalGradient g1;
    g1.setCenterPosition(rect1.center());
    painter()->setFillStyle(g1);
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCanvasConicalGradient g2;
    g2.setCenterPosition(rect2.center());
    g2.setStartColor(QColor(0,255,0,255));
    g2.setEndColor(QColor(255,0,0,255));
    painter()->setFillStyle(g2);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCanvasConicalGradient g3;
    g3.setCenterPosition(rect3.center());
    g3.setAngle(-0.5 * M_PI);
    g3.setStartColor(QColor(m_animationSine*255, 255, 255));
    g3.setEndColor(QColor(255, m_animationSine*255, 255, 255-m_animationSine*255));
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
    posX += w + margin;

    QRectF rect4(posX,posY,w,w);
    QCanvasConicalGradient g4;
    g4.setCenterPosition(rect4.x() + m_animationSine * rect4.width(), rect4.y() + rect4.height()/2);
    g4.setAngle(m_animationTime);
    g4.setStartColor(QColor(255, 255, 0, 255));
    g4.setColorAt(0.25, QColor(0, 255, 0, 255));
    g4.setColorAt(0.5, QColor(0, 0, 255, 255));
    g4.setColorAt(0.75, QColor(255, 0, 255, 255));
    g4.setEndColor(QColor(255, 255, 0, 255));
    painter()->setFillStyle(g4);
    painter()->fillRect(rect4);
}

void GalleryItemRenderer::drawRectsWithImagePattern() {
    int rects = 3;
    float margin = width()*0.02f;
    float w = width() / (rects+2) - margin;
    float posX = w + margin;
    float posY = m_topMargin + 4*(w+margin);

    QRectF rect1(posX,posY,w,w);
    QCanvasImagePattern g4(m_patternImage2, rect1);
    QColor aColor = QColor::fromRgbF(1.0f,
                                     0.5f + 0.5f * sin(m_animationTime),
                                     0.5f + 0.5f * sin(m_animationTime * 2 + M_PI),
                                     1.0f);
    g4.setTintColor(aColor);
    painter()->setFillStyle(g4);
    painter()->fillRect(rect1);
    posX += w + margin;

    QRectF rect2(posX,posY,w,w);
    QCanvasImagePattern g1;
    g1.setImage(m_patternImage);
    g1.setStartPosition(rect2.x()+w/2, rect2.y()+w/2);
    g1.setImageSize(w/5, w/3);
    g1.setRotation(m_animationTime);
    QColor opacityTint(255, 255, 255, 127 + m_animationSine * 128);
    g1.setTintColor(opacityTint);
    painter()->setFillStyle(g1);
    painter()->fillRect(rect2);
    posX += w + margin;

    QRectF rect3(posX,posY,w,w);
    QCanvasImagePattern g3(m_patternImage);
    g3.setStartPosition(rect3.x(), rect3.y());
    g3.setImageSize(w*0.1 + m_animationSine * w*0.2f, w*0.15f + m_animationSine * w*0.3f);
    painter()->setFillStyle(g3);
    painter()->fillRect(rect3);
}

//![draw]
void GalleryItemRenderer::drawRectsWithBrushStroke()
{
    int rects = 3;
    float margin = width()*0.02f;
    float border = margin + margin * m_animationSine;
    float w = width() / (rects+2) - margin;
    float w2 = w - border;
    float posX = w + margin + border/2;
    float posY = m_topMargin + 5*(w+margin) + border/2;

    QRectF rect1(posX,posY,w2,w2);
    painter()->setLineWidth(border);
    painter()->setFillStyle(QCanvasImagePattern(m_patternImage3));
    QCanvasLinearGradient g1(posX, posY, posX+w2, posY+w2);
    g1.setStartColor("#ffffff");
    g1.setEndColor("#000000");
    painter()->setStrokeStyle(g1);
    painter()->beginPath();
    painter()->roundRect(rect1, border);
    painter()->fill();
    painter()->stroke();
    posX += w + margin;
//![draw]

    QRectF rect2(posX,posY,w2,w2);
    g1.setStartPosition(posX, posY);
    g1.setEndPosition(posX+w2, posY+w2);
    painter()->setFillStyle(g1);
    painter()->setStrokeStyle(QCanvasImagePattern(m_patternImage3));
    painter()->beginPath();
    painter()->roundRect(rect2, border);
    painter()->fill();
    painter()->stroke();
    posX += w + margin;

    QRectF rect3(posX,posY,w2,w2);
    QCanvasRadialGradient g2(posX+w2/4, posY+w2/4, w2);
    g2.setStartColor("#900000ff");
    g2.setEndColor("#90ff0000");
    painter()->setStrokeStyle(g2);
    QCanvasImagePattern p1 = QCanvasImagePattern(m_patternImage3);
    p1.setImageSize(16, 16);
    p1.setStartPosition(m_animationTime*40, 0);
    painter()->setFillStyle(p1);
    painter()->beginPath();
    painter()->roundRect(rect3, border);
    painter()->fill();
    painter()->stroke();
}

void GalleryItemRenderer::drawRect(float x, float y, float w, float h) {
    painter()->beginPath();
    painter()->rect(x, y, w, h);
    painter()->setLineWidth(w*0.05f);
    painter()->fill();
    painter()->stroke();
}

void GalleryItemRenderer::drawPaths() {
    int rects = 3;
    float margin = width()*0.1f;
    float w = width() / (rects) - margin;
    float lineWidth = width()*0.02f;
    float posX = margin/2;
    float posY = m_topMargin;

    painter()->setFillStyle(0xFF948979);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->setLineWidth(lineWidth);
    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Round);

    painter()->beginPath();
    painter()->rect(posX, posY, w, w);
    posX += w + margin;
    painter()->roundRect(posX, posY, w, w, w/2*m_animationSine);
    posX += w + margin;
    painter()->roundRect(posX, posY, w, w, w/2*m_animationSine, 0, w/2*(cos(m_animationTime)*0.5+0.5), w/4*(sin(m_animationTime*10)*0.5+0.5));

    posY += w+margin;
    posX = margin/2;
    painter()->ellipse(posX+w/2, posY+w/2, w/3+w/4*m_animationSine, w/3+w/4*(1-m_animationSine));
    posX += w + margin;
    painter()->moveTo(posX+w/2, posY+w/2);
    painter()->arc(posX+w/2, posY+w/2, w/2, 0, 0.5f + m_animationSine*5);
    painter()->closePath();
    posX += w + margin;
    QRectF r(posX, posY, w, w);
    painter()->ellipse(r);
    float a = w*0.1f*m_animationSine;
    float a2 = w*0.1f*(1-m_animationSine);
    r.adjust(w*0.1f+a, w*0.1f+a2, -w*0.1f-a, -w*0.1f-a2);
    painter()->beginHoleSubPath();
    painter()->ellipse(r);
    r.adjust(w*0.2f, w*0.2f, -w*0.2f, -w*0.2f);
    painter()->beginSolidSubPath();
    painter()->rect(r);

    painter()->stroke();
    painter()->fill();

    posY += w+margin;
    posX = margin/2;
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->bezierCurveTo(posX+w*0.2f*m_animationSine, posY+w*0.8f*m_animationSine, posX+w*0.8f*m_animationSine, posY+w*0.2f*m_animationSine, posX+w, posY+w);
    posX += w + margin;
    painter()->moveTo(posX, posY);
    painter()->quadraticCurveTo(posX+w*0.8f*m_animationSine, posY+w*0.2f*m_animationSine, posX+w, posY+w);
    posX += w + margin;
    painter()->moveTo(posX, posY);
    painter()->arcTo(posX, posY+w, posX+w, posY+w, w*m_animationSine);

    painter()->stroke();

    posY += w+margin;
    posX = margin/2;
    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Round);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
    posX += w + margin;
    painter()->setLineCap(QCanvasPainter::LineCap::Butt);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Miter);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
    posX += w + margin;
    painter()->setLineCap(QCanvasPainter::LineCap::Square);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Bevel);
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->lineTo(posX, posY+w);
    painter()->lineTo(posX+w*m_animationSine, posY);
    painter()->stroke();
}

void GalleryItemRenderer::drawPainterPaths()
{
    float w = width() * 0.8;
    float h = height() * 0.2;
    float posX = width() * 0.5 - w * 0.5;
    float posY = m_topMargin + h * 0.5;
    float margin = height()*0.02f;
    const int points = 100;

    enum PathGroups {
        DynamicPath = 0,
        StaticPath
    };

    if (m_sizeChanged) {
        // When the window size changes,
        // invalidate all cached paths.
        m_path1.clear();
        m_path2.clear();
        m_path3.clear();
        m_path4.clear();
        m_pathGraphLine.clear();
        m_pathGraphArea.clear();
    }

    // Path that doesn't change
    if (m_path1.isEmpty()) {
        // Reserve, so allocations are not needed while adding commands.
        // All commands are moveTo & lineTo which take 2 data
        // points (x & y) per command, so second parameter is not needed.
        m_path1.clear();
        m_path1.reserve(points);
        for (int i = 0; i < points; i++) {
            int px = posX + float(i) / (points - 1) * w;
            int py = posY + std::sin(1234.0 * (float(i*i))) * (h * 0.4);
            if (i == 0)
                m_path1.moveTo(px, py);
            else
                m_path1.lineTo(px, py);
        }
    }
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->setLineWidth(4);
    // Use same group for all non-changing paths.
    painter()->stroke(m_path1, StaticPath);

    posY += h + margin;

    // Path that increases, so old commands are remembered
    static int p2Index = 0;
    if (p2Index >= points) {
        // When we reach the end, start from the beginning
        m_path2.clear();
        p2Index = 0;
    }
    int i = p2Index;
    int px = posX + float(i) / (points - 1) * w;
    int py = posY + std::sin(1234.0 * (float(i*i))) * (h * 0.4);
    if (i == 0)
        m_path2.moveTo(px, py);
    else
        m_path2.lineTo(px, py);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->setLineWidth(4);
    painter()->stroke(m_path2, DynamicPath);
    p2Index++;

    posY += h + margin;

    // Path that doesn't change but is moved/scaled/rotated
    QPointF center(posX + 0.5 * w, posY);
    float hSize = h * 0.8;
    if (m_path3.isEmpty()) {
        m_path3.moveTo(center.x(), center.y() + hSize*0.5);
        m_path3.bezierCurveTo(center.x() - hSize, center.y() - hSize*0.3,
                            center.x() - hSize*0.1, center.y() - hSize*0.5,
                            center.x(), center.y() - hSize*0.2);
        m_path3.bezierCurveTo(center.x() + hSize*0.1, center.y() - hSize*0.5,
                            center.x() + hSize, center.y() - hSize*0.3,
                            center.x(), center.y() + hSize*0.5);
        // Add holes into the path
        m_path3.setPathWinding(QCanvasPainter::PathWinding::ClockWise);
        m_path3.circle(center.x() - hSize*0.2, center.y() - hSize*0.1, hSize * 0.1);
        m_path3.circle(center.x() + hSize*0.2, center.y() - hSize*0.1, hSize * 0.1);
    }
    QTransform transform;
    transform.translate(-w * 0.2 + w * 0.2 * sin(m_animationTime), 0);
    transform.translate(center.x(), center.y());
    transform.rotateRadians(m_animationTime);
    const float s = 0.5f + m_animationSine;
    transform.scale(s, s);
    transform.translate(-center.x(), -center.y());
    painter()->setStrokeStyle("#ff0000");
    QCanvasRadialGradient rg(center, hSize);
    rg.setStops({{0.0f, QColor(255, 50 + 50 * sin(10.0 * m_animationTime), 0)},
                 {1.0f, QColor(100, 0, 0)}});
    painter()->setFillStyle(rg);
    painter()->setLineWidth(2);
    // Using path transformation, so group can be StaticPath
    painter()->setTransform(transform);
    painter()->fill(m_path3, StaticPath);
    painter()->stroke(m_path3, StaticPath);

    // Adding paths into path
    if (m_path4.isEmpty()) {
        for (int i = 0; i < 8; i++) {
            // Create path from 8 heart paths
            QTransform t;
            t.translate(center.x(), center.y() - hSize*0.6);
            t.rotateRadians(i * M_PI_2 * 0.5);
            t.scale(0.5, 0.5);
            t.translate(-center.x(), -(center.y() - hSize*0.6));
            m_path4.addPath(m_path3, t);
        }
    }
    QTransform t2;
    t2.translate(center.x(), center.y() - hSize*0.6);
    t2.translate(w * 0.4, h * 0.4);
    t2.rotateRadians(m_animationTime);
    t2.translate(-center.x(), -(center.y() - hSize*0.6));
    painter()->setTransform(t2);
    painter()->stroke(m_path4, StaticPath);

    // Example of reusing parts of paths
    posY += h + margin;
    painter()->resetTransform();
    painter()->save();
    painter()->setRenderHint(QCanvasPainter::RenderHint::HighQualityStroking, true);
    if (m_pathGraphLine.isEmpty()) {
        // Create linegraph path.
        m_pathGraphLine.moveTo(posX, posY);
        const int lines = 200;
        for (int i = 0; i < lines; i++) {
            float fl = float(i) / (lines - 1);
            float lY = posY +
                    sin(8 * fl) * h * 0.15 +
                    sin(20 * fl) * h * 0.05;
            m_pathGraphLine.lineTo(posX + fl * w, lY);
        }
        // Create fillpath, graph + 2 points.
        m_pathGraphArea = m_pathGraphLine;
        m_pathGraphArea.lineTo(posX + w, posY + h);
        m_pathGraphArea.lineTo(posX, posY + h);
    }
    QCanvasLinearGradient g1(0, posY, 0, posY + h);
    g1.setStartColor(Qt::black);
    g1.setEndColor(Qt::transparent);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Round);
    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    painter()->setFillStyle(g1);
    painter()->fill(m_pathGraphArea);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->setLineWidth(1);
    painter()->stroke(m_pathGraphLine);
    // Selection is part of line graph.
    int commands = m_pathGraphLine.commandsSize();
    int startIndex = 0.5f * commands * std::max(sin(0.5f * m_animationTime), 0.0f);
    int count = commands - 0.5f * commands * std::max(sin(-0.5f * m_animationTime), 0.0f);
    QCanvasPath pathSelection = m_pathGraphLine.sliced(startIndex, count);
    // Selection fill is selection + 2 points
    QCanvasPath pathSelectionFill(pathSelection);
    float selectionFirstX = pathSelection.positionAt(0).x();
    float selectionLastX = pathSelection.positionAt(pathSelection.commandsSize() - 1).x();
    pathSelectionFill.lineTo(selectionLastX, posY + h);
    pathSelectionFill.lineTo(selectionFirstX, posY + h);
    painter()->setFillStyle("#20DFD0B8");
    painter()->fill(pathSelectionFill);
    painter()->setStrokeStyle("#80DFD0B8");
    painter()->setLineWidth(16);
    painter()->stroke(pathSelection);
    painter()->restore();
}

void GalleryItemRenderer::drawTransforms() {
    int rects = 3;
    float margin = width()*0.1f;
    float w = width() / (rects) - margin;
    float posX = margin/2;
    float lineWidth = width()*0.01f;
    float posY = m_topMargin;

    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setStrokeStyle(0xFF000000);
    painter()->setLineWidth(lineWidth);
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);
    // Save the painter state
    painter()->save();

    // Modify painter state
    posX += w + margin;
    painter()->setFillStyle(0xFF900000);
    painter()->setStrokeStyle(0xFFE00000);
    painter()->setLineWidth(lineWidth*2);
    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    painter()->setLineJoin(QCanvasPainter::LineJoin::Round);
    painter()->translate(posX+w/2, posY+w/2);
    painter()->rotate(1.0 - m_animationSine);
    painter()->scale(0.2f + 0.8f*m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);

    posX += w + margin;
    // And restore painter state back
    painter()->restore();
    painter()->fillRect(posX, posY, w, w);
    painter()->strokeRect(posX, posY, w, w);

    posY += w+margin;
    posX = margin/2;
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->save();
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    posX += w + margin;
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    posX += w + margin;
    painter()->translate(w/8 * m_animationSine, w/4 * m_animationSine);
    painter()->strokeRect(posX, posY, w/2, w/2);
    painter()->restore();

    posY += w+margin;
    posX = margin/2;
    painter()->save();
    painter()->translate(posX+w/2, posY+w/2);
    painter()->skew(0.4f * m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->strokeRect(posX, posY, w, w);
    painter()->restore();

    posX += w + margin;
    painter()->save();
    painter()->translate(posX+w/2, posY+w/2);
    painter()->skew(0.0f, 0.4f * m_animationSine);
    painter()->translate(-(posX+w/2), -(posY+w/2));
    painter()->strokeRect(posX, posY, w, w);
    painter()->restore();

    posX += w + margin;
    QTransform t;
    t.translate(posX+w/2, posY+w/2);
    t.rotate(m_animationTime*20);
    float scale = 0.5f + 0.5f*m_animationSine;
    t.scale(scale, scale);
    t.translate(-(posX+w/2), -(posY+w/2));
    painter()->setTransform(t);
    painter()->strokeRect(posX, posY, w, w);
    painter()->setClipRect(posX, posY, w, w);
    // Duplicate transformation
    QTransform t2 = painter()->getTransform();
    painter()->transform(t2);

    painter()->setStrokeStyle(0xFF948979);
    painter()->strokeRect(posX, posY, w, w);
    painter()->resetClipping();

    posY += w+margin;
    posX = margin/2;
    painter()->resetTransform();
    painter()->setLineWidth(lineWidth);
    QRectF r(posX, posY, w, w);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(r);
    r.adjust(lineWidth/2, lineWidth/2, -lineWidth/2, -lineWidth/2);
    painter()->setClipRect(r);
    painter()->beginPath();
    painter()->circle(posX + m_animationSine*w, posY + w/2, w/2);
    painter()->fill();
    painter()->resetClipping();

    posX += w + margin;
    painter()->setFillStyle(0xFFDFD0B8);
    painter()->fillRect(posX, posY, w, w);
    painter()->setClipRect(int(posX + w/2), int(posY + w/2), int(w/2)+2, int(w/2)+2);
    painter()->clearRect(posX, posY, w, w);
    painter()->resetClipping();

    posX += w + margin;
    painter()->setFillStyle(0xFFDFD0B8);
    painter()->setStrokeStyle(0xFF948979);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w / 4);
    painter()->fill();
    painter()->stroke();
    float cRectSize = (0.6 + 0.4 * sin(m_animationTime)) * w;
    QRectF cRect(posX + m_animationSine*w, posY + w/2 - cRectSize/2, cRectSize, cRectSize);
    painter()->translate(cRect.center());
    painter()->rotate(m_animationTime);
    painter()->translate(-cRect.center());
    painter()->clearRect(cRect);
}

void GalleryItemRenderer::drawAntialiasing() {
    int rects = 3;
    float margin = std::floor(width() * 0.1f);
    float w = std::floor(width() / (rects) - margin);
    float posX = margin/2;
    float lineWidth = width()*0.01f;
    float posY = std::floor(m_topMargin);

    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    painter()->setStrokeStyle(0xFFFFFFFF);
    painter()->setLineWidth(lineWidth);
    painter()->setAntialias(0);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();
    posX += w + margin;
    painter()->setAntialias(1.0f);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*8);
    painter()->beginPath();
    painter()->roundRect(posX, posY, w, w, w/4);
    painter()->stroke();

    posY += w+margin;
    posX = margin/2;
    int lines = 12;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
         // Align to improve thin lines painting
        painter()->moveTo(int(posX), int(posY+i*w/lines));
        painter()->lineTo(int(posX+w), int(posY+i*w/lines));
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines);
        painter()->stroke();
    }

    posY += w+margin;
    posX = margin/2;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(int(posX), int(posY+i*w/lines));
        painter()->lineTo(int(posX+w), int(posY+i*w/lines) + 5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }

    posY += w+margin;
    posX = margin/2;
    posY += m_animationSine*4;
    painter()->setAntialias(0);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(int(posX), int(posY+i*w/lines));
        painter()->lineTo(int(posX+w), int(posY+i*w/lines) + 5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }
    posX += w + margin;
    painter()->setAntialias(1.0f + m_animationSine*5);
    for (int i=0 ; i<lines ; i++) {
        painter()->beginPath();
        painter()->setLineWidth((i + 1) * 0.5f);
        painter()->moveTo(posX, posY+i*w/lines);
        painter()->lineTo(posX+w, posY+i*w/lines+5);
        painter()->stroke();
    }

}

void GalleryItemRenderer::drawCompositeItem1(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#DFD0B8");
    painter()->fill();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#948979");
    painter()->fill();
}

void GalleryItemRenderer::drawCompositeItem2(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    painter()->setLineWidth(w * 0.05f);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#DFD0B8");
    painter()->fill();
    painter()->setStrokeStyle("#ffffff");
    painter()->stroke();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#948979");
    painter()->fill();
    painter()->setStrokeStyle("#000000");
    painter()->stroke();
}

void GalleryItemRenderer::drawCompositeItem3(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode)
{
    painter()->setGlobalCompositeOperation(QCanvasPainter::CompositeOperation::SourceOver);
    painter()->setLineWidth(w * 0.05f);
    painter()->setAntialias(10.0f);
    // First item
    painter()->beginPath();
    painter()->roundRect(x, y, w, h * 0.7, w * 0.2);
    painter()->setFillStyle("#DFD0B8");
    painter()->fill();
    painter()->setStrokeStyle("#ffffff");
    painter()->stroke();

    painter()->setGlobalCompositeOperation(mode);

    // Second item
    painter()->setGlobalAlpha(0.5);
    painter()->beginPath();
    painter()->circle(x + w * 0.5f, y + h * (0.6f + 0.4f * m_animationSine), w * 0.4f);
    painter()->setFillStyle("#948979");
    painter()->fill();
    painter()->setStrokeStyle("#000000");
    painter()->stroke();
    painter()->setGlobalAlpha(1.0f);
}

void GalleryItemRenderer::drawCompositeModes() {
    int rects = 3;
    float margin = width() * 0.1f;
    float w = width() / rects - margin;
    float h = w * 1.2;
    float posX = margin/2;
    float posY = m_topMargin;

    drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem1(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);

    posY += h + margin;
    posX = margin/2;
    drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem2(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);

    posY += h + margin;
    posX = margin/2;
    drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceOver);
    posX += w + margin;
    drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::SourceAtop);
    posX += w + margin;
    drawCompositeItem3(posX, posY, w, h, QCanvasPainter::CompositeOperation::DestinationOut);
}

void GalleryItemRenderer::drawButton(float x, float y, float w, float h, const QString &label)
{
    float border = h * 0.1;
    QRectF rect1(x,y,w,h);
    painter()->setLineWidth(border);
    QPointF c = rect1.center();
    QCanvasLinearGradient g1(c.x() - 0.25 * h,
                        c.y() - 0.25 * w,
                        c.x() + 0.25 * h,
                        c.y() + 0.25 * w);
    g1.setColorAt(0.0, 0xFFFF0000);
    g1.setColorAt(0.5, 0xFFD0D000);
    g1.setColorAt(1.0, 0xFF000000);
    QCanvasLinearGradient g2(0, rect1.y(), 0, rect1.y() + rect1.height());
    g2.setColorAt(0.0, 0xFF00414A);
    g2.setColorAt(0.4, 0xFF2CDE85);
    g2.setColorAt(1.0, 0xFF00414A);
    painter()->beginPath();
    painter()->roundRect(rect1, border * 2);
    painter()->setStrokeStyle(g1);
    painter()->setFillStyle(g2);
    painter()->fill();
    painter()->stroke();

    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont f1;
    f1.setPixelSize(h * 0.4f);
    painter()->setFont(f1);
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->fillText(label, rect1);
}

void GalleryItemRenderer::drawColorEffects() {
    float w = width() * 0.8;
    float h = w * 0.2;
    float posX = width() * 0.5 - w * 0.5;
    float margin = width()*0.05f;
    float posY = m_topMargin;
    float effectAnim = 0.5 + 0.5 * sin(m_animationTime);
    painter()->setGlobalAlpha(effectAnim * m_viewAlpha);
    drawButton(posX, posY, w, h, "OPACITY");
    painter()->setGlobalAlpha(m_viewAlpha);
    posY += h + margin;
    painter()->setGlobalSaturate(3 * effectAnim * m_viewSaturate);
    drawButton(posX, posY, w, h, "SATURATE");
    painter()->setGlobalSaturate(m_viewSaturate);
    posY += h + margin;
    painter()->setGlobalBrightness(2 * effectAnim);
    drawButton(posX, posY, w, h, "BRIGHTNESS");
    painter()->setGlobalBrightness(1);
    posY += h + margin;
    painter()->setGlobalContrast(3 * effectAnim);
    drawButton(posX, posY, w, h, "CONTRAST");
    painter()->setGlobalContrast(1);
    posY += h + margin;
    painter()->setGlobalAlpha(effectAnim * m_viewAlpha);
    painter()->setGlobalSaturate((8 - 8 * effectAnim) * m_viewSaturate);
    painter()->setGlobalContrast(2 * effectAnim);
    drawButton(posX, posY, w, h, "MULTIPLE");
}

void GalleryItemRenderer::drawGridPatterns() {
    auto *p = painter();
    int rects = 2;
    float margin = width() * 0.05f;
    float w = width() / rects - margin;
    float h = height() * 0.25;
    float posX = margin/2;
    float posY = m_topMargin;

    QRectF rect1(posX, posY, w, h);
    float cellZoom = 1.0 + 0.95 * sin(0.5 * m_animationTime);
    QPointF cp1 = rect1.center();
    // Minor grid
    QCanvasGridPattern gp1(cp1.x(), cp1.y(), w * 0.1 * cellZoom, h * 0.1 * cellZoom);
    gp1.setLineColor("#404040");
    gp1.setBackgroundColor("#202020");
    p->setFillStyle(gp1);
    p->fillRect(rect1);
    // Major grid
    QCanvasGridPattern gp2(cp1.x(), cp1.y(), w * cellZoom, h * cellZoom);
    gp2.setLineColor("#d0d0d0");
    gp2.setBackgroundColor(Qt::transparent);
    p->setFillStyle(gp2);
    p->fillRect(rect1);

    posX += w + margin;
    QRectF rect2(posX, posY, w, h);
    QPointF cp2 = rect2.center();
    float g1 = w * 0.1f;
    QCanvasGridPattern gp3(cp2.x(), cp2.y(), g1, g1);
    QColor color1(m_animationSine * 150, 80, 40);
    gp3.setLineColor(color1);
    gp3.setBackgroundColor("#202020");
    gp3.setLineWidth(g1 * 0.5f);
    gp3.setFeather(g1 * 0.5f);
    gp3.setRotation(m_animationTime);
    p->setFillStyle(gp3);
    p->fillRect(rect2);

    posY += h + margin;
    posX = margin/2;
    QRectF rect3(posX, posY, w, h);
    QCanvasGridPattern gp4;
    float bar = w * 0.1f;
    gp4.setStartPosition(15 * m_animationTime, 0);
    gp4.setCellSize(bar, 0);
    gp4.setLineWidth(bar * 0.5f);
    gp4.setRotation(M_PI / 4);
    QCanvasGridPattern gp5;
    gp5.setCellSize(0, 4);
    gp5.setBackgroundColor("#202020");
    gp5.setLineColor("#404040");
    p->setStrokeStyle(gp4);
    p->setFillStyle(gp5);
    p->setLineWidth(0.5f * bar);
    p->beginPath();
    p->roundRect(rect3, bar);
    p->fill();
    p->stroke();

    posX += w + margin;
    QRectF rect4(posX, posY, w, h);
    QPointF cp4 = rect4.center();
    QCanvasGridPattern gp6;
    float strokeW = 10 * m_animationSine;
    gp6.setLineColor(Qt::transparent);
    gp6.setBackgroundColor(Qt::white);
    gp6.setStartPosition(cp4.x(), cp4.y());
    gp6.setCellSize(rect4.width() / 5, rect4.height() / 5);
    gp6.setLineWidth(strokeW);
    p->setStrokeStyle(gp6);
    p->setLineWidth(4);
    p->beginPath();
    p->roundRect(rect4, 10);
    p->stroke();
}

void GalleryItemRenderer::drawRectangularShadows() {
    auto *p = painter();

    int rects = 3;
    float margin = width() * 0.15f;
    float w = width() / rects - margin;
    float h = w;
    float posX = margin/2;
    float posY = m_topMargin + margin * 0.5;

    QRectF rect1(posX, posY, w, h);
    QCanvasBoxShadow bs1(rect1);
    p->drawBoxShadow(bs1);
    rect1.translate(w + margin, 0);
    bs1.setRect(rect1);
    bs1.setBlur(0.15 * w);
    p->drawBoxShadow(bs1);
    rect1.translate(w + margin, 0);
    bs1.setRect(rect1);
    bs1.setBlur(0.3 * w);
    p->drawBoxShadow(bs1);

    posY += h + margin * 0.5;
    QRectF rect2(posX, posY, w, h);
    QCanvasBoxShadow bs2(rect2);
    bs2.setRadius(h/2);
    p->drawBoxShadow(bs2);
    rect2.translate(w + margin, 0);
    bs2.setRect(rect2);
    bs2.setBlur(0.15 * w);
    p->drawBoxShadow(bs2);
    rect2.translate(w + margin, 0);
    bs2.setRect(rect2);
    bs2.setBlur(0.3 * w);
    p->drawBoxShadow(bs2);

    posY += h + margin * 0.5;
    QRectF rect3(posX, posY, w, h);
    QCanvasBoxShadow bs3(rect3);
    bs3.setBlur(h/8);
    bs3.setColor("#393E46");
    bs3.setTopLeftRadius(h/4 + h/4 * sin(m_animationTime));
    bs3.setTopRightRadius(h/4 + h/4 * sin(m_animationTime + 0.5 * M_PI));
    bs3.setBottomLeftRadius(h/4 + h/4 * sin(m_animationTime + 1.0 * M_PI));
    bs3.setBottomRightRadius(h/4 + h/4 * sin(m_animationTime + 1.5 * M_PI));
    p->drawBoxShadow(bs3);
    rect3.translate(w + margin, 0);
    bs3.setColor("#ffffff");
    bs3.setRect(rect3);
    p->drawBoxShadow(bs3);
    rect3.translate(w + margin, 0);
    bs3.setColor("#40ffff00");
    bs3.setRect(rect3);
    p->drawBoxShadow(bs3);

    posY += h + margin * 0.5;
    float blur = w * 0.3;
    float radius = 5 + m_animationSine * (h / 2 - 5);
    float shadowOffsetX = blur * 0.3;
    float shadowOffsetY = blur * 0.3;
    QRectF buttonRect(posX, posY, w, h);
    // Double shadows
    QRectF shadow1Rect = buttonRect.translated(-shadowOffsetX, shadowOffsetY);
    QCanvasBoxShadow shadow;
    QColor c("#222831");
    QColor cl = c.lighter(200);
    QColor cd = c.darker(250);
    cl.setAlphaF(0.3);
    cd.setAlphaF(0.8);
    shadow.setBlur(blur);
    shadow.setRadius(radius);
    shadow.setRect(shadow1Rect);
    shadow.setColor(cd);
    p->drawBoxShadow(shadow);
    QRectF shadow2Rect = buttonRect.translated(shadowOffsetX, -shadowOffsetY);
    shadow.setRect(shadow2Rect);
    shadow.setColor(cl);
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle(c);
    p->fill();

    buttonRect.translate(w + margin, 0);
    shadow1Rect.translate(w + margin, 0);
    shadow2Rect.translate(w + margin, 0);
    radius = h / 8;
    shadow.setRadius(radius);
    shadow.setBlur(h/16 + m_animationSine * h/4);
    // Double shadows
    shadow.setRect(shadow1Rect);
    shadow.setColor(cd);
    p->drawBoxShadow(shadow);
    shadow.setRect(shadow2Rect);
    shadow.setColor(cl);
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle(c);
    p->fill();

    buttonRect.translate(w + margin, 0);
    shadow1Rect.translate(w + margin, 0);
    shadow2Rect.translate(w + margin, 0);
    radius = h / 8;
    shadow.setRadius(radius);
    shadow.setBlur(h * 0.2);
    // Double shadows
    float t1 = shadowOffsetX * sin(m_animationTime);
    shadow1Rect.translate(t1, -t1);
    shadow.setRect(shadow1Rect);
    shadow.setColor(cd);
    p->drawBoxShadow(shadow);
    float t2 = shadowOffsetX * sin(m_animationTime + M_PI);
    shadow2Rect.translate(t2, -t2);
    shadow.setRect(shadow2Rect);
    shadow.setColor(cl);
    p->drawBoxShadow(shadow);
    // Button on top of shadows
    p->beginPath();
    p->roundRect(buttonRect, radius);
    p->setFillStyle(c);
    p->fill();

}

void GalleryItemRenderer::drawCustomBrushes() {
    float w = width() * 0.8;
    float h = w * 0.3;
    float posX = width() * 0.5 - w * 0.5;
    float posY = m_topMargin;
    float margin = width()*0.02f;
    float border = h * 0.1;

    QRectF rect1(posX,posY,w,h);
    // Set the color for custom brush
    m_customBrush.setData1(QVector4D(1.0, 0.0, 0.0, 1.0));
    painter()->setAntialias(1.0f);
    painter()->setLineWidth(border);
    painter()->setFillStyle(m_customBrush);
    painter()->setStrokeStyle(Qt::transparent);
    painter()->beginPath();
    painter()->roundRect(rect1, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    QRectF rect2(posX,posY,w,h);
    // Adjust the color in custom data
    m_customBrush.setData1(QVector4D(1.0, 1.0, 0.0, 1.0));
    painter()->setStrokeStyle(m_customBrush);
    painter()->setFillStyle(Qt::transparent);
    painter()->beginPath();
    painter()->roundRect(rect2, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    QRectF rect3(posX,posY,w,h);
    // Adjust the color in custom data
    m_customBrush.setData1(QVector4D(1.0, 1.0, 1.0, 1.0));
    m_customBrush2.setData1(QVector4D(1.0, 1.0, 1.0, 1.0));
    painter()->setStrokeStyle(m_customBrush2);
    painter()->setFillStyle(m_customBrush);
    painter()->setLineWidth(border*0.3);
    painter()->beginPath();
    painter()->roundRect(rect3, border * 2);
    painter()->fill();
    painter()->stroke();

    posY += h + border + margin;
    posX = width() * 0.5 - w * 0.5;
    painter()->beginPath();
    painter()->moveTo(posX, posY);
    painter()->bezierCurveTo(posX+w*0.2f*m_animationSine, posY+h*0.8f*m_animationSine, posX+w*0.8f*m_animationSine, posY-h*0.2f*m_animationSine, posX+w, posY+h);
    painter()->setLineCap(QCanvasPainter::LineCap::Round);
    m_customBrush2.setData1(QVector4D(m_animationSine, 0.0, 0.0, 1.0));
    painter()->setStrokeStyle(m_customBrush2);
    painter()->setLineWidth(w * 0.04f);
    painter()->stroke();
}

void GalleryItemRenderer::drawTextsFonts() {

    float margin = height() * 0.04f;
    float posX = margin/2;
    float posY = m_topMargin;
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->fillText("This is text without setting any font", posX, posY);

    posY += margin;
    QFont f;
    painter()->setFillStyle(0xFFD06060);
    f.setPixelSize(20);
    painter()->setFont(f);
    painter()->fillText("Size 20px", posX, posY);
    posY += margin;
    f.setPointSize(20);
    painter()->setFont(f);
    painter()->fillText("Size 20pt", posX, posY);

    posY += margin;
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextAntialias(1.0 + m_animationSine * 5.0);
    f.setPixelSize(14);
    painter()->setFont(f);
    painter()->fillText("Size 14px, blurring", posX, posY);
    posY += margin;
    f.setPixelSize(24);
    painter()->setFont(f);
    painter()->fillText("Size 24px, blurring", posX, posY);
    posY += 1.3 * margin;
    f.setPixelSize(34);
    painter()->setFont(f);
    painter()->fillText("Size 34px, blurring", posX, posY);
    painter()->setTextAntialias(1.0);

    posY += margin;
    painter()->setFillStyle(0xFFFFFFFF);
    f.setPixelSize(margin * 0.8);
    f.setWeight(QFont::Weight::Light);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Light", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Light Italic", posX, posY);

    f.setWeight(QFont::Weight::Normal);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Normal", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Normal Italic", posX, posY);

    f.setWeight(QFont::Weight::Bold);
    f.setItalic(false);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Bold", posX, posY);
    f.setItalic(true);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: Bold Italic", posX, posY);

    f.setItalic(false);
    f.setCapitalization(QFont::SmallCaps);
    painter()->setFont(f);
    posY += margin;
    painter()->fillText("Style: SmallCaps", posX, posY);

    posY += 2*margin;
    QFont f2("Pacifico", margin);
    painter()->setFont(f2);
    painter()->fillText("This is text with custom font", posX, posY);

    posY += 2*margin;
    QFont f5;
    f5.setPixelSize(margin*0.8f);
    f5.setLetterSpacing(QFont::AbsoluteSpacing, 6 * m_animationSine - 2);
    painter()->setFont(f5);
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->fillText("This is text with varying letter spacing", posX, posY);

    posY += 2*margin;
    f5.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    f5.setWordSpacing(20 * m_animationSine - 5);
    painter()->setFont(f5);
    painter()->fillText("This is text with varying word spacing", posX, posY);

    /* TODO: Test how font size animation behaves
    posY += 2*margin;
    f5.setSize(margin*0.5 + margin*4*m_animationSine);
    painter()->setFont(f5);
    painter()->fillText("Font SIZES", posX, posY);
    */
}

void GalleryItemRenderer::drawTextsBrushes() {
    float margin = height() * 0.04f;
    float posX = margin * 0.5;
    float posY = m_topMargin + margin;
    float fontSize1 = width() * 0.09;
    float fontSize2 = width() * 0.13;
    QFont f;
    f.setPixelSize(fontSize1);
    painter()->setFont(f);
    float w = width();
    QCanvasLinearGradient g1(0, 0, 0, 0);
    g1.setStartColor(QColor(255,255,255));
    g1.setEndColor(QColor(0,0,0));
    g1.setStartPosition(posX + (w*0.5f * m_animationSine), 0);
    g1.setEndPosition(posX + w + 1 - (w*0.5f * m_animationSine), 0);
    painter()->setFillStyle(g1);
    painter()->fillText("Text with linear gradient", posX, posY);

    posY += margin + fontSize1;
    float h = 80;
    QCanvasRadialGradient g2;
    g2.setStartColor(QColor(200, 200, 50, 255));
    g2.setColorAt(0.5, QColor(240, 100, 50, 255));
    g2.setEndColor(QColor(0,0,0,0));
    g2.setCenterPosition(posX + w/2, posY + h/2);
    g2.setOuterRadius(0.6*w*m_animationSine);
    g2.setInnerRadius(0.1*w*m_animationSine);
    painter()->setFillStyle(g2);
    painter()->fillText("Text with radial gradient", posX, posY);

    posY += margin + fontSize2;
    QCanvasConicalGradient g4;
    g4.setCenterPosition(width() * 0.5, posY - fontSize2 * 0.5);
    g4.setAngle(m_animationTime);
    g4.setStartColor(QColor(255, 255, 0, 255));
    g4.setColorAt(0.25, QColor(0, 255, 0, 255));
    g4.setColorAt(0.5, QColor(0, 0, 255, 255));
    g4.setColorAt(0.75, QColor(255, 0, 255, 255));
    g4.setEndColor(QColor(255, 255, 0, 255));
    painter()->setFillStyle(g4);
    f.setPixelSize(fontSize2);
    painter()->setFont(f);
    painter()->fillText("MULTIGRADIENT", posX, posY);

    posY += margin + fontSize2;
    QCanvasImagePattern p1 = QCanvasImagePattern(m_patternImage2);
    p1.setImageSize(64, 64);
    p1.setStartPosition(0, m_animationTime * 10);
    painter()->setFillStyle(p1);
    painter()->fillText("IMAGE PATTERN", posX, posY);

    posY += margin + fontSize2;
    QCanvasGridPattern p2;
    p2.setStartPosition(m_animationSine * 10, 0);
    p2.setLineWidth(2);
    p2.setCellSize(4, 4);
    p2.setLineColor("#00414A");
    p2.setBackgroundColor("#2CDE85");
    painter()->setFillStyle(p2);
    painter()->fillText("GRID PATTERN", posX, posY);

    posY += margin + fontSize2;
    painter()->setFillStyle(m_customBrush3);
    painter()->fillText("Custom Brush", posX, posY);

    posY += margin + fontSize2;
    f.setPixelSize(fontSize1);
    painter()->setFont(f);
    // Font color in data1
    m_customBrush4.setData1(QVector4D(0.0, 0.0, 0.0, 1.0));
    painter()->setFillStyle(m_customBrush4);
    QString glowingString("Text with some glowing");
    painter()->fillText(glowingString, posX, posY);
}

void GalleryItemRenderer::drawTextsAlignments() {
    float margin = height() * 0.15f;
    float fontSize = height() * 0.03f;
    float posX = margin/2;
    float posY = m_topMargin + margin/4;

    QFont f3;
    f3.setPixelSize(fontSize);
    painter()->setFont(f3);

    // Test horizontal alignments
    posX = width() * 0.5f;
    painter()->setFillStyle(0xFFDFD0B8);
    painter()->fillRect(posX-1, posY-fontSize, 1, fontSize*3);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    painter()->setFillStyle(0xFFFF9090);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Left);
    painter()->fillText("Left", posX, posY);
    painter()->setFillStyle(0xFF90FF90);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->fillText("Center", posX, posY+fontSize);
    painter()->setFillStyle(0xFF9090FF);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Right);
    painter()->fillText("Right", posX, posY+fontSize*2);

    // Test vertical (baseline) alignments
    posY += margin;
    posX = width() * 0.1f;
    const float spacing = width() * 0.19;
    painter()->setFillStyle(0xFFDFD0B8);
    painter()->fillRect(posX, posY-1, width()*0.8f, 1);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->setFillStyle(0xFFFF90FF);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    painter()->fillText("Top", posX, posY);
    painter()->setFillStyle(0xFFFFFF90);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Hanging);
    painter()->fillText("Hanging", posX+spacing, posY);
    painter()->setFillStyle(0xFF9090FF);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    painter()->fillText("Middle", posX + 2 * spacing, posY);
    painter()->setFillStyle(0xFF90FF90);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Alphabetic);
    painter()->fillText("Alphabetic", posX + 3 * spacing, posY);
    painter()->setFillStyle(0xFFFF9090);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    painter()->fillText("Bottom", posX + 4 * spacing, posY);

    // Test horizontal alignments with rect
    posY += margin;
    QRectF r1(width() * 0.5f, posY, 0, fontSize);
    float animX = width() * 0.4f * m_animationSine;
    r1.adjust(-animX, 0, animX, 0);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(r1);

    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Left);
    painter()->fillText("Left", r1);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->fillText("Center", r1);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Right);
    painter()->fillText("Right", r1);

    // Test vertical (baseline) alignments with rect
    posY += margin;
    posX = width() * 0.1f;
    float rectW = width() * 0.8f;
    QRectF r2(posX, posY, rectW, margin * m_animationSine);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(r2);

    painter()->setTextAlign(QCanvasPainter::TextAlign::Left);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Alphabetic);
    painter()->fillText("Left & Alphabetic", r2);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Right);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    painter()->fillText("Right & Bottom", r2);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    painter()->fillText("Center & Middle", r2);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    painter()->fillText("Center & Top", r2);

    posY += margin;
    painter()->strokeRect(posX, posY, rectW, 8.5 * fontSize);
    painter()->setTextDirection(QCanvasPainter::TextDirection::LeftToRight);
    QRectF r3(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Start);
    painter()->fillText("LeftToRight: Start-aligned", r3);
    posY += fontSize;
    QRectF r4(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::End);
    painter()->fillText("LeftToRight: End-aligned", r4);
    painter()->setTextDirection(QCanvasPainter::TextDirection::RightToLeft);
    posY += fontSize*2;
    QRectF r5(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Start);
    painter()->fillText("RightToLeft: Start-aligned", r5);
    posY += fontSize;
    QRectF r6(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::End);
    painter()->fillText("RightToLeft: End-aligned", r6);
    // Autodetect direction from the text string.
    painter()->setTextDirection(QCanvasPainter::TextDirection::Auto);
    posY += fontSize*2;
    QRectF r7(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Start);
    painter()->fillText("English", r7);
    posY += fontSize;
    QRectF r8(posX, posY, rectW, fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Start);
    painter()->fillText("خامل", r8);
    painter()->setTextDirection(QCanvasPainter::TextDirection::LeftToRight);

}

void GalleryItemRenderer::drawTextsWrapping() {
    const float anim = 0.5f + 0.5f * sin(m_animationTime);
    float margin = height() * 0.02f;
    float fontSize = height() * 0.03f;
    float posX = margin + (anim * width() * 0.2f);
    float posY = m_topMargin;
    float rectW = (width() - 2 * margin) - (anim * width() * 0.4f);
    float rectH = height() * 0.18;

    QFont f3;
    f3.setPixelSize(fontSize);
    painter()->setFont(f3);

    // Test text alignment and line wrapping
    QString s("This is a test string which is slightly longer one. Yes, we want long string for this.");
    painter()->setFillStyle(0xFFFFFFFF);
    painter()->setTextWrapMode(QCanvasPainter::WrapMode::WordWrap);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Left);
    painter()->setTextLineHeight(-10);
    QRectF rect1(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF393E46);
    painter()->strokeRect(rect1);
    painter()->fillText(s, rect1);
    QRectF boundingRect1 = painter()->textBoundingBox(s, rect1);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(boundingRect1);

    posY += rectH + margin;
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    painter()->setTextLineHeight(0);
    QRectF rect2(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF393E46);
    painter()->strokeRect(rect2);
    painter()->fillText(s, rect2);
    QRectF boundingRect2 = painter()->textBoundingBox(s, rect2);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(boundingRect2);

    posY += rectH + margin;
    painter()->setTextAlign(QCanvasPainter::TextAlign::Right);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    painter()->setTextLineHeight(10);
    QRectF rect3(posX, posY, rectW, rectH);
    painter()->setStrokeStyle(0xFF393E46);
    painter()->strokeRect(rect3);
    painter()->fillText(s, rect3);
    QRectF boundingRect3 = painter()->textBoundingBox(s, rect3);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(boundingRect3);

    posY += 2.0 * rectH + margin;
    posX = 0.5f * width();
    QFont f4;
    f4.setPixelSize(width() * 0.04f);
    painter()->setFont(f4);
    painter()->setTextLineHeight(0);
    painter()->setTextWrapMode(QCanvasPainter::WrapMode::WordWrap);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    //painter()->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    //painter()->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    //painter()->setTextAlign(QCanvasPainter::TextAlign::Right);
    //painter()->setTextAlign(QCanvasPainter::TextAlign::Left);
    float circleRadius = width() * 0.1f * (1.0 + anim);
    float textMargin = width() * 0.04f;
    float textW = 2 * (circleRadius - textMargin);
    QString text = QStringLiteral("Circle radius: %1").arg(qRound(circleRadius));
    painter()->fillText(text, posX, posY, textW);
    painter()->beginPath();
    painter()->setLineWidth(1);
    painter()->circle(posX, posY, circleRadius);
    painter()->setStrokeStyle(0xFF393E46);
    painter()->stroke();
    QRectF boundingRect4 = painter()->textBoundingBox(text, posX, posY, textW);
    painter()->setStrokeStyle(0xFFDFD0B8);
    painter()->strokeRect(boundingRect4);
}

void GalleryItemRenderer::drawImages() {

    float margin = height()*0.05f;
    float posX = width()*0.1f;
    float posY = m_topMargin;

    QRectF rect1(0, 0, width(), height());
    // Tint background image darker
    auto pattern = QCanvasImagePattern(m_patternImage2);
    pattern.setTintColor(QColor(10, 10, 10));
    painter()->setFillStyle(pattern);
    painter()->fillRect(rect1);

    // Draw image scaled to view width
    float imageRatio = float(m_testImage.height()) / m_testImage.width();
    float imgX = margin;
    float imgW = width() - 2 * margin;
    QRectF imgRect(imgX, posY, imgW, imgW * imageRatio);
    painter()->drawImage(m_testImage, imgRect);

    // Draw just the "Development" part of the image
    posY += imgRect.height() + margin;
    float cropW = 105;
    QRectF sourceArea(cropW, 0, m_testImage.width() - cropW, m_testImage.height());
    float cx = width() * 0.5f;
    float r = width() * 0.1f + (m_animationSine * width() * 0.2f);
    QRectF targetArea(cx-r, posY, r*2, width()*0.1);
    painter()->drawImage(m_testImage, sourceArea, targetArea);

    // Draw rotating & scaling & tinted image
    posY += targetArea.height() + height() * 0.15;
    QRectF rect(cx-r * 0.5f, posY - r * 0.5f, r, r);
    QPointF c(rect.x()+rect.width() * 0.5f, rect.y()+rect.height() * 0.5f);
    painter()->save();
    painter()->translate(c);
    painter()->rotate(m_animationTime);
    painter()->translate(-c);
    QColor tintColor(255, 255, 255 - m_animationSine * 200);
    image3Gray.setTintColor(tintColor);
    painter()->drawImage(image3Gray,rect);
    painter()->restore();

    // Scaled images with and without mipmapping
    posY = height() * 0.55f;
    float posY2 = posY + (height() * 0.2f);
    bool useNearest = (m_animationSine > 0.5f);
    float sizeDiff = width() * 0.025f;
    float centerY = 4 * sizeDiff;
    for (int i = 0; i < 7; ++i) {
        float size = (i + 1) * sizeDiff;
        QRectF rect2(posX, posY+centerY, size, size);
        painter()->drawImage(useNearest ? image3Nearest : image3Plain, rect2);
        QRectF rect3(posX, posY2+centerY, size, size);
        painter()->drawImage(useNearest ? image3NearestMips : image3Mips, rect3);
        posX += size + 2;
    }
    QFont font;
    float fontSize = width() * 0.04f;
    font.setPixelSize(fontSize);
    painter()->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter()->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    QString scaling = useNearest ? "NEAREST" : "LINEAR";
    QString offString = QString("MIPMAPS: OFF, TEXTURE: %1").arg(scaling);
    QString onString = QString("MIPMAPS: ON, TEXTURE: %1").arg(scaling);
    float text1PosY = posY + 0.5f * centerY;
    float text2PosY = posY2 + 0.5f * centerY;
    painter()->setFillStyle(QColorConstants::White);
    painter()->setFont(font);
    painter()->fillText(offString, cx, text1PosY);
    painter()->fillText(onString, cx, text2PosY);
}
