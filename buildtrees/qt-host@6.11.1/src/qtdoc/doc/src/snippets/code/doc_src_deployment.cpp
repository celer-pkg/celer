// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [9]
QCoreApplication::addLibraryPath("/some/other/path");
//! [9]


//! [19]
QCoreApplication::addLibraryPath("C:/some/other/path");
//! [19]


//! [49]
QDir dir(QCoreApplication::applicationDirPath());
dir.cdUp();
dir.cd("plugins");
QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
//! [49]
