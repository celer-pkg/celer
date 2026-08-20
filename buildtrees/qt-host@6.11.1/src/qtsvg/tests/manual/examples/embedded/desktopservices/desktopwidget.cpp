// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// EXTERNAL INCLUDES
#include <QTabWidget>
#include <QVBoxLayout>
#include <QStandardPaths>

// INTERNAL INCLUDES
#include "linktab.h"
#include "contenttab.h"

// CLASS HEADER
#include "desktopwidget.h"

// CONSTRUCTORS & DESTRUCTORS
DesktopWidget::DesktopWidget(QWidget *parent) : QWidget(parent)

{
    QTabWidget *tabWidget = new QTabWidget(this);

    // Images
    ContentTab* imageTab = new ContentTab(tabWidget);
    imageTab->init(QStandardPaths::PicturesLocation,
                   "*.png;*.jpg;*.jpeg;*.bmp;*.gif",
                   ":/resources/photo.png");
    tabWidget->addTab(imageTab, tr("Images"));

    // Music
    ContentTab* musicTab = new ContentTab(tabWidget);
    musicTab->init(QStandardPaths::MusicLocation,
                   "*.wav;*.mp3;*.mp4",
                   ":/resources/music.png");
    tabWidget->addTab(musicTab, tr("Music"));

    // Links
    LinkTab* othersTab = new LinkTab(tabWidget);;
    // Given icon file will be overridden by LinkTab
    othersTab->init(QStandardPaths::PicturesLocation, "", "");
    tabWidget->addTab(othersTab, tr("Links"));

    // Layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabWidget);
    setLayout(layout);
}

DesktopWidget::~DesktopWidget()
{
}

// End of file
