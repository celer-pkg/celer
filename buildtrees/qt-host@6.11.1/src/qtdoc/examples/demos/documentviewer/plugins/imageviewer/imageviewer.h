// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "viewerinterfaces.h"

#include <QSizeF>

QT_FORWARD_DECLARE_CLASS(QLabel)

class ImageViewer : public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.DocumentViewer.ViewerInterface" FILE "imageviewer.json")
    Q_INTERFACES(ViewerInterface)
public:
    Q_DISABLE_COPY_MOVE(ImageViewer)

    ImageViewer();
    ~ImageViewer() override;

    void init(QFile *file, QWidget *parent, QMainWindow *mainWindow) override;
    QString viewerName() const override { return QLatin1StringView(staticMetaObject.className()); };
    QStringList supportedMimeTypes() const override;
    bool hasContent() const override;
    QByteArray saveState() const override { return {}; }
    bool restoreState(QByteArray &) override { return true; }
    bool supportsOverview() const override { return false; }

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
protected:
    void printDocument(QPrinter *printer) const override;
#endif // DOCUMENTVIEWER_PRINTSUPPORT

private slots:
    void setupImageUi();
    void clear();
    void zoomIn();
    void zoomOut();
    void resetZoom();

private:
    void retranslate() override;
    void openFile();
    void setScaleFactor(qreal scaleFactor);
    void doSetScaleFactor(qreal scaleFactor);
    void enableZoomActions();

    QLabel *m_imageLabel{};
    QAction *m_zoomInAct{};
    QAction *m_zoomOutAct{};
    QAction *m_resetZoomAct{};
    QStringList m_formats;
    qreal m_scaleFactor = 1;
    qreal m_initialScaleFactor = 1;
    qreal m_minScaleFactor = 1;
    qreal m_maxScaleFactor = 1;
    QSizeF m_imageSize;
};

#endif //IMAGEVIEWER_H
