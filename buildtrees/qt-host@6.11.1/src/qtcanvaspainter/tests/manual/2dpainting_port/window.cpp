// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "glwidget.h"
#include "widget.h"
#include "canvasglwidget.h"
#include "window.h"

#include <QGridLayout>
#include <QLabel>
#include <QTimer>

//! [0]
Window::Window()
{
    setWindowTitle(tr("2D Painting on Native and OpenGL and CanvasPainter Widgets"));

    Widget *native = new Widget(&helper, this);
    GLWidget *openGL = new GLWidget(&helper, this);
    CanvasGLWidget *canvasGL = new CanvasGLWidget(&helper, this);
    QLabel *nativeLabel = new QLabel(tr("Native"));
    nativeLabel->setAlignment(Qt::AlignHCenter);
    QLabel *openGLLabel = new QLabel(tr("OpenGL"));
    openGLLabel->setAlignment(Qt::AlignHCenter);
    QLabel *canvasGLLabel = new QLabel(tr("CanvasPainter OpenGL"));
    canvasGLLabel->setAlignment(Qt::AlignHCenter);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(native, 0, 0);
    layout->addWidget(openGL, 0, 1);
    layout->addWidget(canvasGL, 0, 2);
    layout->addWidget(nativeLabel, 1, 0);
    layout->addWidget(openGLLabel, 1, 1);
    layout->addWidget(canvasGLLabel, 1, 2);
    setLayout(layout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, native, &Widget::animate);
    connect(timer, &QTimer::timeout, openGL, &GLWidget::animate);
    connect(timer, &QTimer::timeout, canvasGL, &CanvasGLWidget::animate);
    timer->start(50);
}
//! [0]
