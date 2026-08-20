// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERITEM_H
#define QCANVASPAINTERITEM_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtQuick/qquickrhiitem.h>
#include <QtGui/qcolor.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasPainterItemRenderer;
class QCanvasPainterItemPrivate;

class Q_CANVASPAINTER_EXPORT QCanvasPainterItem : public QQuickRhiItem
{
    Q_OBJECT
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged FINAL)
    Q_PROPERTY(QVariantMap debug READ debug NOTIFY debugChanged FINAL)

public:

    QCanvasPainterItem(QQuickItem *parent = nullptr);
    ~QCanvasPainterItem() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    QVariantMap debug();

protected:
    explicit QCanvasPainterItem(QCanvasPainterItemPrivate &dd, QQuickItem *parent = nullptr);
    virtual QCanvasPainterItemRenderer* createItemRenderer() const = 0;

    QQuickRhiItemRenderer *createRenderer() final;

Q_SIGNALS:
    void fillColorChanged();
    void debugChanged();

private:
    friend class QCanvasPainterItemRenderer;
    Q_DECLARE_PRIVATE(QCanvasPainterItem)
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERITEM_H
