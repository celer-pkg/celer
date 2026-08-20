// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QDockWidget;
class QListWidget;
class QListWidgetItem;
class QTextBrowser;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

private slots:
    void updateText(QListWidgetItem *item);

private:
    void setupContents();
    void setupDockWindow();
    void setupMenus();

    QDockWidget *contentsWindow;
    QListWidget *headingList;
    QTextBrowser *textBrowser;
};

#endif
