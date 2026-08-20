// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FILENAMEPROVIDER_H
#define FILENAMEPROVIDER_H

#include <QObject>
#include <QFileInfo>
#include <qqmlregistration.h>

// Helper class to retrieve the filename from the given path using QFileInfo
class FileNameProvider: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit FileNameProvider(QObject* parent = nullptr): QObject(parent) {}
    Q_INVOKABLE static QString getFileName(const QString &p) { return QFileInfo(p).fileName(); }
};

#endif // FILENAMEPROVIDER_H
