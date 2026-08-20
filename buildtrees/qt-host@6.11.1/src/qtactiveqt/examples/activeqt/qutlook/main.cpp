// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include "addressview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AddressView view;
    view.setWindowTitle(QObject::tr("Qt Example - Looking at Outlook"));
    view.show();

    return a.exec();
}
//! [0]
