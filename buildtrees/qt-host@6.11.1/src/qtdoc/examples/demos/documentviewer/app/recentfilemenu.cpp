// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "recentfilemenu.h"
#include "recentfiles.h"

#include <QAction>

RecentFileMenu::RecentFileMenu(QWidget *parent, RecentFiles *recent) :
    QMenu(parent), m_recentFiles(recent)
{
    Q_ASSERT(recent);
    connect(m_recentFiles, &RecentFiles::changed, this, &RecentFileMenu::updateList);
    connect(m_recentFiles, &QObject::destroyed, this, &QObject::deleteLater);
    updateList();
}

void RecentFileMenu::updateList()
{
    QList<QAction *> acts = actions();
    qDeleteAll(acts);

    if (m_recentFiles->isEmpty()) {
        addAction(tr("<no recent files>"));
        return;
    }

    for (const QString &fileName : m_recentFiles->recentFiles()) {
        QAction *action = addAction(fileName);
        connect(action, &QAction::triggered, this, [&](){
            QAction *action = qobject_cast<QAction *>(sender());
            Q_ASSERT(action);
            emit fileOpened(action->text());
        });
    }
}
