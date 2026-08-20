// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef PARSER_H
#define PARSER_H

#include <qglobal.h>

#include <qfile.h>
#include <qstring.h>
#include <qxml.h>

class ContentHandler;

class Parser : public QXmlSimpleReader
{
public:
    Parser();
    ~Parser();

    bool parseFile(QFile *file);
    QString result() const;
    QString errorMsg() const;

private:
    ContentHandler *handler;
};

#endif
