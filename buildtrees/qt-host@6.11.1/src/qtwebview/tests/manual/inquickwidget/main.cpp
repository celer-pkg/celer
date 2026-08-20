// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtWidgets/qapplication.h>
#include <QtWidgets/QHBoxLayout>
#include <QtQuickWidgets/qquickwidget.h>

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::OpenGL);
    QApplication a(argc, argv);
    QWidget w;
    w.setGeometry(0, 0, 800, 600);
    w.setLayout(new QHBoxLayout);
    QQuickWidget *qw = new QQuickWidget;
    qw->setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);
    qw->setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    w.layout()->addWidget(qw);
    w.show();
    return a.exec();
}
