// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include "ui_window.h"

//! [Window class definition]
class Window : public QWidget, private Ui::Window
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

public slots:
    void saveSvg();
    void updateBackground(int background);
    void updateColor();
    void updateShape(int shape);

private:
    QString path;
};
//! [Window class definition]

#endif
