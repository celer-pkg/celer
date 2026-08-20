// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QWidget>

class Window : public QWidget
{
public:
    Window(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);
};

