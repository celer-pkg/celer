// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
class MyWidget : public QCanvasPainterWidget
{
public:
    QCanvasOffscreenCanvas canvas;
    QCanvasImage canvasImage;

    void graphicsResourcesInvalidated() override
    {
        canvas = {}; // so that the next prePaint() will recreate and redraw the canvas
    }

    void prePaint(QCanvasPainter *p) override
    {
        if (canvas.isNull()) {
            canvas = p->createCanvas(QSize(320, 240));
            beginCanvasPainting(canvas);
            p->beginPath();
            p->circle(160, 120, 20);
            p->setFillStyle(Qt::red);
            p->fill();
            endCanvasPainting();
            canvasImage = p->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);
        }
    }

    void paint(QCanvasPainter *p) override
    {
        // use canvasImage as a brush or with drawImage()
    }
};
//![0]
