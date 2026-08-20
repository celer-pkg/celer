// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#undef QT_NO_FOREACH // this file contains unported legacy Q_FOREACH uses

#include <QWebElement>
#include <QWebFrame>
#include "window.h"

//! [Window class constructor]
Window::Window(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}
//! [Window class constructor]

//! [return pressed]
void Window::on_elementLineEdit_returnPressed()
{
    QWebFrame *frame = webView->page()->mainFrame();

//! [select elements]
    QWebElement document = frame->documentElement();
    QWebElementCollection elements = document.findAll(elementLineEdit->text());
//! [select elements]

    foreach (QWebElement element, elements)
        element.setAttribute("style", "background-color: #f0f090");
}
//! [return pressed]

//! [button clicked]
void Window::on_highlightButton_clicked()
{
    on_elementLineEdit_returnPressed();
}
//! [button clicked]

//! [set URL]
void Window::setUrl(const QUrl &url)
{
    webView->setUrl(url);
}
//! [set URL]
