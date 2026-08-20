// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VIEWERFACTORY_H
#define VIEWERFACTORY_H

#include <QString>
#include <QMap>

QT_BEGIN_NAMESPACE
class QMimeType;
class QWidget;
class QMainWindow;
class QFile;
QT_END_NAMESPACE

class AbstractViewer;

class ViewerFactory
{
public:
    enum class DefaultPolicy {
        NeverDefault,
        DefaultToTxtViewer,
        DefaultToCustomViewer
    };

    explicit ViewerFactory(QWidget *displayWidget, QMainWindow *mainWindow,
                           DefaultPolicy policy = DefaultPolicy::DefaultToTxtViewer);
    ~ViewerFactory();

    DefaultPolicy defaultPolicy() const { return m_defaultPolicy; }
    void setDefaultPolicy(DefaultPolicy policy) { m_defaultPolicy = policy; }
    bool defaultWarning() const { return m_defaultWarning; }
    void setDefaultWarning(bool on) { m_defaultWarning = on; }

    AbstractViewer *viewer(QFile *file) const;

    using ViewerMap = QMap<QString, AbstractViewer *>;
    using ViewerList = QList<AbstractViewer *>;
    QStringList viewerNames(bool showDefault = false) const;
    ViewerList viewers() const;
    AbstractViewer *findViewer(const QString &viewerName) const;
    AbstractViewer *defaultViewer() const;
    QStringList supportedMimeTypes() const;

private:
    DefaultPolicy m_defaultPolicy;
    QWidget *m_displayWidget;
    QMainWindow *m_mainWindow;
    ViewerMap m_viewers;
    AbstractViewer *m_defaultViewer = nullptr;
    bool m_defaultWarning = true;

    void loadViewerPlugins();
    void addViewer(QObject *viewerObject);
    AbstractViewer *viewer(const QMimeType &type) const;
    AbstractViewer *viewer(const QString &extension) const;
    void unload();
};

#endif // VIEWERFACTORY_H
