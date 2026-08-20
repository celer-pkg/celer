// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERITEMRENDERER_H
#define QCANVASPAINTERITEMRENDERER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtQuick/qquickrhiitem.h>
#include <QtGui/qcolor.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCanvasPainter/qcanvaspainter.h>
#include <QtCanvasPainter/qcanvaspainteritem.h>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QRhiCommandBuffer;
class QCanvasPainterItemRendererPrivate;

class Q_CANVASPAINTER_EXPORT QCanvasPainterItemRenderer : public QQuickRhiItemRenderer
{
public:
    QCanvasPainterItemRenderer();
    virtual ~QCanvasPainterItemRenderer() override;

    QColor fillColor() const;
    QCanvasPainter *painter() const;
    float width() const;
    float height() const;

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

protected:
    virtual void initializeResources(QCanvasPainter *painter);
    virtual void prePaint(QCanvasPainter *painter);
    virtual void paint(QCanvasPainter *painter);
    virtual void synchronizeData(QCanvasPainterItem *item);
    virtual void initialize(QRhiCommandBuffer *cb) override;

#if QT_VERSION < QT_VERSION_CHECK(6, 12, 0)
    // Keep synchronize() working with Qt 6.11.x
    virtual void synchronize(QCanvasPainterItem *item) {
        Q_UNUSED(item);
    }
#endif

    void render(QRhiCommandBuffer *cb) override;
    void synchronize(QQuickRhiItem *item) override;

    void grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

    void beginCanvasPainting(QCanvasOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    friend class QCanvasPainterItem;
    Q_DECLARE_PRIVATE(QCanvasPainterItemRenderer)
    QCanvasPainterItemRendererPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERITEMRENDERER_H
