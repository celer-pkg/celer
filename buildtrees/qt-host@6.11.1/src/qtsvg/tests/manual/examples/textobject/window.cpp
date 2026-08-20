// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QtSvg>

#include "window.h"
#include "svgtextobject.h"

Window::Window()
{
    setupGui();
    setupTextObject();

    setWindowTitle("Text Object Example");
}

//![1]
void Window::insertTextObject()
{
    QString fileName = fileNameLineEdit->text();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error Opening File"),
                             tr("Could not open '%1'").arg(fileName));
    }

    QByteArray svgData = file.readAll();
//![1]

//![2]
    QTextCharFormat svgCharFormat;
    svgCharFormat.setObjectType(SvgTextFormat);
    QSvgRenderer renderer(svgData);

    QImage svgBufferImage(renderer.defaultSize(), QImage::Format_ARGB32);
    QPainter painter(&svgBufferImage);
    renderer.render(&painter, svgBufferImage.rect());

    svgCharFormat.setProperty(SvgData, svgBufferImage);

    QTextCursor cursor = textEdit->textCursor();
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), svgCharFormat);
    textEdit->setTextCursor(cursor);
}
//![2]

//![3]
void Window::setupTextObject()
{
    QObject *svgInterface = new SvgTextObject;
    svgInterface->setParent(this);
    textEdit->document()->documentLayout()->registerHandler(SvgTextFormat, svgInterface);
}
//![3]

void Window::setupGui()
{
    fileNameLabel = new QLabel(tr("Svg File Name:"));
    fileNameLineEdit = new QLineEdit;
    insertTextObjectButton = new QPushButton(tr("Insert Image"));

    fileNameLineEdit->setText(":/files/heart.svg");
    connect(insertTextObjectButton, &QPushButton::clicked,
            this, &Window::insertTextObject);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(fileNameLabel);
    bottomLayout->addWidget(fileNameLineEdit);
    bottomLayout->addWidget(insertTextObjectButton);

    textEdit = new QTextEdit;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(textEdit);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
}

