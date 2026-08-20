// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PAINTERWINDOW_H
#define PAINTERWINDOW_H

#include <QWindow>
#include <QColor>
#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#endif
#include <rhi/qrhi.h>

QT_FORWARD_DECLARE_CLASS(QCanvasPainter)
QT_FORWARD_DECLARE_CLASS(QCanvasPainterFactory)

class PainterWindow : public QWindow
{
    Q_OBJECT
public:
    PainterWindow(QRhi::Implementation api);
    ~PainterWindow() override;

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    protected:
    virtual void paint(QCanvasPainter *painter);
    virtual void cleanup();

    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;

private:
    void updateSurfaceType();
    void releaseSwapChain();

    void init();
    void resizeSwapChain();
    void render();

    QColor m_fillColor = {0, 0, 0, 0};

    QCanvasPainterFactory *m_factory = nullptr;
    QRhi::Implementation m_graphicsApi = QRhi::Null;

#if QT_CONFIG(opengl)
    std::unique_ptr<QOffscreenSurface> m_fallbackSurface;
#endif
    std::unique_ptr<QRhi> m_rhi;
    std::unique_ptr<QRhiSwapChain> m_sc;
    std::unique_ptr<QRhiRenderBuffer> m_ds;
    std::unique_ptr<QRhiRenderPassDescriptor> m_rp;

    bool m_hasSwapChain = false;
    bool m_running = false;
    bool m_notExposed = false;
    bool m_newlyExposed = false;
};

#endif // PAINTERWINDOW_H
