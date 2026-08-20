// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exportdialog.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QWheelEvent>

static inline QString picturesLocation()
{
    return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).value(0, QDir::currentPath());
}

MainWindow::MainWindow()
    : QMainWindow()
    , ui(new Ui::MainWindow)
    , m_svgView(new SvgView)
    , m_zoomLabel(new QLabel)
{
    ui->setupUi(this);

    ui->scrollArea->setWidget(m_svgView);
    ui->scrollArea->widget()->installEventFilter(this);

    ui->actionBlack->setIcon(QIcon(QLatin1String(":/files/black.png")));
    ui->actionWhite->setIcon(QIcon(QLatin1String(":/files/white.png")));
    ui->actionChequered->setIcon(QIcon(QLatin1String(":/files/chequered.png")));

    QActionGroup *backgroundActionGroup = new QActionGroup(this);
    backgroundActionGroup->addAction(ui->actionBlack);
    backgroundActionGroup->addAction(ui->actionWhite);
    backgroundActionGroup->addAction(ui->actionChequered);

    connect(backgroundActionGroup, &QActionGroup::triggered, this, &MainWindow::setBackground);
    connect(ui->actionOutline, &QAction::toggled, m_svgView, &SvgView::setOutline);


    connect(ui->actionAnimations, &QAction::toggled, m_svgView, &SvgView::setAnimations);
    connect(ui->actionRestart_Animations, &QAction::triggered, m_svgView, &SvgView::restartAnimations);

    QActionGroup *featuresActionGroup = new QActionGroup(this);
    featuresActionGroup->addAction(ui->actionExtended);
    featuresActionGroup->addAction(ui->actionTiny_1_2);
    connect(featuresActionGroup, &QActionGroup::triggered, this, &MainWindow::setFeature);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::exportImage);

    connect(ui->actionZoom_In, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(ui->actionZoom_Out, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(ui->actionReset_Zoom, &QAction::triggered, this, &MainWindow::resetZoom);
    connect(ui->actionRefresh, &QAction::triggered, m_svgView, &SvgView::reload);

    connect(ui->actionAssume_Trusted_Source, &QAction::toggled, m_svgView, &SvgView::setAssumeTrustedSource);

    QToolBar *toolBar = new QToolBar(this);
    addToolBar(Qt::TopToolBarArea, toolBar);

    toolBar->addAction(ui->actionOpen);
    toolBar->addAction(ui->actionExport);

    toolBar->insertSeparator(ui->actionZoom_In);
    toolBar->addAction(ui->actionZoom_In);
    toolBar->addAction(ui->actionZoom_Out);
    toolBar->addAction(ui->actionRefresh);

    toolBar->insertSeparator(ui->actionWhite);
    toolBar->addAction(ui->actionWhite);
    toolBar->addAction(ui->actionBlack);
    toolBar->addAction(ui->actionChequered);

    m_zoomLabel->setToolTip(tr("Use the mouse wheel to zoom"));
    statusBar()->addPermanentWidget(m_zoomLabel);
    updateZoomLabel();

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::fileModified);
}

void MainWindow::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setMimeTypeFilters(QStringList() << "image/svg+xml" << "image/svg+xml-compressed");
    fileDialog.setWindowTitle(tr("Open SVG File"));
    if (m_currentPath.isEmpty())
        fileDialog.setDirectory(picturesLocation());

    while (fileDialog.exec() == QDialog::Accepted && !loadFile(fileDialog.selectedFiles().constFirst()))
        ;
}

bool MainWindow::loadFile(const QString &fileName)
{
    if (!QFileInfo::exists(fileName)) {
        QMessageBox::critical(this, tr("Open SVG File"),
                              tr("Could not open file '%1'.").arg(QDir::toNativeSeparators(fileName)));
        return false;
    }

    if (!m_svgView->currentFile().empty())
        m_watcher.removePath(m_svgView->currentFile().toString());
    m_watcher.addPath(fileName);
    m_svgView->load(fileName);

    resetZoom();

    if (!fileName.startsWith(":/")) {
        m_currentPath = fileName;
        setWindowFilePath(fileName);
        const QSize size = m_svgView->fileSize();
        const QString message =
            tr("Opened %1, %2x%3").arg(QFileInfo(fileName).fileName()).arg(size.width()).arg(size.width());
        statusBar()->showMessage(message);
    }

    const QSize availableSize = this->screen()->availableGeometry().size();
    resize(m_svgView->fileSize().expandedTo(availableSize / 4) + QSize(80, 80 + menuBar()->height()));

    return true;
}

void MainWindow::fileModified(const QString &path)
{
    if (!ui->actionAuto_Refresh->isChecked())
        return;

    if (m_watcher.files().contains(path) && m_svgView->currentFile() == path)
        m_svgView->reload();
}

void MainWindow::setBackground(QAction *action)
{
    if (action == ui->actionWhite)
        m_svgView->setBackgroundType(SvgView::Background::White);
    else if (action == ui->actionBlack)
        m_svgView->setBackgroundType(SvgView::Background::Black);
    else if (action == ui->actionChequered)
        m_svgView->setBackgroundType(SvgView::Background::Chequered);
}

void MainWindow::setFeature(QAction *action)
{
    if (action == ui->actionTiny_1_2)
        m_svgView->setFeature(SvgView::Feature::Tiny_12);
    else
        m_svgView->setFeature(SvgView::Feature::Extended);
}

void MainWindow::exportImage()
{
    ExportDialog exportDialog(this);
    exportDialog.setExportSize(m_svgView->fileSize());

    QString fileName;
    if (m_currentPath.isEmpty()) {
        fileName = picturesLocation() + QLatin1String("/export.png");
    } else {
        const QFileInfo fi(m_currentPath);
        fileName = fi.absolutePath() + QLatin1Char('/') + fi.baseName() + QLatin1String(".png");
    }

    exportDialog.setExportFileName(fileName);
    exportDialog.exec();

    if (exportDialog.result() == QDialog::Accepted) {
        const QSize imageSize = exportDialog.exportSize();
        QImage renderedSvg = m_svgView->renderAsImage(imageSize);

        fileName = exportDialog.exportFileName();
        if (renderedSvg.save(fileName)) {
            const QString message = tr("Exported %1, %2x%3, %4 bytes")
            .arg(QDir::toNativeSeparators(fileName)).arg(imageSize.width()).arg(imageSize.height())
                .arg(QFileInfo(fileName).size());
            statusBar()->showMessage(message);
        } else {
            QMessageBox::critical(this, tr("Export Image"),
                                  tr("Could not write file '%1'.").arg(QDir::toNativeSeparators(fileName)));
        }
    }
}

void MainWindow::updateZoomLabel()
{
    const int percent = qRound(m_scale * qreal(100));
    m_zoomLabel->setText(QString::number(percent) + QLatin1Char('%'));
}

void MainWindow::zoomIn()
{
    m_scale = qMin(m_scale * 2, 10.);
    m_svgView->setScale(m_scale);
    updateZoomLabel();
}

void MainWindow::zoomOut()
{
    m_scale = qMax(m_scale / 2, 0.5);
    m_svgView->setScale(m_scale);
    updateZoomLabel();
}

void MainWindow::resetZoom()
{
    m_scale = 1.;
    m_svgView->setScale(m_scale);
    updateZoomLabel();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->scrollArea->widget() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        m_scale *= qBound(0., qPow(1.2, wheelEvent->angleDelta().y() / 240.), 2.);
        m_scale = qBound(0.1, m_scale, 10.);
        m_svgView->setScale(m_scale);
        updateZoomLabel();
        return true;
    }
    return false;
}
