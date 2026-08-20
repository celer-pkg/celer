// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPixmap>

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QMouseEvent)
QT_FORWARD_DECLARE_CLASS(QUrl)
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QScreen)

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void displayAnnotatedUrl(const QUrl &url, const QString &title, const QPixmap &pixmap);
    void nfcStateChanged(bool enabled);
    void showTagError(const QString &message);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private slots:
    void updateWidgetLayout(Qt::ScreenOrientation orientation);

private:
    void handleScreenChange();

    QLabel *m_titleLabel;
    QLabel *m_infoLabel;
    QLabel *m_uriLabel;
    QLabel *m_landscapeIconLabel;
    QLabel *m_portraitIconLabel;
    QScreen *m_screen = nullptr;
    QPixmap m_pixmap;
};

#endif // MAINWINDOW_H
