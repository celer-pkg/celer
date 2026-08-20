// Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RECEIVEDFRAMESVIEW_H
#define RECEIVEDFRAMESVIEW_H

#include <QTableView>

class ReceivedFramesView final: public QTableView
{
public:
    explicit ReceivedFramesView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model) final;

protected:
    void keyPressEvent(QKeyEvent *event) final;
    void copyRow();
};

#endif // RECEIVEDFRAMESVIEW_H
