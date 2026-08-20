// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTWEBVIEW_TESTS_AUTO_TESTUTIL_H
#define QTWEBVIEW_TESTS_AUTO_TESTUTIL_H

#include <QtCore/qfile.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

QUrl makeTestFileUrl(const QString &testFile)
{
    const QString tempTestFile = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + testFile;
    const bool exists = QFile::exists(tempTestFile);
    if (exists)
        return QUrl::fromLocalFile(tempTestFile);

    QFile tf(QString(":/") + testFile);
    const bool copied = tf.copy(tempTestFile);

    return QUrl::fromLocalFile(copied ? tempTestFile : testFile);
}

#endif // QTWEBVIEW_TESTS_AUTO_TESTUTIL_H
