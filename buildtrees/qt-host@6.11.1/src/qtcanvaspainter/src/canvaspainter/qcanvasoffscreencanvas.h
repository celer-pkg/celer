// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASOFFSCREENCANVAS_H
#define QCANVASOFFSCREENCANVAS_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobjectdefs.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasOffscreenCanvasPrivate;
class QRhiTexture;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QCanvasOffscreenCanvasPrivate)

class QCanvasOffscreenCanvas
{
    Q_GADGET_EXPORT(Q_CANVASPAINTER_EXPORT)
public:
    enum class Flag {
        PreserveContents = 0x01,
        MipMaps = 0x02
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    Q_CANVASPAINTER_EXPORT QCanvasOffscreenCanvas();
    Q_CANVASPAINTER_EXPORT QCanvasOffscreenCanvas(const QCanvasOffscreenCanvas &canvas);
    Q_CANVASPAINTER_EXPORT ~QCanvasOffscreenCanvas();
    Q_CANVASPAINTER_EXPORT QCanvasOffscreenCanvas &operator=(const QCanvasOffscreenCanvas &canvas);
    QCanvasOffscreenCanvas(QCanvasOffscreenCanvas &&other) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasOffscreenCanvas)
    void swap(QCanvasOffscreenCanvas &other) noexcept { d.swap(other.d); }

    Q_CANVASPAINTER_EXPORT void detach();

    Q_CANVASPAINTER_EXPORT bool isNull() const;

    Q_CANVASPAINTER_EXPORT Flags flags() const;

    Q_CANVASPAINTER_EXPORT QColor fillColor() const;
    Q_CANVASPAINTER_EXPORT void setFillColor(const QColor &color);

    Q_CANVASPAINTER_EXPORT QRhiTexture *texture() const;

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasOffscreenCanvas &lhs, const QCanvasOffscreenCanvas &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasOffscreenCanvas)

    QExplicitlySharedDataPointer<QCanvasOffscreenCanvasPrivate> d;
    friend class QCanvasOffscreenCanvasPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasOffscreenCanvas::Flags)
Q_DECLARE_SHARED(QCanvasOffscreenCanvas)

QT_END_NAMESPACE

#endif // QCANVASOFFSCREENCANVAS_H
