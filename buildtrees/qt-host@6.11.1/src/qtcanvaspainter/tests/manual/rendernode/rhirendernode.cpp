// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rhirendernode.h"
#include <QQuickWindow>
#include <rhi/qrhi.h>

RhiItem::RhiItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void RhiItem::setCircleRadius(float r)
{
    if (m_circleRadius == r)
        return;

    m_circleRadius = r;
    emit circleRadiusChanged();
    update();
}

void RhiItem::setUseClipRect(bool use)
{
    if (m_useClipRect == use)
        return;

    m_useClipRect = use;
    emit useClipRectChanged();
    update();
}

QSGNode *RhiItem::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    RhiRenderNode *node = static_cast<RhiRenderNode *>(old);

    if (!node) {
        node = new RhiRenderNode(window());
        window()->setColor(QColor::fromRgbF(0.4f, 0.7f, 0.0f, 1.0f));
        node->m_width = width();
        node->m_height = height();
    }

    node->m_circleRadius = m_circleRadius;
    node->m_useClipRect = m_useClipRect;

    return node;
}

RhiRenderNode::RhiRenderNode(QQuickWindow *window)
    : m_window(window)
{
    QRhi *rhi = m_window->rhi();

    // One catch currently: using a shared painter works for a single rendernode,
    // but breaks down for multiple ones, because that leads to
    //   prepare, prepare, ..., render, render, ...
    // on the rendernodes, and that cannot be made working with the paintdriver atm.
    // (because it would mean beginPaint-endPaint, beginPaint-endPaint, ...,
    // renderPaint, renderPaint, ... all using the same painter+engine+renderer,
    // which breaks)

    //m_fac = QCanvasPainterFactory::sharedInstance(rhi);
    m_fac = new QCanvasPainterFactory;
    m_fac->create(rhi);

    m_pd = m_fac->paintDriver();
    m_p = m_fac->painter();
}

RhiRenderNode::~RhiRenderNode()
{
    delete m_fac;
}

void RhiRenderNode::releaseResources()
{
}

QSGRenderNode::RenderingFlags RhiRenderNode::flags() const
{
    // DepthAware is actually important (for rendering performance)
    return QSGRenderNode::DepthAwareRendering | QSGRenderNode::NoExternalRendering;
}

QSGRenderNode::StateFlags RhiRenderNode::changedStates() const
{
    // which ones matter, not sure, perhaps none
    return QSGRenderNode::StateFlag::ViewportState | QSGRenderNode::StateFlag::CullState;
}

void RhiRenderNode::prepare()
{
    m_pd->resetForNewFrame();

    QRhiCommandBuffer *cb = commandBuffer();
    QRhiRenderTarget *rt = renderTarget();

    const QMatrix4x4 mvp = *projectionMatrix() * *matrix();
    m_pd->beginPaint(cb, rt, mvp, QCanvasRhiPaintDriver::BeginPaintFlag::DepthTest);

    const QSize itemSize(m_width, m_height);

    QCanvasPainter *painter = m_p;

    painter->resetClipping();

    const QSGClipNode *clip = clipList();
    // only the first, ignore matrix, etc.
    while (clip) {
        painter->setClipRect(clip->clipRect());
        break;
        //clip = clip->clipList();
    }

    painter->save();
    painter->beginPath();
    const QPointF center(itemSize.width() / 2, itemSize.height() / 2);
    painter->circle(center, m_circleRadius); // we are not restricted to the item bounds
    //painter->setFillStyle(QColor::fromRgbF(1.0, 0.0, 0.0, 0.5));
    painter->setFillStyle(Qt::red);
    painter->fill();
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(24);
    painter->setFont(font1);
    painter->setFillStyle(Qt::green);
    painter->fillText("QCanvasPainter", center.x(), center.y() - 30);
    painter->fillText("all inline", center.x(), center.y() + 0);
    painter->fillText("(render node)", center.x(), center.y() + 30);
    painter->restore();

    if (m_useClipRect) {
        QRectF viewArea(20, 20, 160, 160);
        painter->setClipRect(viewArea);
        painter->beginPath();
        painter->circle(40, 40, 110);
        painter->setFillStyle(Qt::green);
        painter->fill();
        painter->setFillStyle(Qt::black);
        painter->fillText("Clip me...", 40, 100);
        painter->setLineWidth(2);
        painter->strokeRect(viewArea);
    }

    m_pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
}

void RhiRenderNode::render(const RenderState *)
{
    m_pd->renderPaint();
}
