// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERWIDGET_H
#define QCANVASPAINTERWIDGET_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtWidgets/qrhiwidget.h>
#include <QtGui/qcolor.h>
#include <functional>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasPainter;
class QCanvasPainterWidgetPrivate;

class Q_CANVASPAINTER_EXPORT QCanvasPainterWidget : public QRhiWidget
{
    Q_OBJECT
public:
    explicit QCanvasPainterWidget(QWidget *parent = nullptr);
    ~QCanvasPainterWidget() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    bool hasSharedPainter() const;
    void setSharedPainter(bool enable);

    void grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

protected:
    virtual void initializeResources(QCanvasPainter *painter);
    virtual void prePaint(QCanvasPainter *painter);
    virtual void paint(QCanvasPainter *painter);
    virtual void graphicsResourcesInvalidated();

    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;

    void beginCanvasPainting(QCanvasOffscreenCanvas &canvas);
    void endCanvasPainting();

private:
    Q_DECLARE_PRIVATE(QCanvasPainterWidget)
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERWIDGET_H
