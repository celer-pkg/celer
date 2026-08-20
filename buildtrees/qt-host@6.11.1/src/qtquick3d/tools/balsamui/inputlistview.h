// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INPUTLISTVIEW_H
#define INPUTLISTVIEW_H

#include <QListWidget>

class InputListView : public QListWidget
{
public:
    InputListView(QWidget *parent = nullptr);
    bool tryAddItem(const QString &label);

protected:
    void dropEvent(QDropEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

private:
    bool containsItem(const QString &needle);
};

#endif
