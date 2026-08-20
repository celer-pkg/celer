// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RHIRENDERNODE_H
#define RHIRENDERNODE_H

#include <QQuickItem>
#include <QSGRenderNode>
#include <QCanvasPainterFactory>
#include <QCanvasRhiPaintDriver>
#include <QCanvasPainter>

class RhiItemRenderer;

class RhiItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(float circleRadius READ circleRadius WRITE setCircleRadius NOTIFY circleRadiusChanged)
    Q_PROPERTY(bool useClipRect READ useClipRect WRITE setUseClipRect NOTIFY useClipRectChanged)

public:
    RhiItem(QQuickItem *parent = nullptr);

    float circleRadius() const { return m_circleRadius; }
    void setCircleRadius(float r);

    bool useClipRect() const { return m_useClipRect; }
    void setUseClipRect(bool use);

signals:
    void circleRadiusChanged();
    void useClipRectChanged();

private:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;

    float m_circleRadius = 100;
    bool m_useClipRect = false;
};

class RhiRenderNode : public QSGRenderNode
{
public:
    RhiRenderNode(QQuickWindow *window);
    ~RhiRenderNode();

    void prepare() override;
    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;
    QSGRenderNode::StateFlags changedStates() const override;

private:
    QQuickWindow *m_window;
    QCanvasPainterFactory *m_fac;
    QCanvasRhiPaintDriver *m_pd;
    QCanvasPainter *m_p;
    float m_width;
    float m_height;
    float m_circleRadius = 100;
    bool m_useClipRect = false;
    friend class RhiItem;
};

#endif
