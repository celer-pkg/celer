// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // std::unique_ptr<QRhi> rhi(QRhi::create(...));

    std::unique_ptr<QRhiTexture> tex(rhi->newTexture(QRhiTexture::RGBA8, QSize(1280, 720), 1,
                                     QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    tex->create();
    std::unique_ptr<QRhiRenderBuffer> ds(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(1280, 720)));
    ds->create();
    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ tex.get() });
    rtDesc.setDepthStencilBuffer(ds.get());
    std::unique_ptr<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    rt->create();

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    QRhiCommandBuffer *cb;
    QRhiReadbackResult readbackResult;

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();

    {
        pd->beginPaint(cb, rt.get());

        painter->beginPath();
        painter->circle(640, 360, 180);
        painter->setFillStyle(Qt::red);
        painter->fill();

        pd->endPaint();

        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->readBackTexture({ tex.get() }, &readbackResult);
        cb->resourceUpdate(u);
    }

    rhi->endOffscreenFrame();

    QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                    readbackResult.pixelSize.width(),
                    readbackResult.pixelSize.height(),
                    QImage::Format_RGBA8888);
    if (rhi->isYUpInFramebuffer())
        image.flip();

    image.save("result.png");

    return 0;
}
//![0]
