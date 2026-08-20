// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "q3dviewer.h"

#include <QApplication>
#include <QMimeData>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QMimeDatabase>

#include <QWidget>

#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickView>
#include <QQuickItem>
#include <QQuick3D>

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
#include <QPrinter>
#include <QPainter>
#endif

using namespace Qt::StringLiterals;

Q3DViewer::Q3DViewer()
{
    connect(this, &AbstractViewer::uiInitialized, this, &Q3DViewer::openQ3DFile);
}

void Q3DViewer::init(QFile *file, QWidget *parent, QMainWindow *mainWindow)
{
    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());
    setTranslationBaseName("q3dviewer"_L1);
    m_quickView = new QQuickView;
    m_quickView->loadFromModule(u"Q3DViewer", "Viewer");
    Q_ASSERT(m_quickView->status() != QQuickView::Status::Error);
    AbstractViewer::init(file, QWidget::createWindowContainer(m_quickView, parent), mainWindow);
}

Q3DViewer::~Q3DViewer()
{
    Q3DViewer::cleanup();
}

void Q3DViewer::cleanup()
{
    if (!m_quickView)
        return;

    delete m_quickView;
    m_quickView = nullptr;
}

QStringList Q3DViewer::supportedMimeTypes() const
{
    static QStringList mimeTypes;
    static bool mimeTypesSearched = false;
    if (mimeTypesSearched)
        return mimeTypes;

    mimeTypesSearched = true;

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadFromModule(u"Q3DViewer", "QueryMimeTypes");
    std::unique_ptr<QObject> loader(component.create());
    if (!loader)
        return {};

    const auto &mt = qvariant_cast<QList<QMimeType>>(loader->property("supportedMimeTypes"));
    for (const auto &type : mt)
        mimeTypes << type.name();
    return mimeTypes;
}

bool Q3DViewer::openQ3DFile()
{
    Q_ASSERT(m_file);

    if (!m_quickView || !m_file->open(QIODevice::ReadOnly))
        return false;

    const auto fileName = QUrl::fromLocalFile(QFileInfo(*m_file).absoluteFilePath());
    m_quickView->rootObject()->setProperty("isMesh", type() == FileType::Mesh);
    m_quickView->rootObject()->setProperty("fileName", fileName);
    m_quickView->show();

    return true;
}

Q3DViewer::FileType Q3DViewer::type() const
{
    Q_ASSERT(m_file);
    return (QFileInfo(*m_file).suffix() == "mesh"_L1) ? FileType::Mesh : FileType::External;
}

bool Q3DViewer::hasContent() const
{
    return false;
}

#ifdef DOCUMENTVIEWER_PRINTSUPPORT
void Q3DViewer::printDocument(QPrinter *printer) const
{
    Q_UNUSED(printer);
}
#endif // DOCUMENTVIEWER_PRINTSUPPORT

QByteArray Q3DViewer::saveState() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString(viewerName());
    return array;
}

bool Q3DViewer::restoreState(QByteArray &array)
{
    QDataStream stream(&array, QIODevice::ReadOnly);
    QString viewer;
    stream >> viewer;
    if (viewer != viewerName())
        return false;
    return true;
}
