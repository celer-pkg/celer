// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QWidget>

class SlideShowPrivate;

class SlideShow : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int slideInterval READ slideInterval WRITE setSlideInterval)

public:
    SlideShow(QWidget* parent = 0);
    ~SlideShow();
    void addImage(QString filename);
    void addImageDir(QString dirName);
    void clearImages();
    void startShow();
    void stopShow();


    int slideInterval();
    void setSlideInterval(int val);

signals:
    void inputReceived();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void showEvent(QShowEvent *event ) override;


private:
    SlideShowPrivate* d;
};













#endif
