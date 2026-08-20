// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testwindow.h"
#include "util.h"

#include <QScopedPointer>
#include <QtQml/QQmlEngine>
#include <QtTest/QtTest>
#include <QtWebViewQuick/private/qquickwebview_p.h>
#include <QtCore/qfile.h>
#include <QtCore/qstandardpaths.h>
#include <QtWebView/qtwebviewfunctions.h>
#include <QtWebView/private/qwebviewfactory_p.h>
#include <QtWebViewQuick/private/qquickwebviewsettings_p.h>

QUrl getTestFilePath(const QString &testFile)
{
    const QString tempTestFile = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + testFile;
    const bool exists = QFile::exists(tempTestFile);
    if (exists)
        return QUrl::fromLocalFile(tempTestFile);

    QFile tf(QString(":/") + testFile);
    const bool copied = tf.copy(tempTestFile);

    return QUrl::fromLocalFile(copied ? tempTestFile : testFile);
}

class tst_QQuickWebView : public QObject {
    Q_OBJECT
public:
    tst_QQuickWebView();

private Q_SLOTS:
    void initTestCase();

    void init();
    void cleanup();

    void navigationStatusAtStartup();
    void stopEnabledAfterLoadStarted();
    void baseUrl();
    void loadEmptyUrl();
    void loadEmptyPageViewVisible();
    void loadEmptyPageViewHidden();
    void loadNonexistentFileUrl();
    void backAndForward();
    void reload();
    void stop();
    void loadProgress();

    void show();
    void showWebView();
    void removeFromCanvas();
    void multipleWebViewWindows();
    void multipleWebViews();
    void titleUpdate();
    void changeUserAgent();
    void setAndDeleteCookies();

    void settings_JS_data();
    void settings_JS();

private:
    inline QQuickWebView *newWebView();
    inline QQuickWebView *webView() const;
    void runJavaScript(const QString &script);
    QScopedPointer<TestWindow> m_window;
    QScopedPointer<QQmlComponent> m_component;
    QQmlEngine *engine = nullptr;
};

tst_QQuickWebView::tst_QQuickWebView()
{
    engine = new QQmlEngine(this);
    m_component.reset(new QQmlComponent(engine, this));
    m_component->setData(QByteArrayLiteral("import QtQuick 2.0\n"
                                           "import QtWebView 1.1\n"
                                           "WebView {}")
                         , QUrl());
}

QQuickWebView *tst_QQuickWebView::newWebView()
{
    QObject *viewInstance = m_component->create();
    QQuickWebView *webView = qobject_cast<QQuickWebView*>(viewInstance);
    webView->settings()->setAllowFileAccess(true);
    webView->settings()->setLocalContentCanAccessFileUrls(true);
    return webView;
}

void tst_QQuickWebView::initTestCase()
{
    if (!qEnvironmentVariableIsEmpty("QEMU_LD_PREFIX"))
        QSKIP("This test is unstable on QEMU, so it will be skipped.");
}

void tst_QQuickWebView::init()
{
    QtWebView::initialize();
    m_window.reset(new TestWindow(newWebView()));
}

void tst_QQuickWebView::cleanup()
{
    m_window.reset();
}

inline QQuickWebView *tst_QQuickWebView::webView() const
{
    return static_cast<QQuickWebView*>(m_window->webView.data());
}

void tst_QQuickWebView::runJavaScript(const QString &script)
{
    webView()->runJavaScript(script);
}

void tst_QQuickWebView::navigationStatusAtStartup()
{
    QCOMPARE(webView()->canGoBack(), false);

    QCOMPARE(webView()->canGoForward(), false);

    QCOMPARE(webView()->isLoading(), false);
}

void tst_QQuickWebView::stopEnabledAfterLoadStarted()
{
    QCOMPARE(webView()->isLoading(), false);

    LoadStartedCatcher loadStartedCatcher(webView());
    LoadSpy loadSpy(webView());
    webView()->setUrl(getTestFilePath("basic_page.html"));

    waitForSignal(&loadStartedCatcher, SIGNAL(finished()));
    waitForSignal(&loadSpy, SIGNAL(loadSucceeded()));
}

void tst_QQuickWebView::baseUrl()
{
    // Test the url is in a well defined state when instanciating the view, but before loading anything.
    QVERIFY(webView()->url().isEmpty());
}

void tst_QQuickWebView::loadEmptyUrl()
{
    webView()->setUrl(QUrl());
    webView()->setUrl(QUrl(QLatin1String("")));
}

void tst_QQuickWebView::loadEmptyPageViewVisible()
{
    m_window->show();
    loadEmptyPageViewHidden();
}

void tst_QQuickWebView::loadEmptyPageViewHidden()
{
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QQuickWebViewLoadRequest*)));

    webView()->setUrl(getTestFilePath("basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(loadSpy.size(), 2);
}

void tst_QQuickWebView::loadNonexistentFileUrl()
{
    QSignalSpy loadSpy(webView(), SIGNAL(loadingChanged(QQuickWebViewLoadRequest*)));

    webView()->setUrl(getTestFilePath("file_that_does_not_exist.html"));
    QVERIFY(waitForLoadFailed(webView()));

    QCOMPARE(loadSpy.size(), 2);
}

void tst_QQuickWebView::backAndForward()
{
    const QUrl basicPage = getTestFilePath("basic_page.html");
    const QUrl basicPage2 = getTestFilePath("basic_page2.html");
    webView()->setUrl(basicPage);
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), basicPage);

    webView()->setUrl(basicPage2);
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), basicPage2);

    webView()->goBack();
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), basicPage);

    webView()->goForward();
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), basicPage2);
}

void tst_QQuickWebView::reload()
{
    webView()->setUrl(getTestFilePath("basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), getTestFilePath("basic_page.html"));

    webView()->reload();
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), getTestFilePath("basic_page.html"));
}

void tst_QQuickWebView::stop()
{
    webView()->setUrl(QUrl(getTestFilePath("basic_page.html")));
    QVERIFY(waitForLoadSucceeded(webView()));

    QCOMPARE(webView()->url(), getTestFilePath("basic_page.html"));

    webView()->stop();
}

void tst_QQuickWebView::loadProgress()
{
    QCOMPARE(webView()->loadProgress(), 0);

    webView()->setUrl(getTestFilePath("basic_page.html"));
    QSignalSpy loadProgressChangedSpy(webView(), SIGNAL(loadProgressChanged()));
    QVERIFY(waitForLoadSucceeded(webView()));

    QVERIFY(loadProgressChangedSpy.size() >= 1);

    QCOMPARE(webView()->loadProgress(), 100);
}

void tst_QQuickWebView::show()
{
    // This should not crash.
    m_window->show();
    QTest::qWait(200);
    m_window->hide();
}

void tst_QQuickWebView::showWebView()
{
    webView()->setUrl(getTestFilePath("direct-image-compositing.html"));
    QVERIFY(waitForLoadSucceeded(webView()));
    m_window->show();
    // This should not crash.
    webView()->setVisible(true);
    QTest::qWait(200);
    webView()->setVisible(false);
    QTest::qWait(200);
}

void tst_QQuickWebView::removeFromCanvas()
{
    showWebView();

    // This should not crash.
    QQuickItem *parent = webView()->parentItem();
    QQuickItem noCanvasItem;
    webView()->setParentItem(&noCanvasItem);
    QTest::qWait(200);
    webView()->setParentItem(parent);
    webView()->setVisible(true);
    QTest::qWait(200);
}

void tst_QQuickWebView::multipleWebViewWindows()
{
    showWebView();

    // This should not crash.
    QQuickWebView *webView1 = newWebView();
    QScopedPointer<TestWindow> window1(new TestWindow(webView1));
    QQuickWebView *webView2 = newWebView();
    QScopedPointer<TestWindow> window2(new TestWindow(webView2));

    webView1->setUrl(getTestFilePath("scroll.html"));
    QVERIFY(waitForLoadSucceeded(webView1));
    window1->show();
    webView1->setVisible(true);

    webView2->setUrl(getTestFilePath("basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webView2));
    window2->show();
    webView2->setVisible(true);
    QTest::qWait(200);
}

void tst_QQuickWebView::multipleWebViews()
{
    showWebView();

    // This should not crash.
    QScopedPointer<QQuickWebView> webView1(newWebView());
    webView1->setParentItem(m_window->contentItem());
    QScopedPointer<QQuickWebView> webView2(newWebView());
    webView2->setParentItem(m_window->contentItem());

    webView1->setSize(QSizeF(300, 400));
    webView1->setUrl(getTestFilePath("scroll.html"));
    QVERIFY(waitForLoadSucceeded(webView1.data()));
    webView1->setVisible(true);

    webView2->setSize(QSizeF(300, 400));
    webView2->setUrl(getTestFilePath("basic_page.html"));
    QVERIFY(waitForLoadSucceeded(webView2.data()));
    webView2->setVisible(true);
    QTest::qWait(200);
}

void tst_QQuickWebView::titleUpdate()
{
    QSignalSpy titleSpy(webView(), SIGNAL(titleChanged()));

    // Load page with no title
    webView()->setUrl(getTestFilePath("basic_page2.html"));
    QVERIFY(waitForLoadSucceeded(webView()));
    if (QWebViewFactory::loadedPluginHasKey("webengine")
        || QWebViewFactory::loadedPluginHasKey("android_view")
        || QWebViewFactory::loadedPluginHasKey("webview2")) {
        // on some platforms if the page has no <title> element, then the URL is used instead
        QCOMPARE(titleSpy.size(), 1);
    } else {
        QCOMPARE(titleSpy.size(), 0);
    }
    titleSpy.clear();

    // No titleChanged signal for failed load
    webView()->setUrl(getTestFilePath("file_that_does_not_exist.html"));
    QVERIFY(waitForLoadFailed(webView()));
    if (QWebViewFactory::loadedPluginHasKey("android_view")
        || QWebViewFactory::loadedPluginHasKey("webview2")) {
        QTRY_COMPARE(titleSpy.size(), 1);
    } else {
        QCOMPARE(titleSpy.size(), 0);
    }
}

void tst_QQuickWebView::changeUserAgent()
{
    QQmlComponent userAgentWebView(engine, this);
    userAgentWebView.setData(QByteArrayLiteral("import QtQuick 2.0\n"
                                               "import QtWebView 1.14\n"
                                               "WebView {\n"
                                               "httpUserAgent: \"dummy\"\n"
                                               "}"),
                             QUrl());
    QObject *viewInstance = userAgentWebView.create();
    QQuickWebView *webView = qobject_cast<QQuickWebView *>(viewInstance);
    QCOMPARE(webView->httpUserAgent(), "dummy");
}

void tst_QQuickWebView::setAndDeleteCookies()
{
    QSignalSpy cookieAddedSpy(webView(), SIGNAL(cookieAdded(QString,QString)));
    QSignalSpy cookieRemovedSpy(webView(), SIGNAL(cookieRemoved(QString,QString)));

    if (QWebViewFactory::loadedPluginHasKey("webengine")) {
        webView()->setUrl(QUrl("qrc:///cookies.html"));
        QVERIFY(waitForLoadSucceeded(webView()));

        QTRY_COMPARE(cookieAddedSpy.size(), 2);

        webView()->deleteAllCookies();
        QTRY_COMPARE(cookieRemovedSpy.size(), 2);

        cookieAddedSpy.clear();
        cookieRemovedSpy.clear();
    }

    Cookie::List cookies { {".example.com", "TestCookie", "testValue"},
                           {".example2.com", "TestCookie2", "testValue2"},
                           {".example3.com", "TestCookie3", "testValue3"} };

    for (const auto &cookie : cookies)
        webView()->setCookie(cookie.domain, cookie.name, cookie.value);

    QTRY_COMPARE(cookieAddedSpy.size(), cookies.size());
    QVERIFY(Cookie::testSignalValues(cookies, cookieAddedSpy));

    auto removedCookie = cookies.takeLast();

    webView()->deleteCookie(removedCookie.domain, removedCookie.name);
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);
    {
        const auto &first = cookieRemovedSpy.first();
        Cookie::SigArg sigArg{ first.at(0).toString(), first.at(1).toString() };
        QCOMPARE(removedCookie, sigArg);
    }

    // deleting a cookie using a name that has not been set
    webView()->deleteCookie(".example.com", "NewCookieName");
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);

    // deleting a cookie using a domain that has not been set
    webView()->deleteCookie(".new.domain.com", "TestCookie2");
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);

    webView()->deleteAllCookies();
    if (QWebViewFactory::loadedPluginHasKey("android_view"))
        QEXPECT_FAIL("", "Notification for deleteAllCookies() is not implemented on Android, yet!", Continue);
    QTRY_COMPARE(cookieRemovedSpy.size(), 3);
}

void tst_QQuickWebView::settings_JS_data()
{
    QTest::addColumn<bool>("jsEnabled");
    QTest::addColumn<QUrl>("testUrl");
    QTest::addColumn<QString>("expectedTitle");

    // Title should be updated from "JavaScript" to "JavaScript Test"
    QTest::newRow("Test JS enabled") << true << getTestFilePath("javascript.html") << "JavaScript Test";
    // Title should _not_ be updated from "JavaScript" to "JavaScript Test"
    QTest::newRow("Test JS disabled") << false << getTestFilePath("javascript.html") << "JavaScript";
}

void tst_QQuickWebView::settings_JS()
{

    QFETCH(bool, jsEnabled);
    QFETCH(QUrl, testUrl);
    QFETCH(QString, expectedTitle);

    bool wasJsEnabled = webView()->settings()->javaScriptEnabled();
    webView()->settings()->setJavaScriptEnabled(jsEnabled);
    webView()->setUrl(testUrl);
    QVERIFY(waitForLoadSucceeded(webView()));
    QTRY_COMPARE(webView()->title(), expectedTitle);
    webView()->settings()->setJavaScriptEnabled(wasJsEnabled);
}

QTEST_MAIN(tst_QQuickWebView)
#include "tst_qquickwebview.moc"
