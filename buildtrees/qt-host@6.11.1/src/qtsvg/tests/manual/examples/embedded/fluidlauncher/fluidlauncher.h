// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FLUID_LAUNCHER_H
#define FLUID_LAUNCHER_H

#include <QtWidgets>
#include <QTimer>

#include "pictureflow.h"
#include "slideshow.h"
#include "demoapplication.h"

class FluidLauncher : public QStackedWidget
{
    Q_OBJECT

public:
    FluidLauncher(QStringList* args);
    ~FluidLauncher();

public slots:
    void launchApplication(int index);
    void switchToLauncher();
    void resetInputTimeout();
    void inputTimedout();
    void demoFinished();

protected:
    void changeEvent(QEvent *event) override;

private:
    PictureFlow* pictureFlowWidget;
    SlideShow* slideShowWidget;
    QTimer* inputTimer;
    QList<DemoApplication*> demoList;

    bool loadConfig(QString configPath);
    void populatePictureFlow();
    void switchToSlideshow();
    void parseDemos(QXmlStreamReader& reader);
    void parseSlideshow(QXmlStreamReader& reader);

};


#endif
