// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGEHELPER_H
#define IMAGEHELPER_H

#include <QtCore/QObject>
#include <QtQml/QtQml>

class ImageHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit ImageHelper(QObject *parent = nullptr);

    Q_INVOKABLE QString getSupportedImageFormatsFilter() const;

};

#endif // IMAGEHELPER_H
