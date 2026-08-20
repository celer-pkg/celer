// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HANDLER_H
#define HANDLER_H

#include <QObject>
#include <QString>
#include <QXmlDefaultHandler>

/* Note that QObject must precede QXmlDefaultHandler in the following list. */

class Handler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    bool startDocument();
    bool startElement(const QString &, const QString &, const QString &qName,
                       const QXmlAttributes &attr);
    bool endElement(const QString &, const QString &, const QString &qName);
    bool characters(const QString &chars);

    bool fatalError(const QXmlParseException &exception);

signals:
    void newItem(QString &title, QString &link);

private:
    QString titleString;
    QString linkString;
    bool inItem;
    bool inTitle;
    bool inLink;
};

#endif

