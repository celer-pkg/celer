// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include <QByteArray>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QSvgWidget;
QT_END_NAMESPACE
class MimeData;

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    SourceWidget(QWidget *parent = 0);
//![0]
public slots:
    void createData(const QString &mimetype);
    void startDrag();

private:
    QByteArray imageData;
    QSvgWidget *imageLabel;
    MimeData *mimeData;
//![0]
};

#endif
