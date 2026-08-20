// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTimeLine>

struct MyObject : public QWidget
{
    void examples()
    {
        //! [0]
        //...
        auto progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);

        // Construct a 1-second timeline with a frame range of 0 - 100
        QTimeLine *timeLine = new QTimeLine(1000, this);
        timeLine->setFrameRange(0, 100);
        connect(timeLine, &QTimeLine::frameChanged, progressBar, &QProgressBar::setValue);

        // Clicking the push button will start the progress bar animation
        auto pushButton = new QPushButton(tr("Start animation"), this);
        connect(pushButton, &QPushButton::clicked, timeLine, &QTimeLine::start);
        //...
        //! [0]
    }
};
