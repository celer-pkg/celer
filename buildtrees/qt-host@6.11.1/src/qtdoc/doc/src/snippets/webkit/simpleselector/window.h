// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QUrl>
#include <QWidget>
//! [Window class definition]
#include "ui_window.h"

class Window : public QWidget, private Ui::Window
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);
    void setUrl(const QUrl &url);

public slots:
    void on_elementLineEdit_returnPressed();
    void on_highlightButton_clicked();
};
//! [Window class definition]

#endif
