// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QPushButton>
#include "widgettest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    HelloWidget *widget = new HelloWidget;
    widget->resize(400, 400);

    QWidget *tlw = nullptr;

    QPushButton *btn = new QPushButton("Make it a child widget", widget);
    QObject::connect(btn, &QPushButton::clicked, widget, [&] {
        if (!tlw) {
            btn->setText("Make it a top-level");
            tlw = new QWidget;
            tlw->resize(600, 600);
            widget->setParent(tlw);
            widget->move(0, 0); // why is this needed?
            widget->show();
            tlw->show();
            widget->update();
        } else {
            btn->setText("Make it a child widget");
            widget->setParent(nullptr);
            widget->show();
            delete tlw;
            tlw = nullptr;
        }
    });

    widget->show();

    int r = app.exec();

    if (!widget->parent())
        delete widget;

    return r;
}
