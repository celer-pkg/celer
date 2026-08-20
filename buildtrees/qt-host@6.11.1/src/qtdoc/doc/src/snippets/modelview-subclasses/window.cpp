// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QAction>
#include <QDataStream>
#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QFileDialog>
#include <QListView>

#include "window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Model/View example");

    setupModelView();

    QAction *openAction = new QAction(tr("&Open"), this);
    QAction *quitAction = new QAction(tr("E&xit"), this);
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAction);
    fileMenu->addAction(quitAction);
    menuBar()->addMenu(fileMenu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(selectOpenFile()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    setCentralWidget(view);
}

void MainWindow::setupModelView()
{
    model = new LinearModel(this);
    view = new LinearView(this);
    view->setModel(model);
}

void MainWindow::selectOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select a file to open"), "", tr("Sound files (*.wav)"));

    if (!fileName.isEmpty())
        openFile(fileName);
}

void MainWindow::openFile(const QString &fileName)
{
    QFile file(fileName);
    int length = file.size();

    if (file.open(QFile::ReadOnly)) {
        model->removeRows(0, model->rowCount());

        int rows = (length - 0x2c)/2;
        model->insertRows(0, rows);

        // Perform some dodgy tricks to extract the data from the file.
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        Q_INT16 left;
        Q_INT16 right;

        for (int row = 0; row < rows; ++row) {
            QModelIndex index = model->index(row);

            stream >> left >> right;
            model->setData(index, int(left / 256));
        }
    }
}
