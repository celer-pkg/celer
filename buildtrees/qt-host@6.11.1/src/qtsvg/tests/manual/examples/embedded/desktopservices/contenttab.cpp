// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// EXTERNAL INCLUDES
#include <QDesktopServices>
#include <QKeyEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QFileInfoList>
#include <QListWidgetItem>

// INTERNAL INCLUDES

// CLASS HEADER
#include "contenttab.h"


// CONSTRUCTORS & DESTRUCTORS
ContentTab::ContentTab(QWidget *parent) :
        QListWidget(parent)
{
    setDragEnabled(false);
    setIconSize(QSize(45, 45));
}

ContentTab::~ContentTab()
{
}

// NEW PUBLIC METHODS
void ContentTab::init(const QStandardPaths::StandardLocation &location,
                      const QString &filter,
                      const QString &icon)
{
    setContentDir(location);
    QStringList filterList;
    filterList = filter.split(";");
    m_ContentDir.setNameFilters(filterList);
    setIcon(icon);

    connect(this, &ContentTab::itemClicked,
            this, &ContentTab::openItem);

    populateListWidget();
}

// NEW PROTECTED METHODS
void ContentTab::setContentDir(const QStandardPaths::StandardLocation &location)
{
    m_ContentDir.setPath(QStandardPaths::writableLocation(location));
}

void ContentTab::setIcon(const QString &icon)
{
    m_Icon = QIcon(icon);
}

void ContentTab::populateListWidget()
{
    const QFileInfoList fileList = m_ContentDir.entryInfoList(QDir::Files, QDir::Time);
    for (const QFileInfo &item : fileList)
        new QListWidgetItem(m_Icon, itemName(item), this);
}

QString ContentTab::itemName(const QFileInfo &item)
{
    return QString(item.baseName() + "." + item.completeSuffix());
}

QUrl ContentTab::itemUrl(QListWidgetItem *item)
{
    return QUrl("file:///" + m_ContentDir.absolutePath() + "/" + item->text());
}

void ContentTab::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Select:
        openItem(currentItem());
        Q_FALLTHROUGH();
    default:
        QListWidget::keyPressEvent(event);
        break;
    }
}

void ContentTab::handleErrorInOpen(QListWidgetItem *item)
{
    Q_UNUSED(item);
    QMessageBox::warning(this, tr("Operation Failed"), tr("Unknown error!"), QMessageBox::Close);
}

// NEW SLOTS
void ContentTab::openItem(QListWidgetItem *item)
{
    bool ret = QDesktopServices::openUrl(itemUrl(item));
    if (!ret)
        handleErrorInOpen(item);
}


// End of File
