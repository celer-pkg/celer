// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringLiteral>

class AbstractViewer;
class RecentFiles;
class ViewerFactory;
class Translator;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Translator &translator, QWidget *parent = nullptr);
    ~MainWindow();
    bool hasPlugins() const;

public slots:
    bool openFile(const QString &fileName);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onActionOpenTriggered();
    void onActionAboutTriggered();
    void onActionAboutQtTriggered();
    void onActionSwitchLanguage(QLocale::Language lang);

private:
    void readSettings();
    void saveSettings() const;
    void restoreViewerSettings();
    void resetViewer() const;
    void saveViewerSettings() const;

    QDir m_currentDir;
    AbstractViewer *m_viewer = nullptr;
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<RecentFiles> m_recentFiles;
    std::unique_ptr<ViewerFactory> m_factory;
    std::array<QMetaObject::Connection, 3> m_viewerConnections;
    Translator &m_translator;

    static constexpr QLatin1StringView settingsDir = QLatin1StringView("WorkingDir");
    static constexpr QLatin1StringView settingsMainWindow = QLatin1StringView("MainWindow");
    static constexpr QLatin1StringView settingsViewers = QLatin1StringView("Viewers");
    static constexpr QLatin1StringView settingsFiles = QLatin1StringView("RecentFiles");
};

#endif // MAINWINDOW_H
