// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RSSLISTING_H
#define RSSLISTING_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include "handler.h"

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

class RSSListing : public QWidget
{
    Q_OBJECT
public:
    RSSListing(QWidget *widget = 0);

public slots:
    void addItem(QString &title, QString &link);
    void fetch();
    void finished(QNetworkReply *reply);
    void readData();

private:
    Handler *handler;
    QXmlInputSource xmlInput;
    QXmlSimpleReader xmlReader;

    bool newInformation;

    QNetworkAccessManager http;
    int connectionId;

    QLineEdit *lineEdit;
    QTreeWidget *treeWidget;
    QTreeWidgetItem *lastItemCreated;
    QPushButton *abortButton;
    QPushButton *fetchButton;
};

#endif

