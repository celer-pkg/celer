// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LINKTAB_H_
#define LINKTAB_H_

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include "contenttab.h"

// FORWARD DECLARATIONS
QT_BEGIN_NAMESPACE
class QWidget;
class QListWidgetItem;
QT_END_NAMESPACE

// CLASS DECLARATION

/**
* LinkTab class.
*
* This class implements tab for opening http and mailto links.
*/
class LinkTab : public ContentTab
{
    Q_OBJECT

public:     // Constructors & Destructors
    LinkTab(QWidget *parent);
    ~LinkTab();

protected:  // Derived Methods
    void populateListWidget() override;
    QUrl itemUrl(QListWidgetItem *item) override;
    void handleErrorInOpen(QListWidgetItem *item) override;

private:    // Used variables
    QListWidgetItem *m_WebItem;
    QListWidgetItem *m_MailToItem;

private:    // Owned variables

};

#endif // CONTENTTAB_H_

// End of File
