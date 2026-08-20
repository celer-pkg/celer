// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// EXTERNAL INCLUDES
#include <QUrl>
#include <QMessageBox>
#include <QListWidgetItem>

// INTERNAL INCLUDES

// CLASS HEADER
#include "linktab.h"

LinkTab::LinkTab(QWidget *parent) :
        ContentTab(parent)
{
}

LinkTab::~LinkTab()
{
}

void LinkTab::populateListWidget()
{
    m_WebItem = new QListWidgetItem(QIcon(":/resources/browser.png"), tr("Launch Browser"), this);
    m_MailToItem = new QListWidgetItem(QIcon(":/resources/message.png"), tr("New e-mail"), this);
}

QUrl LinkTab::itemUrl(QListWidgetItem *item)
{
    if (m_WebItem == item) {
        return QUrl(tr("http://qt.io"));
    } else if (m_MailToItem == item) {
        return QUrl(tr("mailto:noreply@qt.io?subject=Qt feedback&body=Hello"));
    } else {
        // We should never endup here
        Q_ASSERT(false);
        return QUrl();
    }
}
void LinkTab::handleErrorInOpen(QListWidgetItem *item)
{
    if (m_MailToItem == item) {
        QMessageBox::warning(this, tr("Operation Failed"), tr("Please check that you have\ne-mail account defined."), QMessageBox::Close);
    } else {
        ContentTab::handleErrorInOpen(item);
    }
}

// End of file
