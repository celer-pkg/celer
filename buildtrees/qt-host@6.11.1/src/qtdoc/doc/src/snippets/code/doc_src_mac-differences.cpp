// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [1]
#ifdef Q_OS_MAC
    QString bundlePath = QString::fromNSString(NSBundle.mainBundle.bundlePath);
    qDebug() << "Bundle path =" << bundlePath;
#endif
//! [1]
