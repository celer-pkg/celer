// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "textureitem.h"
#include <QPainter>


TextureItem::TextureItem(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_image(512, 512, QImage::Format_RGBA8888)
{
    m_image.fill(Qt::white);
}

void TextureItem::paint(QPainter *painter)
{
    const double scale = qMin(width() / double(m_image.width()),
                            height() / double(m_image.height()));
    double w = m_image.width() * scale;
    double h = m_image.height() * scale;

    painter->drawImage(QRectF(0, 0, w, h), m_image);
    updateImage();
}


const QQuick3DTextureData *TextureItem::textureData() const
{
    return &m_textureData;
}

void TextureItem::setPoint(float x, float y, int id, bool pressed)
{
    const double s = qMin(width(), height());
    setUv(x/s, y/s, id, pressed);
}

void TextureItem::setUv(float u, float v, int id, bool pressed)
{
    auto &s = state[id];

    bool inRange = u >= 0 && u <= 1 && v >=0 && v <= 1;
    if (!pressed || !inRange) {
        s.reset();
        return;
    }

    bool isDrawing = s.initialized;

    QPointF pt(u, v);
    QPointF prevRaw = s.prevRaw;

    QPointF uv1 = s.prevFiltered;
    QPointF uv2 = s.filter({u, v}, QDateTime::currentMSecsSinceEpoch());

    // Huge jumps are probably caused by crossing a
    // uv boundary on the model
    const float dist = (pt - prevRaw).manhattanLength();
    const bool jumped = dist > 0.9 && isDrawing;

    if (jumped) {
        QPointF adjust;

        if (u < 0.2 && prevRaw.x() > 0.8)
            adjust.setX(1);
        else if (u > 0.8 && prevRaw.x() < 0.2)
            adjust.setX(-1);
        if (v < 0.2 && prevRaw.y() > 0.8)
            adjust.setY(1);
        else if (v > 0.8 && prevRaw.y() < 0.2)
            adjust.setY(-1);

        // End point of this line is moved outside. Since the filtered point is lagging behind, use the raw point for this.
        uv2 = pt + adjust;

        // And then we need to set the start point of the next line to be outside the opposite side.
        // To avoid discontinuities, we use the same start point as this line.
        s.prevFiltered = uv1 - adjust;

        // Ideally, we should have drawn a second line here, but for simplicity, we'll wait until the next point arrives.
    }

    if (isDrawing) {
        QPainter p;
        p.begin(&m_image);
        p.scale(m_image.width(), m_image.height());

        QPen pen(s.color, m_penWidth);
        pen.setCosmetic(true);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);

        p.drawLine(uv1, uv2);
        p.end();
        updateImage();
        update();
    }
}

void TextureItem::clear(const QColor &color)
{
    m_image.fill(color);
    updateImage();
    update();
}

void TextureItem::setColor(const QColor &newColor, int id)
{
    auto &s = state[id];
    s.color = newColor;
}

void TextureItem::setPenWidth(qreal newPenWidth)
{
    m_penWidth = newPenWidth;
}

void TextureItem::resetPoint(int id)
{
    auto &s = state[id];
    reset(s);
}

void TextureItem::updateImage()
{
    m_textureData.setSize(m_image.size());
    m_textureData.setTextureData(QByteArray::fromRawData(reinterpret_cast<const char*>(m_image.constBits()), m_image.sizeInBytes()));
}

void TextureItem::reset(TouchPointState &s)
{
    s.reset();
}


