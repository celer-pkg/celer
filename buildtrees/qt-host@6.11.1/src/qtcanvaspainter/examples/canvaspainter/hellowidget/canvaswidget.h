// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QCanvasPainterWidget>
#include <QCanvasImage>

//![0]
class CanvasWidget : public QCanvasPainterWidget
{
public:
    CanvasWidget();
    void initializeResources(QCanvasPainter *p) override;
    void paint(QCanvasPainter *p) override;
    void graphicsResourcesInvalidated() override;

private:
    QCanvasImage m_image;
};
//![0]

#endif
