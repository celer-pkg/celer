// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
class MyWidget : public QCanvasPainterWidget
{
public:
    void initializeResources(QCanvasPainter *p) override
    {
        // load assets
        if (m_image.isNull())
            m_image = p->addImage(QImage("image.png"), QCanvasPainter::ImageFlag::Repeat);
    }

    void paint(QCanvasPainter *p) override
    {
        // ... draw using m_image
    }

    void graphicsResourcesInvalidated() override
    {
        // textures are lost, indicate the need for reload
        m_image = {};
    }

    QCanvasImage m_image;
};
//![0]
