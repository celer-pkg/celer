// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef EMBEDDED_SVG_VIEWER_H
#define EMBEDDED_SVG_VIEWER_H

#include <QWidget>
#include <QString>
#include <QSize>

class QSvgRenderer;
class QMouseEvent;
class QSlider;
class QPushButton;

class EmbeddedSvgViewer : public QWidget
{
    Q_OBJECT
public:
    EmbeddedSvgViewer(const QString& filePath);
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public Q_SLOTS:
    void setZoom(int); // 100 <= newZoom < 0

private:
    QSvgRenderer* m_renderer;
    QSlider* m_zoomSlider;
    QPushButton* m_quitButton;
    QSize m_imageSize;
    qreal m_zoomLevel;
    qreal m_imageScale; // How many Image coords 1 widget pixel is worth

    QRectF m_viewBox;
    QRectF m_viewBoxBounds;
    QSizeF m_viewBoxSize;
    QPointF m_viewBoxCenter;
    QPointF m_viewBoxCenterOnMousePress;
    QPoint m_mousePress;

    void updateImageScale();
    QRectF getViewBox(QPointF viewBoxCenter);
};



#endif
