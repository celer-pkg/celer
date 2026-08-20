// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewerfactory.h"
#include "abstractviewer.h"
#include "recentfiles.h"
#include "recentfilemenu.h"
#include "translator.h"

#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>

#include <QDir>
#include <QSettings>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(Translator &translator, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_translator(translator)
{
    ui->setupUi(this);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);
    connect(ui->actionAboutQt, &QAction::triggered, this, &MainWindow::onActionAboutQtTriggered);
    connect(ui->actionDeutsch, &QAction::triggered, this,
            [this] { onActionSwitchLanguage(QLocale::German); });
    connect(ui->actionEnglish, &QAction::triggered, this,
            [this] { onActionSwitchLanguage(QLocale::English); });

    m_recentFiles.reset(new RecentFiles(ui->actionRecent));
    connect(m_recentFiles.get(), &RecentFiles::countChanged, this, [&](int count){
        ui->actionRecent->setText(tr("%n recent files", nullptr, count));
    });

    readSettings();
    m_factory.reset(new ViewerFactory(ui->viewArea, this));
    const QStringList &viewers = m_factory->viewerNames();

    const QString msg = tr("Available viewers: %1").arg(viewers.join(", "_L1));
    statusBar()->showMessage(msg);

    auto *menu = new RecentFileMenu(this, m_recentFiles.get());
    ui->actionRecent->setMenu(menu);
    connect(menu, &RecentFileMenu::fileOpened, this, &MainWindow::openFile);
    QWidget *w = ui->mainToolBar->widgetForAction(ui->actionRecent);
    auto *button = qobject_cast<QToolButton *>(w);
    if (button)
        connect(ui->actionRecent, &QAction::triggered, button, &QToolButton::showMenu);
}

bool MainWindow::hasPlugins() const
{
    return m_factory ? !m_factory->viewers().isEmpty() : false;
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::onActionSwitchLanguage(QLocale::Language lang)
{
    QLocale::setDefault(QLocale(lang));
    QEvent event(QEvent::LocaleChange);
    QCoreApplication::sendEvent(this, &event);
}

void MainWindow::onActionOpenTriggered()
{
    QFileDialog fileDialog(this, tr("Open Document"), m_currentDir.absolutePath());
    fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
    while (fileDialog.exec() == QDialog::Accepted
           && !openFile(fileDialog.selectedFiles().constFirst())) {
    }
}

bool MainWindow::openFile(const QString &fileName)
{
    QFile *file = new QFile(fileName);
    if (!file->exists()) {
        statusBar()->showMessage(tr("File %1 could not be opened")
                                 .arg(QDir::toNativeSeparators(fileName)));
        delete file;
        return false;
    }

    QFileInfo fileInfo(*file);
    m_currentDir = fileInfo.dir();
    m_recentFiles->addFile(fileInfo.absoluteFilePath());

    // If a viewer is already open, clean it up and save its settings
    resetViewer();
    m_viewer = m_factory->viewer(file);
    if (!m_viewer) {
        statusBar()->showMessage(tr("File %1 can't be opened.")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;
    }

    for (const QMetaObject::Connection &connection : m_viewerConnections)
        disconnect(connection);

    m_viewerConnections = {
        connect(m_viewer, &AbstractViewer::printingEnabledChanged, ui->actionPrint,
                &QAction::setEnabled),
        connect(ui->actionPrint, &QAction::triggered, m_viewer, &AbstractViewer::print),
        connect(m_viewer, &AbstractViewer::showMessage, statusBar(), &QStatusBar::showMessage)
    };

    m_viewer->initViewer(ui->actionBack, ui->actionForward, ui->menuHelp->menuAction(), ui->tabWidget);
    restoreViewerSettings();
    ui->scrollArea->setWidget(m_viewer->widget());
    return true;
}

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        statusBar()->clearMessage();
        break;
    case QEvent::LocaleChange:
        m_translator.setLanguage(QLocale().language());
        m_translator.install();
        break;
    default:
        break;
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::onActionAboutTriggered()
{
    const QString viewerNames = m_factory->viewerNames().join(", "_L1);
    const QString mimeTypes = m_factory->supportedMimeTypes().join(u'\n');
    QString text = tr("A Widgets application to display and print JSON, "
                      "text and PDF files. Demonstrates various features to use "
                      "in widget applications: Using QSettings, query and save "
                      "user preferences, manage file histories and control cursor "
                      "behavior when hovering over widgets.\n\n"
                      "This version has loaded the following plugins:\n%1\n"
                      "\n\nIt supports the following mime types:\n%2")
                    .arg(viewerNames, mimeTypes);

    if (auto *def = m_factory->defaultViewer())
        text += tr("\n\nOther mime types will be displayed with %1.").arg(def->viewerName());

    QMessageBox::about(this, tr("About Document Viewer Demo"), text);
}

void MainWindow::onActionAboutQtTriggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::readSettings()
{
    QSettings settings;

    // Restore working directory
    if (settings.contains(settingsDir))
        m_currentDir = QDir(settings.value(settingsDir).toString());
    else
        m_currentDir = QDir::current();

    // Restore QMainWindow state
    if (settings.contains(settingsMainWindow)) {
        QByteArray mainWindowState = settings.value(settingsMainWindow).toByteArray();
        restoreState(mainWindowState);
    }

    // Restore recent files
    m_recentFiles->restoreFromSettings(settings, settingsFiles);
}

void MainWindow::saveSettings() const
{
    QSettings settings;

    // Save working directory
    settings.setValue(settingsDir, m_currentDir.absolutePath());

    // Save QMainWindow state
    settings.setValue(settingsMainWindow, saveState());

    // Save recent files
    m_recentFiles->saveSettings(settings, settingsFiles);

    settings.sync();
}

void MainWindow::saveViewerSettings() const
{
    if (!m_viewer)
        return;

    QSettings settings;
    settings.beginGroup(settingsViewers);
        settings.setValue(m_viewer->viewerName(), m_viewer->saveState());
    settings.endGroup();
    settings.sync();
}

void MainWindow::resetViewer() const
{
    if (!m_viewer)
        return;

    saveViewerSettings();
    m_viewer->cleanup();
}

void MainWindow::restoreViewerSettings()
{
    if (!m_viewer)
        return;

    QSettings settings;
    settings.beginGroup(settingsViewers);
    QByteArray viewerSettings = settings.value(m_viewer->viewerName(), QByteArray()).toByteArray();
    settings.endGroup();
    if (!viewerSettings.isEmpty())
        m_viewer->restoreState(viewerSettings);
}

