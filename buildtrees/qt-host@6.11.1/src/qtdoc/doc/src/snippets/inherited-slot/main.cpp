// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMetaMethod>
#include <QWidget>
#include "button.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    QHBoxLayout *layout = new QHBoxLayout(&window);
    QLineEdit *lineEdit = new QLineEdit;
    Button *button = new Button;

    QObject::connect(lineEdit, SIGNAL(returnPressed()), button, SLOT(animateClick()));

    layout->addWidget(lineEdit);
    layout->addWidget(button);
    window.show();

    for (int i = 0; i < button->metaObject()->methodCount(); ++i)
        qDebug() << i << button->metaObject()->method(i).signature();
    return app.exec();
}
