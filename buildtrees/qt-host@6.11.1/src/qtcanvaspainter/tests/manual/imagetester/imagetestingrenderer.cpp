// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "imagetestingrenderer.h"
#include "imagetestingitem.h"
#include <QColor>
#include <QRandomGenerator>
#include <QTimer>
#include "qcanvaslineargradient.h"

ImageTestingRenderer::ImageTestingRenderer()
{
}

void ImageTestingRenderer::synchronizeData(QCanvasPainterItem *item)
{
    auto p = painter();
    ImageTestingItem *realItem = static_cast<ImageTestingItem*>(item);
    auto actions = realItem->m_actions;
    bool memDataUpdated = false;
    if (actions.testFlag(ImageTestingItem::Action::Generate)) {
        generateImage();
        memDataUpdated = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveAll)) {
        int clearedImages = m_images.size();
        if (clearedImages > 0) {
            for (int i = 0; i < clearedImages; i++) {
                auto image = m_images.takeFirst().image;
                p->removeImage(image);
            }
            memDataUpdated = true;
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveHidden)) {
        auto it = m_images.begin();
        while (it != m_images.end()) {
            if (!it->visible) {
                p->removeImage(it->image);
                it = m_images.erase(it);
            } else {
                it++;
            }
        }
        memDataUpdated = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::ShowHidden)) {
        for (int i = 0; i < m_images.size(); i++)
            m_images[i].visible = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveNewest)) {
        if (!m_images.isEmpty()) {
            auto image = m_images.takeLast().image;
            p->removeImage(image);
            memDataUpdated = true;
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::HideNewest)) {
        for (int i = m_images.size()-1; i >= 0; i--) {
            if (m_images.at(i).visible) {
                m_images[i].visible = false;
                break;
            }
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::CleanupResources)) {
        p->cleanupResources();
        memDataUpdated = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::GenerateCanvas)) {
        m_newCanvasPending += 1;
    }
    if (actions.testFlag(ImageTestingItem::Action::ReregisterCanvases)) {
        m_reregisterPending = true;
    }
    if (actions.testFlag(ImageTestingItem::Action::UnregisterNewestCanvas)) {
        if (!m_canvases.isEmpty()) {
            p->removeImage(m_canvases.last().image);
            m_canvases.last().image = {}; // now QCanvasImage::isNull() == true
        }
    }
    if (actions.testFlag(ImageTestingItem::Action::RemoveNewestCanvas)) {
        if (!m_canvases.isEmpty()) {
            auto canvas = m_canvases.takeLast().canvas;
            p->destroyCanvas(canvas);
        }
    }

    m_showGradient = realItem->m_showGradient;
    m_animateGradient = realItem->m_animateGradient;

    // Update data values for the item
    realItem->m_dataAmount = p->activeImageMemoryUsage();
    realItem->dataAmountChanged();
    realItem->m_imageAmount = p->activeImageCount();
    realItem->imageAmountChanged();
    if (memDataUpdated) {
        // This is needed for cache amounts to update from the item side
        QTimer::singleShot(0, realItem, [realItem]{realItem->update();});
    }
    realItem->m_actions = {};
}

void ImageTestingRenderer::prePaint(QCanvasPainter *painter)
{
    if (m_reregisterPending) {
        m_reregisterPending = false;
        for (auto &canvas : m_canvases) {
            if (canvas.image.isNull())
                canvas.image = painter->addImage(canvas.canvas);
        }
    }

    while (m_newCanvasPending > 0) {
        const int canvasWidth = 1024;
        const int canvasHeight = 1024;
        QCanvasOffscreenCanvas canvas = painter->createCanvas({ canvasWidth, canvasHeight });
        if (canvas.isNull())
            qFatal("createCanvas() failed");

        CanvasData canvasData;
        canvasData.canvas = canvas;
        canvasData.image = painter->addImage(canvas);
        m_canvases << canvasData;

        auto *rand = QRandomGenerator::global();
        beginCanvasPainting(canvas);
        painter->setFillStyle(qRgba(rand->generate() % 255, rand->generate() % 255, rand->generate() % 255, 255));
        painter->fillRect({ 0, 0, canvasWidth, canvasHeight });
        QFont font;
        font.setPixelSize(40);
        painter->setFont(font);
        painter->setFillStyle(QColorConstants::Black);
        painter->fillText(QString::asprintf("This is an offscreen canvas of pixel size %dx%d", canvasWidth, canvasHeight), 50, 500);
        painter->fillText(QString::asprintf("using ca. %d KB", canvasData.image.sizeInBytes() / 1024), 50, 600);
        endCanvasPainting();

        m_newCanvasPending -= 1;
    }
}

void ImageTestingRenderer::paint(QCanvasPainter *painter)
{
    const float w = width();
    const float h = height();
    // Render all visible images
    int imageAndCanvasCount = m_images.size() + m_canvases.size();
    const float imageW = w * 0.5;
    const float imageH = h * 0.5;
    int index = 0;
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    QFont font;
    float fontSize = imageH * 0.08;
    font.setPixelSize(fontSize);
    painter->setFont(font);

    auto calculateRect = [=](int index) {
        const float movPos = (float(index + 0.5) / imageAndCanvasCount - 0.5);
        const float movX = imageW * movPos;
        const float movY = imageH * movPos;
        QRectF pos(w * 0.5 - imageW * 0.5 + movX,
                    h * 0.5 - imageH * 0.5 + movY,
                    imageW, imageH);
        return pos;
    };

    for (auto &image : std::as_const(m_images)) {
        if (image.visible) {
            const QRectF pos = calculateRect(index);
            painter->drawImage(image.image, pos);
            painter->fillText(QStringLiteral("IMAGE INDEX: %1").arg(QString::number(index)), pos);
        }
        index++;
    }
    for (auto &canvas : std::as_const(m_canvases)) {
        const QRectF pos = calculateRect(index);
        painter->setStrokeStyle(QColorConstants::Red);
        painter->setLineWidth(4);
        painter->strokeRect(pos);
        painter->drawImage(canvas.image, pos);
        painter->fillText(QStringLiteral("CANVAS INDEX: %1").arg(QString::number(index - m_images.size())), pos);
        index++;
    }

    static int anim = 0;
    if (m_showGradient) {
        float gradW = w * 0.8;
        float gradH = h * 0.05;
        float gradX = (w - gradW) * 0.5;
        float gradY = 0;
        QRectF rect(gradX, gradY, gradW, gradH);
        QCanvasLinearGradient g(rect.x(), rect.y(), rect.x()+rect.width(), rect.y());
        g.setColorAt(0.0f, QColorConstants::Black);
        if (m_animateGradient)
            anim++;
        float gradPos = 0.5f + 0.5f * qSin(0.005 * anim);
        g.setColorAt(gradPos, QColorConstants::Red);
        g.setColorAt(1.0f, QColorConstants::Black);
        painter->setFillStyle(g);
        painter->fillRect(rect);
    }
}

void ImageTestingRenderer::generateImage()
{
    QImage image(1024, 1024, QImage::Format_RGBA8888);
    image.fill(QColorConstants::Black);
    auto *rand =  QRandomGenerator::global();
    int g = rand->generate() % 255;
    int b = rand->generate() % 255;
    int r = 0;
    for (int y = 0; y < image.height(); y++) {
        if (y % 10 == 0)
            r = rand->generate() % 255;
        for (int x = 0; x < image.width(); x++)
            image.setPixel(x, y, qRgba(r, g, b, 100));
    }
    ImageData imageData;
    imageData.image = painter()->addImage(image);
    m_images << imageData;
}
