// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

#include "parser.h"

static QTextStream qerr(stderr, QIODevice::WriteOnly);

static void usage()
{
    qerr << "Usage: parse [-report-whitespace-only-chardata] [-report-start-end-entity] <in-file> [<out-file>]\n";
    exit(1);
}

int main(int argc, const char *argv[])
{
    QString file_name;
    QString out_file_name;
    bool report_start_end_entity = false;
    bool report_whitespace_only_chardata = false;

    for (int i = 1 ; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("-report-whitespace-only-chardata"))
            report_whitespace_only_chardata = true;
        else if (arg == QLatin1String("-report-start-end-entity"))
            report_start_end_entity = true;
        else if (file_name.isEmpty())
            file_name = arg;
        else if (out_file_name.isEmpty())
            out_file_name = arg;
        else
            usage();
    }

    if (file_name.isEmpty())
        usage();

    QFile in_file(file_name);
    if (!in_file.open(QIODevice::ReadOnly)) {
        qerr << "Could not open " << file_name << ": " << strerror(errno) << Qt::endl;
        return 1;
    }

    if (out_file_name.isEmpty())
        out_file_name = file_name + ".ref";

    QFile out_file;
    if (out_file_name == "-") {
        out_file.open(stdout, QFile::WriteOnly);
    } else {
        out_file.setFileName(out_file_name);
        if (!out_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qerr << "Could not open " << out_file_name << ": " << strerror(errno) << Qt::endl;
            return 1;
        }
    }

    Parser parser;
    if (report_start_end_entity)
        parser.setFeature("http://trolltech.com/xml/features/report-start-end-entity", true);
    if (report_whitespace_only_chardata)
        parser.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", true);

    parser.parseFile(&in_file);

    out_file.write(parser.result().toUtf8());

    return 0;
}
