// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VIEW_H
#define VIEW_H

#include <QAbstractItemView>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QRect>
#include <QSize>
#include <QWidget>

class LinearView : public QAbstractItemView
{
    Q_OBJECT
public:
    LinearView(QWidget *parent = 0);

    QRect itemViewportRect(const QModelIndex &index) const;
    void ensureVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;

protected slots:
    /*void dataChanged(const QModelIndex &topLeft, const QModelIndex
    &bottomRight);*/
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    /*void selectionChanged(const QItemSelection &deselected, const QItemSelection &selected);
    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);*/

protected:
    void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;
    QRect itemRect(const QModelIndex &item) const;
    bool isIndexHidden(const QModelIndex &index) const;
    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    QSize sizeHint() const;

private:
    int rows(const QModelIndex &index = QModelIndex()) const;
    void updateGeometries();
};

#endif
