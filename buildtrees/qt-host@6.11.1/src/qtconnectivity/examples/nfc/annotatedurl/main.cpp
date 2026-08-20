// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "annotatedurl.h"
#include "mainwindow.h"

#include <QNearFieldManager>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>

#include <QApplication>

//! [0]
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow mainWindow;
    AnnotatedUrl annotatedUrl;

    QObject::connect(&annotatedUrl, &AnnotatedUrl::annotatedUrl,
                     &mainWindow, &MainWindow::displayAnnotatedUrl);
    QObject::connect(&annotatedUrl, &AnnotatedUrl::nfcStateChanged,
                     &mainWindow, &MainWindow::nfcStateChanged);
    QObject::connect(&annotatedUrl, &AnnotatedUrl::tagError,
                     &mainWindow, &MainWindow::showTagError);

    annotatedUrl.startDetection();
    mainWindow.show();

    return a.exec();
}
//! [0]
