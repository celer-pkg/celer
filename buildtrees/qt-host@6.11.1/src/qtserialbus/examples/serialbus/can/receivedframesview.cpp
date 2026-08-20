// Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "receivedframesview.h"

#include "common.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>

ReceivedFramesView::ReceivedFramesView(QWidget *parent)
    : QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QWidget::customContextMenuRequested,
        this, [this] (const QPoint &pos) {
        QMenu contextMenu(tr("Context menu"), this);

#ifndef QT_NO_CLIPBOARD
        QAction copyAction("Copy", this);
        connect(&copyAction, &QAction::triggered, this, &ReceivedFramesView::copyRow);

        if (selectedIndexes().count())
            contextMenu.addAction(&copyAction);
#endif

        QAction selectAllAction("Select all", this);
        connect(&selectAllAction, &QAction::triggered, this, &QAbstractItemView::selectAll);

        contextMenu.addAction(&selectAllAction);

        contextMenu.exec(mapToGlobal(pos));
    });
}

void ReceivedFramesView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    for (int i = 0, count = model->columnCount(); i < count; i++) {
        const QSize size = model->headerData(i, Qt::Horizontal, Qt::SizeHintRole).value<QSize>();
        setColumnWidth(i, size.width());
    }
    horizontalHeader()->setStretchLastSection(true);
}

void ReceivedFramesView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyRow();
    } else if (event->matches(QKeySequence::SelectAll)) {
        selectAll();
    } else {
        QTableView::keyPressEvent(event);
    }
}

void ReceivedFramesView::copyRow()
{
#ifndef QT_NO_CLIPBOARD
    QClipboard *clipboard = QApplication::clipboard();

    const QModelIndexList ilist = selectedIndexes();

    QString strRow;

    for (const QModelIndex &index : ilist) {
        strRow += index.data(ClipboardTextRole).toString() + " ";
        if (index.column() == model()->columnCount() - 1)
            strRow += '\n';
    }

    clipboard->setText(strRow);
#endif
}
