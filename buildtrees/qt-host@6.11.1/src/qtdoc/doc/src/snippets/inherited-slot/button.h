// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QPushButton>

//! [Button class with reimplemented slot]
class Button : public QPushButton
{
    Q_OBJECT

public:
    Button(QWidget *parent = 0);

public slots:
    void animateClick();
};
//! [Button class with reimplemented slot]

#endif
