// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>
#include <QtWidgets>

#include "inputlistview.h"
#include "inputtab.h"
#include "mainwindow.h"
#include "settingstab.h"

const auto WINDOW_TITLE = "Balsam UI";

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    settingsTab = new SettingsTab;
    generalTab = new InputTab(settingsTab);
    tabWidget = new QTabWidget;
    tabWidget->addTab(generalTab, tr("Input"));
    tabWidget->addTab(settingsTab, tr("Settings"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle(tr(WINDOW_TITLE));
}
