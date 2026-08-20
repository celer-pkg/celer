// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RECENTFILEMENU_H
#define RECENTFILEMENU_H

#include <QMenu>

class RecentFiles;

class RecentFileMenu : public QMenu
{
    Q_OBJECT

public:
    explicit RecentFileMenu(QWidget *parent, RecentFiles *recent);

signals:
    void fileOpened(const QString &fileName);

private slots:
    void updateList();

private:
    RecentFiles *m_recentFiles;
};

#endif // RECENTFILEMENU_H
