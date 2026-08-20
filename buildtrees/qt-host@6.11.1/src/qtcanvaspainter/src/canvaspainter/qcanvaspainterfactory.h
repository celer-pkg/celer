// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERFACTORY_H
#define QCANVASPAINTERFACTORY_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasPainter;
class QCanvasPainterFactoryPrivate;
class QCanvasRhiPaintDriver;
class QRhi;

class QCanvasPainterFactory
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasPainterFactory();
    Q_CANVASPAINTER_EXPORT ~QCanvasPainterFactory();

    Q_CANVASPAINTER_EXPORT static QCanvasPainterFactory *sharedInstance(QRhi *rhi);

    Q_CANVASPAINTER_EXPORT bool isValid() const;
    Q_CANVASPAINTER_EXPORT QCanvasPainter *create(QRhi *rhi);
    Q_CANVASPAINTER_EXPORT void destroy();
    Q_CANVASPAINTER_EXPORT QCanvasPainter *painter();

    Q_CANVASPAINTER_EXPORT QCanvasRhiPaintDriver *paintDriver();

private:
    Q_DISABLE_COPY(QCanvasPainterFactory)
    QCanvasPainterFactoryPrivate *d = nullptr;
    friend class QCanvasPainterFactoryPrivate;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERFACTORY_H
