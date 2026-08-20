// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "canvaswidget.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QMdiArea>
#include <QMdiSubWindow>

#include <QKeySequence>

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    void createCanvasWidget();

    QMdiArea *mdi;
};

MainWindow::MainWindow()
{
    mdi = new QMdiArea;
    setCentralWidget(mdi);

    createCanvasWidget();

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New widget"),
                        QKeySequence(QKeySequence::StandardKey::New),
                        this, &MainWindow::createCanvasWidget);
    fileMenu->addAction(tr("E&xit"),
                        QKeySequence(QKeySequence::StandardKey::Quit),
                        qApp, &QCoreApplication::quit);
}

void MainWindow::createCanvasWidget()
{
    CanvasWidget *canvasWidget = new CanvasWidget;
    mdi->addSubWindow(canvasWidget)->resize(500, 500);
    canvasWidget->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.resize(1280, 720);
    mainWindow.show();

    return QCoreApplication::exec();
}
