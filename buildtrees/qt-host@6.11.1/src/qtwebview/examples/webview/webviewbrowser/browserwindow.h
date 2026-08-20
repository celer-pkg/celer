// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets/qmainwindow.h>
#include <QtWebView/qwebviewsettings.h>

QT_BEGIN_NAMESPACE
class QAction;
class QLineEdit;
class QProgressBar;
class QToolBar;
class QToolButton;
class QWebView;
class QWebViewLoadingInfo;
QT_END_NAMESPACE

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
public:
    BrowserWindow();
    virtual ~BrowserWindow() = default;

public Q_SLOTS:
    void onLoadingChanged(const QWebViewLoadingInfo &loadingInfo);
    void onTitleChanged(const QString &title);
    void onLoadProgressChanged(int loadProgress);
    void onSettingToggled(QAction *action, QWebViewSettings::WebAttribute attribute);
    void onUrlInput();

private:
    void addSettingToMenu(QAction *action, QWebViewSettings::WebAttribute setting);

    QWebView *webView;

    QToolBar *toolBar;
    QLineEdit *lineEdit;
    QProgressBar *progressBar;
    QToolButton *settingsMenuButton;

    QAction *backAction;
    QAction *forwardAction;
    QAction *reloadAction;
    QAction *stopAction;

    QAction *localStorageAction;
    QAction *javaScriptAction;
    QAction *allowFileAccessAction;
    QAction *localContentsCanAccessFileUrlsAction;
};
