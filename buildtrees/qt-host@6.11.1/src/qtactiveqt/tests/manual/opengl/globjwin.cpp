// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "globjwin.h"
#include "glbox.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFrame>
#include <QMenuBar>
#include <QMenu>
#include <QApplication>


GLObjectWindow::GLObjectWindow(QWidget *parent)
    : QWidget(parent)
{
    // Create a menu
    QMenu *file = new QMenu(this);
    file->addAction(tr("Exit"), qApp, &QApplication::quit);

    // Create a menu bar
    QMenuBar *m = new QMenuBar(this);
    m->addMenu(file)->setText(tr("&File"));

    // Create a nice frame to put around the OpenGL widget
    QFrame *f = new QFrame(this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    GLBox *c = new GLBox(f, "glbox");

    // Create the three sliders; one for each rotation axis
    QSlider *x = new QSlider(Qt::Vertical, this);
    x->setMaximum(360);
    x->setPageStep(60);
    x->setTickPosition(QSlider::TicksLeft);
    connect(x, &QSlider::valueChanged, c, &GLBox::setXRotation);

    QSlider *y = new QSlider(Qt::Vertical, this);
    y->setMaximum(360);
    y->setPageStep(60);
    y->setTickPosition(QSlider::TicksLeft);
    connect(y, &QSlider::valueChanged, c, &GLBox::setYRotation);

    QSlider *z = new QSlider(Qt::Vertical, this);
    z->setMaximum(360);
    z->setPageStep(60);
    z->setTickPosition(QSlider::TicksLeft);
    connect(z, &QSlider::valueChanged, c, &GLBox::setZRotation);

    // Now that we have all the widgets, put them into a nice layout

    // Top level layout, puts the sliders to the left of the frame/GL widget
    QHBoxLayout *hlayout = new QHBoxLayout(this);

    // Put the sliders on top of each other
    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(x);
    vlayout->addWidget(y);
    vlayout->addWidget(z);

    // Put the GL widget inside the frame
    QHBoxLayout *flayout = new QHBoxLayout(f);
    flayout->setContentsMargins(0, 0, 0, 0);
    flayout->addWidget(c, 1);

    hlayout->setMenuBar(m);
    hlayout->addLayout(vlayout);
    hlayout->addWidget(f, 1);
}
