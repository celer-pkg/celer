// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "painterwindow.h"
#include <QtCanvasPainter/QCanvasPainter>
#include <QtCanvasPainter/QCanvasImage>
#include <QtCanvasPainter/QCanvasPath>
#include <QTimer>
#include <QElapsedTimer>

#include "ecggraph.h"
#include "theme.h"

class MainWindow : public PainterWindow
{

public:
    MainWindow(QRhi::Implementation api);
    ~MainWindow();

protected:
    void paint(QCanvasPainter *p) override;
    void touchEvent(QTouchEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;

private:
    friend class ECGGraph;

    enum class Dirty {
        TemperatureBars = 0x0001,
        GraphGrid = 0x0002,
        ActivitySlider = 0x0004,
        ViewBackgrounds = 0x0008,
        All = 0xFFFF
    };
    Q_DECLARE_FLAGS(DirtyFlags, Dirty)

    enum PathGroups {
        // Paths which basically change only when resizing window.
        StaticPaths = 1, // TODO: Why text rendering fails if this is 0?
        // Paths for graphs which don't update on every frame (e.g. temperature graph).
        GraphPaths,
        // Paths for activity slider, changes only when adjusted.
        ActivitySliderPaths
    };

    void initializeTempData();
    void initializeRespData();
    void updateViewSizes();
    void paintTempBars(float x, float y, float w, float h);
    void paintRespGraph(float x, float y, float w, float h);
    void paintSlider(float x, float y, float w, float h);
    void paintGrid(float x, float y, float w, float h);
    void buttonClicked(int buttonID);
    void animateData();
    void updateRespData();
    void updateWarnings();

    QCanvasPainter *m_painter;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    bool m_initialized = false;
    bool m_showSettings = true;
    DirtyFlags m_dirty = { Dirty::All };
    float m_px = 1.0f;
    float m_margin = 0.0f;
    float m_activity = 0.2f;
    // Current pulse
    float m_hr = 82.0f;
    // Current body temperature
    float m_temperature = 37.0f;
    float m_temperatureMax = 43.0f;
    float m_temperatureMin = 35.0f;
    // How often temperature graph is updated
    int m_temperatureLogMs = 1000;
    float m_spo2 = 99.0f;
    // AirWay Respiratory Rate
    float m_awrr = 12.5;
    QCanvasPath m_tempBarsPath;
    QList<float> m_tempData;
    int m_tempDataCount = 64;

    QList<float> m_respData;
    int m_respDataCount = 256;

    ECGGraph m_ecgGraph;

    QRectF m_sliderRect;
    float m_sliderMarginTop = 0;
    float m_sliderMarginBottom = 0;
    int m_selectedButton = 0;

    struct View {
        QRectF rect;
        QString title;
        bool fillBackground = true;
    };

    enum Views {
        W1,
        W2,
        W3,
        W4,
        Center,
        Slider,
        B1,
        B2,
        B3,
        ViewsEnd,
    };
    View m_views[ViewsEnd];
    // These views will be shows with warning box
    QSet<int> m_warningViews;

    Theme m_theme;
    float m_iconSize = 20;
    QCanvasImage m_b1ImageLight;
    QCanvasImage m_b1ImageDark;
    QCanvasImage m_b2ImageLight;
    QCanvasImage m_b2ImageDark;
    QCanvasImage m_b3ImageLight;
    QCanvasImage m_b3ImageDark;
    QCanvasImage m_sImageLight;
    QCanvasImage m_sImageDark;

    QFont m_titleFont;
    QFont m_bigFont;
};

#endif // MAINWINDOW_H
