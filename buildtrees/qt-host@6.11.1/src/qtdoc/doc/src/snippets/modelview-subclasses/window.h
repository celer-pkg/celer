// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QString>
#include <QWidget>

#include "model.h"
#include "view.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow::MainWindow(QWidget *parent = 0);

public slots:
    void selectOpenFile();

private:
    void setupModelView();
    void openFile(const QString &fileName);

    LinearModel *model;
    LinearView *view;
};

#endif
