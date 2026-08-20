// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HANDLER_H
#define HANDLER_H

#include <qstring.h>
#include <qxml.h>

class Handler : public QXmlDefaultHandler
{
public:
    bool startDocument();
    bool startElement(const QString &, const QString &, const QString &qName,
                       const QXmlAttributes &);
    bool endElement(const QString &, const QString &, const QString &);

    bool fatalError(const QXmlParseException &exception);

    QStringList& names();
    QList<int>& indentations();

private:
    int indentationLevel;
    QStringList elementName;
    QList<int> elementIndentation;
};

#endif

