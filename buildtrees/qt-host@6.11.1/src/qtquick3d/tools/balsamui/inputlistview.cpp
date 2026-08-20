// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "inputlistview.h"

#include <QDropEvent>
#include <QMimeData>

InputListView::InputListView(QWidget *parent) : QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
}

bool InputListView::tryAddItem(const QString &label)
{
    if (containsItem(label))
        return false;

    addItem(label);
    return true;
}

void InputListView::dropEvent(QDropEvent *event)
{
    constexpr int MAX_URLS = 1024;
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && i < MAX_URLS; ++i) {
            const QUrl &url = urlList.at(i);
            const auto filename = url.toLocalFile();
            if (url.isLocalFile() && !containsItem(filename))
                addItem(filename);
        }
    }
}

void InputListView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void InputListView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void InputListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

bool InputListView::containsItem(const QString &needle)
{
    for (int i = 0; i < count(); ++i) {
        if (item(i)->text() == needle)
            return true;
    }
    return false;
}
