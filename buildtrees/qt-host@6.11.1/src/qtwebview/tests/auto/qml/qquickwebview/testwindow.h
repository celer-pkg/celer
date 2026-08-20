// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#if 0
#pragma qt_no_master_include
#endif

#include <QResizeEvent>
#include <QScopedPointer>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

// TestWindow: Utility class to ignore QQuickView details.
class TestWindow : public QQuickView {
public:
    inline TestWindow(QQuickItem *webView);
    QScopedPointer<QQuickItem> webView;

protected:
    inline void resizeEvent(QResizeEvent*) override;
};

inline TestWindow::TestWindow(QQuickItem *webView)
    : webView(webView)
{
    Q_ASSERT(webView);
    webView->setParentItem(contentItem());
    resize(300, 400);
}

inline void TestWindow::resizeEvent(QResizeEvent *event)
{
    QQuickView::resizeEvent(event);
    webView->setX(0);
    webView->setY(0);
    webView->setWidth(event->size().width());
    webView->setHeight(event->size().height());
}

#endif /* TESTWINDOW_H */
