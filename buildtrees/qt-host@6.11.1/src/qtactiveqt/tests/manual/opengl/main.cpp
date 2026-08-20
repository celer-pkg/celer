// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
//
// Qt OpenGL example: Box
//
// A small example showing how a GLWidget can be used just as any Qt widget
//
// File: main.cpp
//
// The main() function
//

#include "globjwin.h"
#include "glbox.h"
#include <QApplication>
//! [0]
#include <QAxFactory>

QAXFACTORY_BEGIN(
    "{2c3c183a-eeda-41a4-896e-3d9c12c3577d}", // type library ID
    "{83e16271-6480-45d5-aaf1-3f40b7661ae4}") // application ID
    QAXCLASS(GLBox)
QAXFACTORY_END()

//! [0] //! [1]
/*
  The main program is here.
*/

int main(int argc, char *argv[])
{
    QApplication a(argc,argv);

    if (QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGL) {
        qWarning("This system does not support OpenGL. Exiting.");
        return -1;
    }

    if (!QAxFactory::isServer()) {
        GLObjectWindow w;
        w.resize(400, 350);
        w.show();
        return a.exec();
//! [1] //! [2]
    }
    return a.exec();
//! [2] //! [3]
}
//! [3]
