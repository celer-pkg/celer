// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileSystemWatcher>
#include "svgview.h"

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }

class QLabel;

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    bool loadFile(const QString &path);

public slots:
    void openFile();
    void exportImage();

    void setBackground(QAction *action);
    void setFeature(QAction *action);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void fileModified(const QString &path);

    void updateZoomLabel();
    void zoomIn();
    void zoomOut();
    void resetZoom();

private:
    Ui::MainWindow *ui;
    SvgView *m_svgView;
    QLabel *m_zoomLabel;

    QFileSystemWatcher m_watcher;
    QString m_currentPath;
    qreal m_scale = 1;
};

#endif // MAINWINDOW_H
