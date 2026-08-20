// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "pdfviewer.h"
#include "zoomselector.h"
#include "hoverwatcher.h"

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QMouseEvent>

#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#include <QPdfPageSelector>

#include <QtMath>
#include <QDir>
#include <QStandardPaths>

#include <QListView>
#include <QListWidget>
#include <QMainWindow>
#include <QScrollBar>
#include <QScroller>
#include <QSpinBox>
#include <QToolBar>
#include <QTreeView>

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
#include <QPrinter>
#include <QPainter>
#endif

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

using namespace Qt::StringLiterals;

PdfViewer::PdfViewer()
    : m_actionZoomIn(new QAction(this)),
      m_actionZoomOut(new QAction(this))

{
    connect(this, &AbstractViewer::uiInitialized, this, &PdfViewer::initPdfViewer);

    QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn,
                                  QIcon(":/demos/documentviewer/images/zoom-in.png"_L1));
    m_actionZoomIn->setIcon(icon);
    m_actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(m_actionZoomIn, &QAction::triggered, this, &PdfViewer::onActionZoomInTriggered);

    icon = QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut,
                            QIcon(":/demos/documentviewer/images/zoom-out.png"_L1));
    m_actionZoomOut->setIcon(icon);
    m_actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(m_actionZoomOut, &QAction::triggered, this, &PdfViewer::onActionZoomOutTriggered);
}

void PdfViewer::init(QFile *file, QWidget *parent, QMainWindow *mainWindow)
{
    AbstractViewer::init(file, new QPdfView(parent), mainWindow);
    setTranslationBaseName("pdfviewer"_L1);
    m_document = new QPdfDocument(this);
    m_pdfView = qobject_cast<QPdfView *>(widget());
}

void PdfViewer::cleanup()
{
    delete m_pageSelector;
    m_pageSelector = nullptr;
    delete m_zoomSelector;
    m_zoomSelector = nullptr;
    delete m_pages;
    m_pages = nullptr;
    delete m_bookmarks;
    m_bookmarks = nullptr;
    delete m_document;
    m_document = nullptr;
    AbstractViewer::cleanup();
}

PdfViewer::~PdfViewer()
{
    PdfViewer::cleanup();
}

QStringList PdfViewer::supportedMimeTypes() const
{
    return {"application/pdf"_L1};
}

void PdfViewer::initPdfViewer()
{
    QToolBar *toolBar = addToolBar();
    m_zoomSelector = new ZoomSelector(toolBar);

    auto *nav = m_pdfView->pageNavigator();
    m_pageSelector = new QPdfPageSelector(toolBar);
    toolBar->insertWidget(m_uiAssets.forward, m_pageSelector);
    m_pageSelector->setDocument(m_document);
    connect(m_pageSelector, &QPdfPageSelector::currentPageChanged,
            this, &PdfViewer::pageSelected);
    connect(m_pageSelector, &QPdfPageSelector::currentPageChanged,
            this, &PdfViewer::pageSelected);
    connect(nav, &QPdfPageNavigator::currentPageChanged,
            m_pageSelector, &QPdfPageSelector::setCurrentPage);

    connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::backAvailableChanged,
            m_uiAssets.back, &QAction::setEnabled);
    m_actionBack = m_uiAssets.back;
    m_actionForward = m_uiAssets.forward;
    m_connections.append(connect(m_uiAssets.back, &QAction::triggered,
                                 this, &PdfViewer::onActionBackTriggered));
    m_connections.append(connect(m_uiAssets.forward, &QAction::triggered,
                                 this, &PdfViewer::onActionForwardTriggered));

    toolBar->addSeparator();
    toolBar->addWidget(m_zoomSelector);

    toolBar->addAction(m_actionZoomIn);

    toolBar->addAction(m_actionZoomOut);

    connect(nav, &QPdfPageNavigator::backAvailableChanged, m_actionBack, &QAction::setEnabled);
    connect(nav, &QPdfPageNavigator::forwardAvailableChanged, m_actionForward, &QAction::setEnabled);

    connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, m_pdfView, &QPdfView::setZoomMode);
    connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, m_pdfView, &QPdfView::setZoomFactor);
    m_zoomSelector->reset();
    connect(m_pdfView, &QPdfView::zoomFactorChanged, m_zoomSelector, &ZoomSelector::setZoomFactor);

    QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
    bookmarkModel->setDocument(m_document);
    m_uiAssets.tabs->clear();
    m_bookmarks = new QTreeView(m_uiAssets.tabs);
    connect(m_bookmarks, &QAbstractItemView::activated, this, &PdfViewer::bookmarkSelected);
    m_bookmarks->setModel(bookmarkModel);
    m_pdfView->setDocument(m_document);
    m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);

    openPdfFile();
    if (!m_document->pageCount())
        return;

    m_pages = new QListView(m_uiAssets.tabs);
    m_pages->setModel(m_document->pageModel());
    connect(m_pages->selectionModel(), &QItemSelectionModel::currentRowChanged, m_pages, [&]
            (const QModelIndex &current, const QModelIndex &previous){
        if (previous == current)
            return;

        auto *nav = m_pdfView->pageNavigator();
        const int &row = current.row();
        if (nav->currentPage() == row)
            return;

        nav->jump(row, QPointF(), nav->currentZoom());
    });

    connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged, m_pages, [&](int page){
       if (m_pages->currentIndex().row() == page)
           return;

       m_pages->setCurrentIndex(m_pages->model()->index(page, 0));
    });

    m_uiAssets.tabs->addTab(m_pages, {});
    m_uiAssets.tabs->addTab(m_bookmarks, {});
    QScroller::grabGesture(m_pdfView->viewport(), QScroller::ScrollerGestureType::LeftMouseButtonGesture);
    HoverWatcher::watcher(m_pdfView->viewport());

    retranslate();
}

void PdfViewer::openPdfFile()
{
    disablePrinting();

    if (m_file->open(QIODevice::ReadOnly))
        m_document->load(m_file.get());

    const auto documentTitle = m_document->metaData(QPdfDocument::MetaDataField::Title).toString();
    statusMessage(documentTitle.isEmpty() ? "PDF Viewer"_L1 : documentTitle);
    pageSelected(0);

    statusMessage(tr("Opened PDF file %1")
                  .arg(QDir::toNativeSeparators(m_file->fileName())));
    qCDebug(lcExample) << "Opened file" << m_file->fileName();

    maybeEnablePrinting();
}

bool PdfViewer::hasContent() const
{
    return m_document ? m_document->pageCount() > 0 : false;
}

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
void PdfViewer::printDocument(QPrinter *printer) const
{
    if (!hasContent())
        return;

    QPainter painter;
    painter.begin(printer);
    const QRect pageRect = printer->pageRect(QPrinter::Unit::DevicePixel).toRect();
    const QSize pageSize = pageRect.size();
    for (int i = 0; i < m_document->pageCount(); ++i) {
        if (i > 0)
            printer->newPage();
        const QImage &page = m_document->render(i, pageSize);
        painter.drawImage(pageRect, page);
    }
    painter.end();
}
#endif // DOCUMENTVIEWER_PRINTSUPPORT

void PdfViewer::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int page = index.data(int(QPdfBookmarkModel::Role::Page)).toInt();
    const qreal zoomLevel = index.data(int(QPdfBookmarkModel::Role::Level)).toReal();
    m_pdfView->pageNavigator()->jump(page, {}, zoomLevel);
}

void PdfViewer::pageSelected(int page)
{
    auto nav = m_pdfView->pageNavigator();
    nav->jump(page, {}, nav->currentZoom());
}

void PdfViewer::onActionZoomInTriggered()
{
    m_pdfView->setZoomFactor(m_pdfView->zoomFactor() * zoomMultiplier);
}

void PdfViewer::onActionZoomOutTriggered()
{
    m_pdfView->setZoomFactor(m_pdfView->zoomFactor() / zoomMultiplier);
}

void PdfViewer::onActionPreviousPageTriggered()
{
    auto nav = m_pdfView->pageNavigator();
    nav->jump(nav->currentPage() - 1, {}, nav->currentZoom());
}

void PdfViewer::onActionNextPageTriggered()
{
    auto nav = m_pdfView->pageNavigator();
    nav->jump(nav->currentPage() + 1, {}, nav->currentZoom());
}

void PdfViewer::onActionBackTriggered()
{
    m_pdfView->pageNavigator()->back();
}

void PdfViewer::onActionForwardTriggered()
{
    m_pdfView->pageNavigator()->forward();
}

void PdfViewer::retranslate()
{
    if (toolBars().isEmpty())
        return;
    toolBars().at(0)->setWindowTitle(tr("PDF"));
    m_actionZoomIn->setText(tr("Zoom in"));
    m_actionZoomOut->setText(tr("Zoom out"));
    if (m_pages && m_bookmarks && m_uiAssets.tabs) {
        int pagesIndex = m_uiAssets.tabs->indexOf(m_pages);
        if (pagesIndex >= 0)
            m_uiAssets.tabs->setTabText(pagesIndex, tr("Pages"));
        int bookmarksIndex = m_uiAssets.tabs->indexOf(m_bookmarks);
        if (bookmarksIndex >= 0)
            m_uiAssets.tabs->setTabText(bookmarksIndex, tr("Bookmarks"));
    }
}
