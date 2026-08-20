// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QScreen>
#include <QXmlStreamReader>

#include "fluidlauncher.h"

#define DEFAULT_INPUT_TIMEOUT 10000
#define SIZING_FACTOR_HEIGHT 6/10
#define SIZING_FACTOR_WIDTH 6/10

FluidLauncher::FluidLauncher(QStringList* args)
{
    pictureFlowWidget = new PictureFlow();
    slideShowWidget = new SlideShow();
    inputTimer = new QTimer();

    addWidget(pictureFlowWidget);
    addWidget(slideShowWidget);
    setCurrentWidget(pictureFlowWidget);
    pictureFlowWidget->setFocus();

    QRect screen_size = QGuiApplication::primaryScreen()->geometry();

    connect(pictureFlowWidget, &PictureFlow::itemActivated, this, &FluidLauncher::launchApplication);
    connect(pictureFlowWidget, &PictureFlow::inputReceived, this, &FluidLauncher::resetInputTimeout);
    connect(slideShowWidget,   &SlideShow::inputReceived,   this, &FluidLauncher::switchToLauncher);
    connect(inputTimer,        &QTimer::timeout,            this, &FluidLauncher::inputTimedout);

    inputTimer->setSingleShot(true);
    inputTimer->setInterval(DEFAULT_INPUT_TIMEOUT);

    const int h = screen_size.height() * SIZING_FACTOR_HEIGHT;
    const int w = screen_size.width() * SIZING_FACTOR_WIDTH;
    const int hh = qMin(h, w);
    const int ww = hh / 3 * 2;
    pictureFlowWidget->setSlideSize(QSize(ww, hh));

    bool success;
    int configIndex = args->indexOf("-config");
    if ( (configIndex != -1) && (configIndex != args->count()-1) )
        success = loadConfig(args->at(configIndex+1));
    else
        success = loadConfig(":/fluidlauncher/config.xml");

    if (success) {
      populatePictureFlow();

      showFullScreen();
      inputTimer->start();
    } else {
        pictureFlowWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        pictureFlowWidget->close();
    }

}

FluidLauncher::~FluidLauncher()
{
    delete pictureFlowWidget;
    delete slideShowWidget;
}

bool FluidLauncher::loadConfig(QString configPath)
{
    QFile xmlFile(configPath);

    if (!xmlFile.exists() || (xmlFile.error() != QFile::NoError)) {
        qDebug() << "ERROR: Unable to open config file " << configPath;
        return false;
    }

    slideShowWidget->clearImages();

    xmlFile.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&xmlFile);
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            if (reader.name() == u"demos")
                parseDemos(reader);
            else if(reader.name() == u"slideshow")
                parseSlideshow(reader);
        }
    }

    if (reader.hasError()) {
       qDebug() << QString("Error parsing %1 on line %2 column %3: \n%4")
                .arg(configPath)
                .arg(reader.lineNumber())
                .arg(reader.columnNumber())
                .arg(reader.errorString());
    }

    // Append an exit Item
    DemoApplication* exitItem = new DemoApplication(QString(), QLatin1String("Exit Embedded Demo"), QString(), QStringList());
    demoList.append(exitItem);

    return true;
}


void FluidLauncher::parseDemos(QXmlStreamReader& reader)
{
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == u"example") {
            QXmlStreamAttributes attrs = reader.attributes();
            QStringView filename = attrs.value("filename");
            if (!filename.isEmpty()) {
                QStringView name = attrs.value("name");
                QStringView image = attrs.value("image");
                QStringView args = attrs.value("args");

                DemoApplication* newDemo = new DemoApplication(
                        filename.toString(),
                        name.isEmpty() ? "Unnamed Demo" : name.toString(),
                        image.toString(),
                        args.toString().split(" "));
                demoList.append(newDemo);
            }
        } else if(reader.isEndElement() && reader.name() == u"demos") {
            return;
        }
    }
}

void FluidLauncher::parseSlideshow(QXmlStreamReader& reader)
{
    QXmlStreamAttributes attrs = reader.attributes();

    QStringView timeout = attrs.value("timeout");
    bool valid;
    if (!timeout.isEmpty()) {
        int t = timeout.toString().toInt(&valid);
        if (valid)
            inputTimer->setInterval(t);
    }

    QStringView interval = attrs.value("interval");
    if (!interval.isEmpty()) {
        int i = interval.toString().toInt(&valid);
        if (valid)
            slideShowWidget->setSlideInterval(i);
    }

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            QXmlStreamAttributes attrs = reader.attributes();
            if (reader.name() == u"imagedir") {
                QStringView dir = attrs.value("dir");
                slideShowWidget->addImageDir(dir.toString());
            } else if(reader.name() == u"image") {
                QStringView image = attrs.value("image");
                slideShowWidget->addImage(image.toString());
            }
        } else if(reader.isEndElement() && reader.name() == u"slideshow") {
            return;
        }
    }

}

void FluidLauncher::populatePictureFlow()
{
    pictureFlowWidget->setSlideCount(demoList.count());

    for (int i=demoList.count()-1; i>=0; --i) {
        const QImage image = demoList[i]->getImage();
        if (!image.isNull())
            pictureFlowWidget->setSlide(i, image);
        pictureFlowWidget->setSlideCaption(i, demoList[i]->getCaption());
    }

    pictureFlowWidget->setCurrentSlide(demoList.count()/2);
}


void FluidLauncher::launchApplication(int index)
{
    // NOTE: Clearing the caches will free up more memory for the demo but will cause
    // a delay upon returning, as items are reloaded.
    //pictureFlowWidget->clearCaches();

    if (index == demoList.size() -1) {
        qApp->quit();
        return;
    }

    inputTimer->stop();

    connect(demoList[index], &DemoApplication::demoFinished, this, &FluidLauncher::demoFinished);

    demoList[index]->launch();
}


void FluidLauncher::switchToLauncher()
{
    slideShowWidget->stopShow();
    inputTimer->start();
    setCurrentWidget(pictureFlowWidget);
}


void FluidLauncher::resetInputTimeout()
{
    if (inputTimer->isActive())
        inputTimer->start();
}

void FluidLauncher::inputTimedout()
{
    switchToSlideshow();
}


void FluidLauncher::switchToSlideshow()
{
    inputTimer->stop();
    slideShowWidget->startShow();
    setCurrentWidget(slideShowWidget);
}

void FluidLauncher::demoFinished()
{
    setCurrentWidget(pictureFlowWidget);
    inputTimer->start();

    // Bring the Fluidlauncher to the foreground to allow selecting another demo
    raise();
    activateWindow();
}

void FluidLauncher::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            if(currentWidget() == pictureFlowWidget) {
                resetInputTimeout();
            } else {
                slideShowWidget->startShow();
            }
        } else {
            inputTimer->stop();
            slideShowWidget->stopShow();
        }
    }
    QStackedWidget::changeEvent(event);
}
