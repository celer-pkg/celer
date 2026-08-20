// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QtGui/QWindow>

class TestWindow : public QWindow
{
    Q_OBJECT
public:
    explicit TestWindow(QWindow *parent = nullptr);
};

#endif // TESTWINDOW_H
