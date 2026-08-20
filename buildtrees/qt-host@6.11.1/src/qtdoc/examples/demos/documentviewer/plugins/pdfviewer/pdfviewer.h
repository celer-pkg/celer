// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include "viewerinterfaces.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

QT_BEGIN_NAMESPACE
class QMainWindow;
class QPdfDocument;
class QPdfView;
class QPdfPageSelector;
class QListView;
class QTabWidget;
class QTreeView;
QT_END_NAMESPACE

class ZoomSelector;
class PdfViewer : public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.DocumentViewer.ViewerInterface" FILE "pdfviewer.json")
    Q_INTERFACES(ViewerInterface)
public:
    PdfViewer();
    ~PdfViewer() override;
    void init(QFile *file, QWidget *parent, QMainWindow *mainWindow) override;
    void cleanup() override;
    QString viewerName() const override { return QLatin1StringView(staticMetaObject.className()); };
    QStringList supportedMimeTypes() const override;
    bool supportsOverview() const override { return true; }
    bool hasContent() const override;
    QByteArray saveState() const override { return QByteArray(); }
    bool restoreState(QByteArray &) override { return true; }

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
protected:
    void printDocument(QPrinter *printer) const override;
#endif // DOCUMENTVIEWER_PRINTSUPPORT

public slots:
    void openPdfFile();

private slots:
    void initPdfViewer();
    void bookmarkSelected(const QModelIndex &index);
    void pageSelected(int page);

    // action handlers
    void onActionZoomInTriggered();
    void onActionZoomOutTriggered();
    void onActionPreviousPageTriggered();
    void onActionNextPageTriggered();
    void onActionBackTriggered();
    void onActionForwardTriggered();

private:
    void retranslate() override;
    void populateQuestions();

    const qreal zoomMultiplier = qSqrt(2.0);
    ZoomSelector *m_zoomSelector = nullptr;
    QPdfPageSelector *m_pageSelector = nullptr;
    QPdfDocument *m_document = nullptr;
    QPdfView *m_pdfView = nullptr;
    QAction *m_actionForward = nullptr;
    QAction *m_actionBack = nullptr;
    QTreeView *m_bookmarks = nullptr;
    QListView *m_pages = nullptr;
    QAction *m_actionZoomIn = nullptr;
    QAction *m_actionZoomOut = nullptr;
};

#endif //PDFVIEWER_H
