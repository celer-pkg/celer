// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "testwindow.h"

//! [0]
#include <QtGui/QGuiApplication>
//! [0]
#include <QDebug>

#if defined(Q_OS_UNIX)
#if QT_VERSION < 0x060000
#include <QtX11Extras/QX11Info>
#endif
#endif

#include <X11/Xlib.h>
#include <xcb/xcb.h>

TestWindow::TestWindow(QWindow *parent)
    : QWindow(parent)
{

#if QT_VERSION < 0x060000
    Display *display = QX11Info::display();
    xcb_connection_t *connection = QX11Info::connection();
    bool isPlatformX11 = QX11Info::isPlatformX11();
#else
//! [1]
    Display *display = nullptr;
    xcb_connection_t *connection = nullptr;
    bool isPlatformX11 = false;
    if (auto *x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()) {
        display = x11Application->display();
        connection = x11Application->connection();
        isPlatformX11 = true;
    }
    // or
    // isPlatformX11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
//! [1]
#endif

    qDebug() << "Display *display=" << display;
    if (display)
        qDebug() << "XConnectionNumber=" << XConnectionNumber(display);

    qDebug() << "xcb_connection_t *xcbConnection=" << connection;
    if (connection) {
        const xcb_setup_t *xcbSetup = xcb_get_setup(connection);
        if (xcbSetup)
            qDebug() << "protocol_major_version=" << xcbSetup->protocol_major_version << ", protocol_minor_version=" << xcbSetup->protocol_minor_version;
    }

    qDebug() << "isPlatformX11=" << isPlatformX11;
}
