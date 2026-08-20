// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QModelIndex>

class QAction;
class QListView;
class StringListModel;

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
    void updateMenus(const QModelIndex &current);

private:
    QAction *insertAction;
    QAction *removeAction;
    QListView *listView;
    StringListModel *model;
};

#endif
