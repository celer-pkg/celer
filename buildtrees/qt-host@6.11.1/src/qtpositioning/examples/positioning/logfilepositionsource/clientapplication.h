// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef CLIENTAPPLICATION_H
#define CLIENTAPPLICATION_H

#include <QtWidgets/qmainwindow.h>

QT_BEGIN_NAMESPACE
class QGeoPositionInfo;
class QTextEdit;
QT_END_NAMESPACE

class ClientApplication : public QMainWindow
{
    Q_OBJECT
public:
    ClientApplication(QWidget *parent = 0);

private slots:
    void positionUpdated(const QGeoPositionInfo &info);

private:
    QTextEdit *textEdit;
};


#endif
