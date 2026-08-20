// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>

class QPaintEvent;

class PaintWidget : public QWidget
{
    Q_OBJECT

public:
    PaintWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif
