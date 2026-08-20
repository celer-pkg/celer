// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GRAPHPRINTER_H
#define GRAPHPRINTER_H

#include <QtCore/qfile.h>
#include <QtCore/qurl.h>
#include <QtGui/qimage.h>
#include <QtQml/qqmlregistration.h>

class GraphPrinter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    using QObject::QObject;
    ~GraphPrinter() override;

    Q_INVOKABLE QString generatePDF(const QUrl &path, const QImage &image);
    Q_INVOKABLE QString print(const QImage &image, const QString &printerName);
    Q_INVOKABLE QStringList getPrinters();
    Q_INVOKABLE qreal maxTextureSize();
};

#endif // GRAPHPRINTER_H
