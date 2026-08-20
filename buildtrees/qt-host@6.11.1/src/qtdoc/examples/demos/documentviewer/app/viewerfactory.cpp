// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "abstractviewer.h"
#include "viewerfactory.h"
#include "viewerinterfaces.h"

#include <QApplication>
#include <QMessageBox>
#include <QWidget>

#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPluginLoader>
#include <QTimer>

using namespace Qt::StringLiterals;

namespace {
    struct Tr {
        Q_DECLARE_TR_FUNCTIONS(ViewerFactory);
    };
}

ViewerFactory::ViewerFactory(QWidget *displayWidget, QMainWindow *mainWindow, DefaultPolicy policy)
    : m_defaultPolicy(policy),
      m_displayWidget(displayWidget),
      m_mainWindow(mainWindow)
{
    Q_ASSERT(m_displayWidget);
    Q_ASSERT(m_mainWindow);
    loadViewerPlugins();
}

ViewerFactory::~ViewerFactory()
{
    unload();
}

AbstractViewer *ViewerFactory::viewer(QFile *file) const
{
    Q_ASSERT(file);

    const QFileInfo info(*file);

    // Find via extension
    AbstractViewer *viewer = ViewerFactory::viewer(info.suffix());

    // Find via mime type
    if (!viewer) {
        QMimeDatabase db;
        const auto mimeType = db.mimeTypeForFile(info);
        viewer = ViewerFactory::viewer(mimeType);
        if (!viewer) {
            qWarning() << "Mime type" << mimeType.name() << "not supported.";
            return nullptr;
        }
    }

    Q_ASSERT(viewer);
    viewer->init(file, m_displayWidget, m_mainWindow);
    return viewer;
}

//! [loader]
void ViewerFactory::loadViewerPlugins()
{
    if (!m_viewers.isEmpty())
        return;
//! [loader]

//! [static]
    // Load static plugins
    const QObjectList &staticPlugins = QPluginLoader::staticInstances();
    for (auto *plugin : staticPlugins)
        addViewer(plugin);
//! [static]

//! [shared]
    // Load shared plugins
    QDir pluginsDir = QDir(QApplication::applicationDirPath());
#if defined(Q_OS_DARWIN)
    if (pluginsDir.exists("../PlugIns"_L1)) { // installed build
        pluginsDir.cd("../PlugIns"_L1);
    } else {
        pluginsDir.cd("../../../../plugins"_L1); // non-installed build
    }
#elif defined(Q_OS_WIN)
    if (pluginsDir.exists("plugins"_L1)) { // non-installed build
        pluginsDir.cd("plugins"_L1);
    } else {
        pluginsDir.cd("../plugins"_L1); // installed build
    }
#else
    pluginsDir.cd("../plugins"_L1); // installed and non-installed build
#endif

    // qDebug("Loading plugins from %s...", qUtf8Printable(pluginsDir.path()));
    const auto entryList = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : entryList) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin)
            addViewer(plugin);
#if 0
        else
            qDebug() << loader.errorString();
#endif
    }
}
//! [shared]

void ViewerFactory::unload()
{
    qDeleteAll(viewers());
    m_viewers.clear();
}

void ViewerFactory::addViewer(QObject *viewerObject)
{
    auto *interface = qobject_cast<ViewerInterface *>(viewerObject);
    if (!interface)
        return;

    // discard viewers, which don't support any MIME types
    if (interface->supportedMimeTypes().isEmpty()) {
        delete interface;
        return;
    }

    // Set custom default viewer
    if (interface->viewer()->isDefaultViewer())
        m_defaultViewer = interface->viewer();

    m_viewers.insert(interface->viewerName(), interface->viewer());
}

QStringList ViewerFactory::viewerNames(bool showDefault) const
{
    if (!showDefault)
        return m_viewers.keys();

    QStringList list;
    for (auto it = m_viewers.constBegin(); it != m_viewers.constEnd(); ++it) {
        QString name = it.key();
        if ((m_defaultViewer && it.value()->isDefaultViewer())
             || (!m_defaultViewer && it.key() == "TxtViewer"_L1)) {
            name += "(default)"_L1;
        }
        list.append(name);
    }
    return list;
}

ViewerFactory::ViewerList ViewerFactory::viewers() const
{
    return m_viewers.values();
}

AbstractViewer *ViewerFactory::findViewer(const QString &viewerName) const
{
    const ViewerList &viewerList = viewers();
    for (auto *viewer : viewerList) {
        if (viewer->viewerName() == viewerName)
            return viewer;
    }
    qWarning() << "Plugin" << viewerName << "not loaded.";
    return nullptr;
}

AbstractViewer *ViewerFactory::viewer(const QMimeType &mimeType) const
{
    const ViewerList &viewerList = viewers();

    for (AbstractViewer *viewer : viewerList) {
        for (const QString &type : viewer->supportedMimeTypes()) {
            if (mimeType.inherits(type))
                return viewer;
        }
    }

    AbstractViewer *viewer = defaultViewer();
    if (!viewer) {
        QMessageBox mbox;
        mbox.setIcon(QMessageBox::Warning);
        mbox.setText(Tr::tr("No viewers for the chosen file format."));
        mbox.setStandardButtons(QMessageBox::Ok);
        QTimer::singleShot(8000, &mbox, [&mbox]() { mbox.close(); });
        mbox.exec();
        return nullptr;
    }

    if (m_defaultWarning) {
        QMessageBox mbox;
        mbox.setIcon(QMessageBox::Warning);
        mbox.setText(Tr::tr("Mime type %1 not supported. Falling back to %2.")
                     .arg(mimeType.name(), viewer->viewerName()));
        mbox.setStandardButtons(QMessageBox::Ok);
        QTimer::singleShot(8000, &mbox, [&mbox](){ mbox.close(); });
        mbox.exec();
    }

    return viewer;
}

AbstractViewer *ViewerFactory::viewer(const QString &extension) const
{
    const ViewerList &viewerList = viewers();
    for (AbstractViewer *viewer : viewerList) {
        if (viewer->supportedExtensions().contains(extension))
            return viewer;
    }
    return nullptr;
}

AbstractViewer *ViewerFactory::defaultViewer() const
{
    switch (m_defaultPolicy) {
    case DefaultPolicy::NeverDefault:
        return nullptr;
    case DefaultPolicy::DefaultToCustomViewer:
        return m_defaultViewer ? m_defaultViewer : findViewer("TxtViewer"_L1);
    case DefaultPolicy::DefaultToTxtViewer:
        return findViewer("TxtViewer"_L1);
    }
    Q_UNREACHABLE();
}

QStringList ViewerFactory::supportedMimeTypes() const
{
    static QStringList mimeTypes;
    if (!mimeTypes.isEmpty())
        return mimeTypes;

    const ViewerList &viewerList = viewers();
    for (auto viewer : viewerList) {
        mimeTypes.append(viewer->supportedMimeTypes());
        const QStringList &extensions = viewer->supportedExtensions();
        if (extensions.isEmpty())
            continue;
        mimeTypes << (Tr::tr("Plus extensions: %1").arg(extensions.join(","_L1)));
    }

    return mimeTypes;
}
