// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASRHIPAINTPAINTDRIVER_H
#define QCANVASRHIPAINTPAINTDRIVER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qsize.h>
#include <QtGui/qmatrix4x4.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>
#include <functional>

QT_BEGIN_NAMESPACE

class QRhi;
class QRhiCommandBuffer;
class QRhiRenderTarget;
class QCanvasRhiPaintDriverPrivate;

class QCanvasRhiPaintDriver
{
public:
    enum class BeginPaintFlag {
        DepthTest = 0x01
    };
    Q_DECLARE_FLAGS(BeginPaintFlags, BeginPaintFlag)
    enum class EndPaintFlag {
        DoNotRecordRenderPass = 0x01
    };
    Q_DECLARE_FLAGS(EndPaintFlags, EndPaintFlag)

    Q_CANVASPAINTER_EXPORT QCanvasRhiPaintDriver();
    Q_CANVASPAINTER_EXPORT ~QCanvasRhiPaintDriver();

    Q_CANVASPAINTER_EXPORT void resetForNewFrame();
    Q_CANVASPAINTER_EXPORT void beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt,
                                           const QColor &fillColor = Qt::black, QSize logicalSize = QSize(), float dpr = 1.0f,
                                           BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void beginPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QMatrix4x4 &matrix, BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void beginPaint(QCanvasOffscreenCanvas &canvas, QRhiCommandBuffer *cb, BeginPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void endPaint(EndPaintFlags flags = {});
    Q_CANVASPAINTER_EXPORT void renderPaint();
    Q_CANVASPAINTER_EXPORT void grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

private:
    Q_DISABLE_COPY(QCanvasRhiPaintDriver)
    QCanvasRhiPaintDriverPrivate *d = nullptr;
    friend class QCanvasRhiPaintDriverPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasRhiPaintDriver::BeginPaintFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasRhiPaintDriver::EndPaintFlags)

QT_END_NAMESPACE

#endif // QCANVASRHIPAINTPAINTDRIVER_H
