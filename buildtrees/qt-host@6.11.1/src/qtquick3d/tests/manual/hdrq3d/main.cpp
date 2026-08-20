// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QQuickView>
#include <QtQuick3D/qquick3d.h>

#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QDialog>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    qputenv("QSG_INFO", "1");

    QDialog sel;
    QVBoxLayout *selLayout = new QVBoxLayout;
    selLayout->addWidget(new QLabel("Select SDR/HDR mode"));
    QListWidget *modeList = new QListWidget;
    modeList->addItem("SDR");
    modeList->addItem("HDR: scRGB (extended linear sRGB + FP16 color buffer)");
    modeList->addItem("HDR: HDR10 Rec.2020 ST2084 + RGB10A2 color buffer\n(will appear incorrect, just for demo)");
    modeList->setCurrentRow(0);
    selLayout->addWidget(modeList);
    QPushButton *okBtn = new QPushButton("Ok");
    okBtn->setDefault(true);
    selLayout->addWidget(okBtn);
    sel.setLayout(selLayout);
    sel.resize(340, 160);
    sel.show();

    std::unique_ptr<QQuickView> view;
    QObject::connect(okBtn, &QPushButton::clicked, [modeList, &sel, &view] {
        switch (modeList->currentRow()) {
        case 0:
            break;
        case 1:
            qputenv("QSG_RHI_HDR", "scrgb");
            break;
        case 2:
            qputenv("QSG_RHI_HDR", "hdr10");
            break;
        default:
            break;
        }

        view.reset(new QQuickView);
        view->setColor(Qt::black);
        view->setResizeMode(QQuickView::SizeRootObjectToView);
        view->resize(1280, 720);
        view->setSource(QUrl("qrc:/main.qml"));
        view->show();

        sel.close();
    });

    int r = app.exec();

    return r;
}
