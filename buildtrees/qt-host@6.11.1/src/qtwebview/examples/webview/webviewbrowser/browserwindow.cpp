// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "browserwindow.h"

#include <QtCore/qurl.h>
#include <QtGui/qaction.h>
#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qprogressbar.h>

using namespace Qt::StringLiterals;

BrowserWindow::BrowserWindow()
    : webView(new QWebView())
    , toolBar(addToolBar("Navigation"))
    , lineEdit(new QLineEdit(this))
    , progressBar(new QProgressBar(this))
    , settingsMenuButton(new QToolButton(toolBar))
    , backAction(new QAction(QIcon(":left-32.png"_L1), "Go back"_L1, this))
    , forwardAction(new QAction(QIcon(":right-32.png"_L1), "Go forward"_L1, this))
    , reloadAction(new QAction(QIcon(":refresh-32.png"_L1), "Reload"_L1, this))
    , stopAction(new QAction(QIcon(":stop-32.png"_L1), "Stop"_L1, this))
    , localStorageAction(new QAction("Enable Local storage"_L1, settingsMenuButton))
    , javaScriptAction(new QAction("Enable JavaScript"_L1, settingsMenuButton))
    , allowFileAccessAction(new QAction("Allow file access"_L1, settingsMenuButton))
    , localContentsCanAccessFileUrlsAction(new QAction("Enable file URLs for local documents"_L1, settingsMenuButton))
{
    // Set up central widget. We use a QVBoxLayout to put the progress bar above the web content
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(centralWidget);

    // Wrap the QWebView in a QWidget
    QWidget *webViewContainer = QWidget::createWindowContainer(webView);
    webViewContainer->setMinimumSize(QSize(1280, 720));
    layout->addWidget(webViewContainer);

    // Set up toolbar
    toolBar->setMovable(false);
    toolBar->addAction(backAction);
    toolBar->addAction(forwardAction);
    toolBar->addAction(reloadAction);
    toolBar->addAction(stopAction);
    toolBar->addSeparator();

    lineEdit->setReadOnly(false);
    toolBar->addWidget(lineEdit);

    stopAction->setVisible(false);

    // Set up shortcuts
    backAction->setShortcut(QKeySequence::Back);
    forwardAction->setShortcut(QKeySequence::Forward);
    reloadAction->setShortcut(QKeySequence::Refresh);
    stopAction->setShortcut(QKeySequence::Cancel);

    // Set up progress bar
    progressBar->setMaximumHeight(2);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet("QProgressBar {border: 0px} QProgressBar::chunk {background-color: #da4453}"_L1);
    layout->insertWidget(0, progressBar);

    // Set up settings menu
    toolBar->addSeparator();
    settingsMenuButton->setIcon(QIcon(":settings-32.png"_L1));
    settingsMenuButton->setToolTip("Settings"_L1);
    settingsMenuButton->setPopupMode(QToolButton::InstantPopup);
    settingsMenuButton->setStyleSheet("QToolButton::menu-indicator {image: none;}"_L1);
    addSettingToMenu(localStorageAction, QWebViewSettings::WebAttribute::LocalStorageEnabled);
    addSettingToMenu(javaScriptAction, QWebViewSettings::WebAttribute::JavaScriptEnabled);
    addSettingToMenu(allowFileAccessAction, QWebViewSettings::WebAttribute::AllowFileAccess);
    addSettingToMenu(localContentsCanAccessFileUrlsAction, QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls);
    toolBar->addWidget(settingsMenuButton);

    // Set up signal/slot connections
    connect(backAction, &QAction::triggered, webView, &QWebView::goBack);
    connect(forwardAction, &QAction::triggered, webView, &QWebView::goForward);
    connect(reloadAction, &QAction::triggered, webView, &QWebView::reload);
    connect(stopAction, &QAction::triggered, webView, &QWebView::stop);

    connect(webView, &QWebView::loadingChanged, this, &BrowserWindow::onLoadingChanged);
    connect(webView, &QWebView::loadProgressChanged, this, &BrowserWindow::onLoadProgressChanged);
    connect(webView, &QWebView::titleChanged, this, &BrowserWindow::onTitleChanged);
    connect(lineEdit, &QLineEdit::returnPressed, this, &BrowserWindow::onUrlInput);

    // Load the startup URL
    webView->setUrl(QUrl("https://qt.io"_L1));
}

void BrowserWindow::onLoadingChanged(const QWebViewLoadingInfo &loadingInfo)
{
    switch (loadingInfo.status()) {
    case QWebViewLoadingInfo::LoadStatus::Started:
        reloadAction->setEnabled(false);
        reloadAction->setVisible(false);
        stopAction->setEnabled(true);
        stopAction->setVisible(true);
        break;
    case QWebViewLoadingInfo::LoadStatus::Stopped:
    case QWebViewLoadingInfo::LoadStatus::Succeeded:
    case QWebViewLoadingInfo::LoadStatus::Failed:
        reloadAction->setEnabled(true);
        reloadAction->setVisible(true);
        stopAction->setEnabled(false);
        stopAction->setVisible(false);
        break;
    }

    if (webView->canGoBack()) {
        backAction->setEnabled(true);
    } else {
        backAction->setEnabled(false);
    }

    if (webView->canGoForward()) {
        forwardAction->setEnabled(true);
    } else {
        forwardAction->setEnabled(false);
    }

    lineEdit->setText(loadingInfo.url().toString());
}

void BrowserWindow::onTitleChanged(const QString &title)
{
    setWindowTitle(title);
}

void BrowserWindow::onLoadProgressChanged(int loadProgress)
{
    if (0 < loadProgress && loadProgress < 100) {
        progressBar->setValue(loadProgress);
    } else {
        progressBar->setValue(0);
    }
}

void BrowserWindow::onUrlInput()
{
    webView->setUrl(QUrl::fromUserInput(lineEdit->text()));
}

void BrowserWindow::onSettingToggled(QAction *action, QWebViewSettings::WebAttribute attribute)
{
    webView->settings()->setAttribute(attribute, action->isChecked());
    action->setChecked(webView->settings()->testAttribute(attribute));
}

void BrowserWindow::addSettingToMenu(QAction *action, QWebViewSettings::WebAttribute setting)
{
    action->setCheckable(true);
    action->setChecked(webView->settings()->testAttribute(setting));
    connect(action, &QAction::toggled, [this, action, setting](bool){
        onSettingToggled(action, setting);
    });
    settingsMenuButton->addAction(action);
}
