// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "bleiodevice.h"

#include <QtWidgets/QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_connectPage(new ConnectPage())
{
    setStatusBar(new QStatusBar);
    setCentralWidget(m_stack);
    m_stack->addWidget(m_connectPage);

    QObject::connect(m_connectPage, &ConnectPage::showMessage, this, &MainWindow::showMessage);
    QObject::connect(m_connectPage, &ConnectPage::connected, this, [this](BLEIoDevice *ioDevice) {
        if (m_heaterView) {
            m_stack->removeWidget(m_heaterView);
            delete m_heaterView;
        }
        m_heaterView = new HeaterView(ioDevice, this);
        QObject::connect(m_heaterView.data(), &HeaterView::showMessage,
                         this, &MainWindow::showMessage);
        QObject::connect(m_heaterView.data(), &HeaterView::closeMe,
                         m_connectPage, &ConnectPage::disconnectFromDevice);
        QObject::connect(m_heaterView.data(), &HeaterView::closeMe,
                         this, &MainWindow::showConnectPage);
        m_stack->addWidget(m_heaterView);
        m_stack->setCurrentWidget(m_heaterView);
    });
    QObject::connect(m_connectPage, &ConnectPage::disconnected,
                     this, &MainWindow::showConnectPage);
    m_connectPage->startScanning();
}

void MainWindow::showMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::showConnectPage()
{
    if (m_heaterView) {
        m_stack->removeWidget(m_heaterView);
        delete m_heaterView;
    }
    m_connectPage->startScanning();
}
