// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef Q3DVIEWER_H
#define Q3DVIEWER_H

#include "viewerinterfaces.h"
#include <QCache>
#include <QTemporaryDir>

QT_BEGIN_NAMESPACE
class QQuickView;
QT_END_NAMESPACE

class Q3DViewer : public ViewerInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.DocumentViewer.ViewerInterface/1.0" FILE "q3dviewer.json")
    Q_INTERFACES(ViewerInterface)
public:
    Q3DViewer();
    ~Q3DViewer() override;

    void init(QFile *file, QWidget *parent, QMainWindow *mainWindow) override;
    QString viewerName() const override { return QLatin1StringView(staticMetaObject.className()); };
    QStringList supportedMimeTypes() const override;
    QByteArray saveState() const override;
    bool supportsOverview() const override { return false; }
    bool restoreState(QByteArray &) override;
    bool hasContent() const override;
    void cleanup() override;
    QStringList supportedExtensions() const override { return {QLatin1StringView("mesh")}; };

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
protected:
    void printDocument(QPrinter *printer) const override;
#endif // QT_ABSTRACTVIEWER_PRINTSUPPORT

private slots:
    bool openQ3DFile();

private:
    enum class FileType {
        External,
        Mesh,
    };

    FileType type() const;
    QQuickView *m_quickView = nullptr;
};

#endif // #ifndef Q3DVIEWER_H
