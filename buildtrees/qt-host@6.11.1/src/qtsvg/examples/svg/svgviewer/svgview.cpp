// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "svgview.h"
#include <QPainter>
#include <QPaintEvent>

SvgView::SvgView(QWidget *parent)
    : QWidget(parent)
    , m_background(Background::Chequered)
{
    connect(&m_renderer, &QSvgRenderer::repaintNeeded, this, qOverload<>(&QWidget::update));

    // Prepare background check-board pattern
    m_chequered = QPixmap(64, 64);
    m_chequered.fill(Qt::white);
    QPainter chequeredPainter(&m_chequered);
    QColor color(220, 220, 220);
    chequeredPainter.fillRect(0, 0, 32, 32, color);
    chequeredPainter.fillRect(32, 32, 32, 32, color);
}

void SvgView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const QSize svgSize = m_renderer.defaultSize() * m_scale;
    const QSize widgetSize = QSize(parentWidget()->width(), parentWidget()->height());
    setFixedSize(svgSize.expandedTo(widgetSize));
    QPainter painter(this);

    switch (m_background) {
    case Black:
        painter.fillRect(0, 0, width(), height(), QBrush(QColor(33, 33, 33, 255)));
        break;
    case White:
        painter.fillRect(0, 0, width(), height(), QBrush(Qt::white));
        break;
    case Chequered:
        painter.fillRect(0, 0, width(), height(), QBrush(m_chequered));
        break;
    }

    painter.save();

    const QRect bounds = QRect((widgetSize.width() - svgSize.width()) / 2,
                         (widgetSize.height() - svgSize.height()) / 2,
                         svgSize.width(), svgSize.height());

    painter.setClipRect(bounds);
    m_renderer.render(&painter, bounds);
    painter.restore();

    if (m_outline) {
        QBrush penBrush;
        if (m_background == Background::Black)
            penBrush = QBrush(Qt::white);
        else
            penBrush = QBrush(QColor(33, 33, 33, 255));

        QPen outline(penBrush, 2, Qt::DashLine);
        outline.setCosmetic(true);
        painter.setPen(outline);
        painter.drawRect(bounds);
    }
}

QImage SvgView::renderAsImage(QSize imageSize)
{
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    m_renderer.render(&painter, QRectF(QPointF(), QSizeF(imageSize)));

    return image;
}

void SvgView::setBackgroundType(Background type)
{
    if (m_background == type)
        return;

    m_background = type;
    update();
}

void SvgView::setOutline(bool enable)
{
    if (m_outline == enable)
        return;

    m_outline = enable;
    update();
}

void SvgView::setAnimations(bool enable)
{
    m_renderer.setAnimationEnabled(enable);
}

void SvgView::setFeature(Feature feature)
{
    QtSvg::Options options = m_renderer.options();
    if (feature == Feature::Tiny_12)
        options.setFlag(QtSvg::Tiny12FeaturesOnly, true);
    else
        options.setFlag(QtSvg::Tiny12FeaturesOnly, false);

    m_renderer.setOptions(options);

    if (m_renderer.isValid())
        m_renderer.load(m_fileName);
}

void SvgView::restartAnimations()
{
    m_renderer.setCurrentFrame(0);
}

void SvgView::setAssumeTrustedSource(bool enable)
{
    QtSvg::Options options = m_renderer.options();
    options.setFlag(QtSvg::AssumeTrustedSource, enable);

    m_renderer.setOptions(options);

    if (m_renderer.isValid())
        m_renderer.load(m_fileName);
}

void SvgView::load(QStringView fileName)
{
    m_fileName = fileName.toString();
    m_renderer.load(m_fileName);
    update();
}

void SvgView::reload()
{
    if (!m_renderer.isValid())
        return;

    m_renderer.load(m_fileName);
    update();
}

QStringView SvgView::currentFile() const
{
    return m_fileName;
}

void SvgView::setScale(qreal scale)
{
    if (m_scale == scale)
        return;

    m_scale = scale;
    update();
}

QSize SvgView::fileSize() const
{
    return m_renderer.isValid() ? m_renderer.defaultSize() : QSize();
}

QSize SvgView::sizeHint() const
{
    return m_renderer.isValid() ? m_renderer.defaultSize() * m_scale : QSize(1, 1);
}
