// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ITEMTEST_H
#define ITEMTEST_H

#include "qcanvaspainteritem.h"
#include "qcanvaspainteritemrenderer.h"
#include "../shared/painthelper.h"

class HelloItemRenderer : public QCanvasPainterItemRenderer
{

public:
    HelloItemRenderer()
    {
        // why this is interesting: because moving an item to a new QQuickWindow
        // automatically destroys and creates a new renderer object, thus
        // ensuring that no graphicsResourcesInvalidated() and similar are
        // needed, unlike the widget that has no dedicated renderer object.
        qDebug() << "HelloItemRenderer" << this << "created";
    }

    void initializeResources(QCanvasPainter *p) override
    {
        qDebug() << "initializeResources" << this;

        // Provide our own QCanvasImage, to verify that a "load-if-not-yet-done"
        // logic works as expected, and it does not break down when the item
        // is moved between windows (and so changes QRhis, losing all graphics
        // resources in the process).
        static QImage logoImage(":/quitlogo.png");
        if (logo.isNull())
            logo = p->addImage(logoImage, QCanvasPainter::ImageFlag::Repeat);
    }

    inline void synchronizeData(QCanvasPainterItem *painterItem) override;

    void paint(QCanvasPainter *p) override
    {
        p->setAntialias(m_antialiasAmount);
        paintHelloItem(p, width(), height(), &logo);
    }

    QCanvasImage logo;
    float m_antialiasAmount;
};

class HelloItem : public QCanvasPainterItem
{
    Q_OBJECT
    Q_PROPERTY(float aa READ aa WRITE setAa NOTIFY aaChanged)

public:
    HelloItem(QQuickItem *parent = nullptr)
        :  QCanvasPainterItem(parent)
    {
    }

    float aa() const { return m_aa; }
    void setAa(float a)
    {
        if (qFuzzyCompare(a, m_aa))
            return;
        m_aa = a;
        emit aaChanged();
        update();
    }

    QCanvasPainterItemRenderer *createItemRenderer() const override
    {
        return new HelloItemRenderer;
    }

signals:
    void aaChanged();

public:
    float m_aa = 1.0f;
};

inline void HelloItemRenderer::synchronizeData(QCanvasPainterItem *painterItem)
{
    //qDebug() << "synchronizeData";

    auto *item = static_cast<HelloItem *>(painterItem);
    m_antialiasAmount = item->m_aa;
}

#endif // ITEMTEST_H
