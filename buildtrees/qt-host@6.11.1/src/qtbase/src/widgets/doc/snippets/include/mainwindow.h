// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

class QWidget;
class QAction;

class QTableWidget;
class QListWidget;
class QTreeWidget;
class QDockWidget;

class QTableView;
class QListView;

class QTextBrowser;

class QListWidgetItem;
class QTreeWidgetItem;

class QAbstractItemModel;
class QItemSelectionModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    MainWindow(QWidget *parent = nullptr);

    void setupContents();
    void setupDockWindow();
    void setupMenus();
    void setupTableItems();
    void setupTreeItems();

    void updateText(QListWidgetItem *item);
    void updateMenus(QTreeWidgetItem *current);
    void updateSortItems();
    void updateSelection(const QItemSelection &selected,
                         const QItemSelection &deselected);

    void createMenus();
    void createToolBars();
    void createDockWidgets();

    void sortAscending();
    void sortDescending();

    void insertItem();
    void removeItem();

    void changeWidth();
    void changeHeight();
    void changeCurrent(const QModelIndex &current,
                       const QModelIndex &previous);

    void findItems();
    void averageItems();
    void sumItems();

private:
    void setupListItems();

    void fillSelection();
    void clearSelection();
    void selectAll();

    QTextBrowser *textBrowser;
    QListWidget *headingList;
    QDockWidget *contentsWindow;

    QAction *insertAction;
    QAction *removeAction;
    QAction *ascendingAction;
    QAction *descendingAction;
    QAction *autoSortAction;
    QAction *findItemsAction;

    //! [0]
    QTableWidget *tableWidget;
    //! [0]

    QTreeWidget *treeWidget;
    QListWidget *listWidget;

    QAbstractItemModel *model;
    QItemSelectionModel *selectionModel;

    QTableView *table;
    QListView *listView;

};

QT_END_NAMESPACE

#endif
