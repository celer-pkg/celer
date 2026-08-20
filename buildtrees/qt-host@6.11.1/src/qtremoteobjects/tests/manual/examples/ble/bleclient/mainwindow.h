// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectpage.h"
#include "heaterview.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

public slots:
    void showMessage(const QString &message);

private:
    void showConnectPage();

private:
    QStackedWidget *m_stack;
    ConnectPage *m_connectPage;
    QPointer<HeaterView> m_heaterView;
};

#endif // MAINWINDOW_H
