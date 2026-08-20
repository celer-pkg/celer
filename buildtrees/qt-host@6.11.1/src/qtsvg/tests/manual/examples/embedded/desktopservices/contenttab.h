// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CONTENTTAB_H_
#define CONTENTTAB_H_

// EXTERNAL INCLUDES
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <QListWidget>
#include <QStandardPaths>

// INTERNAL INCLUDES

// FORWARD DECLARATIONS
QT_BEGIN_NAMESPACE
class QListWidgetItem;
class QFileInfo;
QT_END_NAMESPACE

// CLASS DECLARATION

/**
* ContentTab class.
*
* This class implements general purpose tab for media files.
*/
class ContentTab : public QListWidget
{
    Q_OBJECT

public:         // Constructors & Destructors
    ContentTab(QWidget *parent);
    virtual ~ContentTab();

public:         // New Methods
    virtual void init(const QStandardPaths::StandardLocation &location,
                      const QString &filter,
                      const QString &icon);

protected:      // New Methods
    virtual void setContentDir(const QStandardPaths::StandardLocation &location);
    virtual void setIcon(const QString &icon);
    virtual void populateListWidget();
    virtual QString itemName(const QFileInfo &item);
    virtual QUrl itemUrl(QListWidgetItem *item);
    virtual void handleErrorInOpen(QListWidgetItem *item);
protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:   // New Slots
    virtual void openItem(QListWidgetItem *item);

protected:     // Owned variables
    QDir m_ContentDir;
    QIcon m_Icon;
};


#endif // CONTENTTAB_H_

// End of File
