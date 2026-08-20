// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QtSvg>
#include <QtSvgWidgets>
#include "mimedata.h"
#include "sourcewidget.h"

SourceWidget::SourceWidget(QWidget *parent)
    : QWidget(parent)
{
    QFile imageFile(":/images/example.svg");
    imageFile.open(QIODevice::ReadOnly);
    imageData = imageFile.readAll();
    imageFile.close();

    QScrollArea *imageArea = new QScrollArea;
    imageLabel = new QSvgWidget;
    imageLabel->renderer()->load(imageData);
    imageArea->setWidget(imageLabel);
    //imageLabel->setMinimumSize(imageLabel->renderer()->viewBox().size());

    QLabel *instructTopLabel = new QLabel(tr("This is an SVG drawing:"));
    QLabel *instructBottomLabel = new QLabel(
        tr("Drag the icon to copy the drawing as a PNG file:"));
    instructBottomLabel->setWordWrap(true);
    QPushButton *dragIcon = new QPushButton(tr("Export"));
    dragIcon->setIcon(QIcon(":/images/drag.png"));

    connect(dragIcon, &QPushButton::pressed, this, &SourceWidget::startDrag);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(instructTopLabel, 0, 0, 1, 2);
    layout->addWidget(imageArea, 1, 0, 2, 2);
    layout->addWidget(instructBottomLabel, 3, 0);
    layout->addWidget(dragIcon, 3, 1);
    setLayout(layout);
    setWindowTitle(tr("Delayed Encoding"));
}

//![1]
void SourceWidget::createData(const QString &mimeType)
{
    if (mimeType != "image/png")
        return;

    QImage image(imageLabel->size(), QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&image);
    imageLabel->renderer()->render(&painter);
    painter.end();

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();

    mimeData->setData("image/png", data);
}
//![1]

//![0]
void SourceWidget::startDrag()
{
    mimeData = new MimeData;

    connect(mimeData, &MimeData::dataRequested,
            this, &SourceWidget::createData, Qt::DirectConnection);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/images/drag.png"));

    drag->exec(Qt::CopyAction);
}
//![0]

