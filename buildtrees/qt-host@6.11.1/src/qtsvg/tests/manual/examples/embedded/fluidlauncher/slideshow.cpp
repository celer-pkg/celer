// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QBasicTimer>
#include <QList>
#include <QImage>
#include <QDir>
#include <QPainter>
#include <QPaintEvent>

#include <QDebug>


#include "slideshow.h"


class SlideShowPrivate
{
public:
    SlideShowPrivate();

    int currentSlide;
    int slideInterval;
    QBasicTimer interSlideTimer;
    QStringList imagePaths;

    void showNextSlide();
};



SlideShowPrivate::SlideShowPrivate()
{
    currentSlide = 0;
    slideInterval = 10000; // Default to 10 sec interval
}


void SlideShowPrivate::showNextSlide()
{
    currentSlide++;
    if (currentSlide >= imagePaths.size())
      currentSlide = 0;
}



SlideShow::SlideShow(QWidget* parent) :
    QWidget(parent)
{
    d = new SlideShowPrivate;

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setMouseTracking(true);
}


SlideShow::~SlideShow()
{
    delete d;
}


void SlideShow::addImageDir(QString dirName)
{
    QDir dir(dirName);

    // lookup in directories
    QStringList fileNames = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
    for (int i=0; i<fileNames.count(); i++)
        d->imagePaths << dir.absoluteFilePath(fileNames[i]);

    // lookup in qrc
    dir = QDir(QString(":/fluidlauncher/" + dirName));
    fileNames = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
    for (int i=0; i<fileNames.count(); i++)
        d->imagePaths << dir.absoluteFilePath(fileNames[i]);
}

void SlideShow::addImage(QString filename)
{
    d->imagePaths << filename;
}


void SlideShow::clearImages()
{
    d->imagePaths.clear();
}


void SlideShow::startShow()
{
    d->interSlideTimer.start(d->slideInterval, this);
    d->showNextSlide();
    update();
}


void SlideShow::stopShow()
{
    d->interSlideTimer.stop();
}


int SlideShow::slideInterval()
{
    return d->slideInterval;
}

void SlideShow::setSlideInterval(int val)
{
    d->slideInterval = val;
}


void SlideShow::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    d->showNextSlide();
    update();
}


void SlideShow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (d->imagePaths.size() > 0) {
        QPixmap slide = QPixmap(d->imagePaths[d->currentSlide]);
        QSize slideSize = slide.size();
        QSize scaledSize = QSize(qMin(slideSize.width(), size().width()),
            qMin(slideSize.height(), size().height()));
        if (slideSize != scaledSize)
            slide = slide.scaled(scaledSize, Qt::KeepAspectRatio);

        QRect pixmapRect(qMax( (size().width() - slide.width())/2, 0),
                         qMax( (size().height() - slide.height())/2, 0),
                         slide.width(),
                         slide.height());

        if (pixmapRect.top() > 0) {
            // Fill in top & bottom rectangles:
            painter.fillRect(0, 0, size().width(), pixmapRect.top(), Qt::black);
            painter.fillRect(0, pixmapRect.bottom(), size().width(), size().height(), Qt::black);
        }

        if (pixmapRect.left() > 0) {
            // Fill in left & right rectangles:
            painter.fillRect(0, 0, pixmapRect.left(), size().height(), Qt::black);
            painter.fillRect(pixmapRect.right(), 0, size().width(), size().height(), Qt::black);
        }

        painter.drawPixmap(pixmapRect, slide);

    } else
        painter.fillRect(event->rect(), Qt::black);
}


void SlideShow::keyPressEvent(QKeyEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}


void SlideShow::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}


void SlideShow::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}


void SlideShow::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit inputReceived();
}


void SlideShow::showEvent(QShowEvent * event )
{
    Q_UNUSED(event);
#ifndef QT_NO_CURSOR
    setCursor(Qt::BlankCursor);
#endif
}

