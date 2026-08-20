// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

class QAction;
class QListWidget;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void insertItem();
    void removeItem();
    void sortAscending();
    void sortDescending();
    void updateMenus(QListWidgetItem *current);

private:
    void setupListItems();

    QAction *insertAction;
    QAction *removeAction;
    QListWidget *listWidget;
};

#endif
