// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
QAxSelect select;
if (select.exec()) {
    QAxWidget *container = new QAxWidget;
    container->setControl(select.clsid());
    container->show();
}
//! [0]
