// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qdir.h>
#include <QtCore/qtemporarydir.h>
#include <QtCore/qfileinfo.h>
#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewsettings.h>
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWebView/private/qwebviewfactory_p.h>

#include <testutil.h>

#if defined(Q_OS_ANDROID) && !defined(Q_OS_ANDROID_NO_SDK)
#include <QtCore/private/qjnihelpers_p.h>
#define ANDROID_REQUIRES_API_LEVEL(N) \
    if (QtAndroidPrivate::androidSdkVersion() < N) \
        QSKIP("This feature is not supported on this version of Android");
#else
#define ANDROID_REQUIRES_API_LEVEL(N)
#endif

class TestWebView : public QWebView {
public:
    TestWebView() : QWebView() { settings()->setAttribute(QWebViewSettings::WebAttribute::AllowFileAccess, true); };
    QSignalSpy urlSpy = QSignalSpy(this, SIGNAL(urlChanged(QUrl)));
    QSignalSpy loadingSpy = QSignalSpy(this, SIGNAL(loadingChanged(QWebViewLoadingInfo)));
    QSignalSpy titleSpy = QSignalSpy(this, SIGNAL(titleChanged(QString)));
};

class tst_QWebView : public QObject
{
    Q_OBJECT
public:
    tst_QWebView() : m_cacheLocation(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)) {}

private slots:
    void initTestCase();

    void startupState();

    void load();
    void loadNonexistentFileUrl();
    void loadHtml_data();
    void loadHtml();
    void loadRequest();

    void backAndForward();
    void reload();

    void runJavaScript();
    void setAndDeleteCookie();

private:
    const QString m_cacheLocation;
};

void tst_QWebView::initTestCase()
{
    if (!qEnvironmentVariableIsEmpty("QEMU_LD_PREFIX"))
        QSKIP("This test is unstable on QEMU, so it will be skipped.");
    if (!QFileInfo(m_cacheLocation).isDir()) {
        QDir dir;
        QVERIFY(dir.mkpath(m_cacheLocation));
    }
}

void tst_QWebView::startupState()
{
    QWebView webView;
    QVERIFY(!webView.canGoBack());
    QVERIFY(!webView.canGoForward());
    QVERIFY(!webView.isLoading());
    QCOMPARE(webView.url(), QUrl());
}

void tst_QWebView::load()
{
    TestWebView webView;
    webView.setUrl(makeTestFileUrl("basic_page.html"));

    QTRY_COMPARE(webView.loadingSpy.size(), 2);
    QCOMPARE(webView.urlSpy.size(), 1);
    QCOMPARE(webView.loadProgress(), 100);
    QCOMPARE(webView.loadingSpy[0][0].value<QWebViewLoadingInfo>().status(), QWebViewLoadingInfo::LoadStatus::Started);
    QCOMPARE(webView.loadingSpy[1][0].value<QWebViewLoadingInfo>().status(), QWebViewLoadingInfo::LoadStatus::Succeeded);
    QCOMPARE(webView.title(), QStringLiteral("Basic Page"));
}

void tst_QWebView::loadNonexistentFileUrl()
{
    TestWebView webView;
    webView.setUrl(QUrl::fromLocalFile("file_that_does_not_exist.html"));
    QTRY_COMPARE(webView.loadingSpy.size(), 2);
    QCOMPARE(webView.loadingSpy[1][0].value<QWebViewLoadingInfo>().status(), QWebViewLoadingInfo::LoadStatus::Failed);
}

void tst_QWebView::loadHtml_data()
{
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QUrl>("loadUrl");
    QTest::addColumn<QUrl>("resultUrl");

    const QByteArray content(
            QByteArrayLiteral("<html><title>WebViewTitle</title>"
                              "<body><span style=\"color:#ff0000\">Hello</span></body></html>"));
    QByteArray encoded("data:text/html;charset=UTF-8,");
    encoded.append(content.toPercentEncoding());

    if (!QWebViewFactory::loadedPluginHasKey("webkit")) {
        QTest::newRow("set content without base url") << content << QUrl() << QUrl(encoded);
    } else {
        QTest::newRow("set content without base url") << content << QUrl() << QUrl("about:blank");
    }
    QTest::newRow("set content with data base url") << content << QUrl(encoded) << QUrl(encoded);

    if (!QWebViewFactory::loadedPluginHasKey("webview2")) {
        QTest::newRow("set content with non-data base url")
                << content << QUrl("http://foobar.com/") << QUrl("http://foobar.com/");
    } else {
        QTest::newRow("set content with non-data base url")
                << content << QUrl("http://foobar.com/") << QUrl(encoded);
    }
}

void tst_QWebView::loadHtml()
{
    QFETCH(QByteArray, content);
    QFETCH(QUrl, loadUrl);
    QFETCH(QUrl, resultUrl);

    TestWebView view;
    view.loadHtml(content, loadUrl);
    QTRY_COMPARE(view.loadingSpy.size(), 2);
    QVERIFY(!view.isLoading());
    QCOMPARE(view.loadProgress(), 100);
    QTRY_COMPARE(view.title(), QStringLiteral("WebViewTitle"));
    // take load finished
    const QWebViewLoadingInfo &lr = view.loadingSpy.at(1).at(0).value<QWebViewLoadingInfo>();
    QCOMPARE(lr.status(), QWebViewLoadingInfo::LoadStatus::Succeeded);
    bool called = false;
    QUrl url;
    auto callback = [&](const QVariant &result) {
        called = true;
        url = result.value<QUrl>();
    };
    view.runJavaScript("document.baseURI", callback);
    QTRY_COMPARE(called, true);
    QCOMPARE(url, resultUrl);

    QVERIFY(view.url().isValid());
    QCOMPARE(view.url(), resultUrl);
}

void tst_QWebView::loadRequest()
{
    // LoadSucceeded
    {
        QTemporaryFile file(m_cacheLocation + QStringLiteral("/XXXXXXfile.html"));
        QVERIFY2(file.open(),
                 qPrintable(QStringLiteral("Cannot create temporary file:") + file.errorString()));

        file.write("<html><head><title>FooBar</title></head><body/></html>");
        const QString fileName = file.fileName();
        file.close();

        QWebView view;

        view.settings()->setAttribute(QWebViewSettings::WebAttribute::AllowFileAccess, true);
        view.settings()->setAttribute(QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls,
                                      true);
        const QUrl url = QUrl::fromLocalFile(fileName);
        QSignalSpy loadChangedSignalSpy(&view, SIGNAL(loadingChanged(QWebViewLoadingInfo)));
        view.setUrl(url);
        QTRY_COMPARE(loadChangedSignalSpy.size(), 2);
        QVERIFY(!view.isLoading());
        QCOMPARE(view.loadProgress(), 100);
        QTRY_COMPARE(view.title(), QStringLiteral("FooBar"));
        QCOMPARE(view.url(), url);
        {
            const QList<QVariant> &loadStartedArgs = loadChangedSignalSpy.takeFirst();
            const QWebViewLoadingInfo &lr = loadStartedArgs.at(0).value<QWebViewLoadingInfo>();
            QCOMPARE(lr.status(), QWebViewLoadingInfo::LoadStatus::Started);
        }
        {
            const QList<QVariant> &loadStartedArgs = loadChangedSignalSpy.takeFirst();
            const QWebViewLoadingInfo &lr = loadStartedArgs.at(0).value<QWebViewLoadingInfo>();
            QCOMPARE(lr.status(), QWebViewLoadingInfo::LoadStatus::Succeeded);
        }
    }

    // LoadFailed
    {
        QWebView view;
        view.settings()->setAttribute(QWebViewSettings::WebAttribute::AllowFileAccess, true);
        view.settings()->setAttribute(QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls,
                                      true);
        QSignalSpy loadChangedSignalSpy(&view, SIGNAL(loadingChanged(QWebViewLoadingInfo)));
        view.setUrl(QUrl(QStringLiteral("file:///file_that_does_not_exist.html")));
        QTRY_VERIFY(!view.isLoading());
        QTRY_COMPARE(loadChangedSignalSpy.size(), 2);
        {
            const QList<QVariant> &loadStartedArgs = loadChangedSignalSpy.takeFirst();
            const QWebViewLoadingInfo &lr = loadStartedArgs.at(0).value<QWebViewLoadingInfo>();
            QCOMPARE(lr.status(), QWebViewLoadingInfo::LoadStatus::Started);
        }
        {
            const QList<QVariant> &loadStartedArgs = loadChangedSignalSpy.takeFirst();
            const QWebViewLoadingInfo &lr = loadStartedArgs.at(0).value<QWebViewLoadingInfo>();
            QCOMPARE(lr.status(), QWebViewLoadingInfo::LoadStatus::Failed);
        }
        if (QWebViewFactory::loadedPluginHasKey("webengine"))
            QCOMPARE(view.loadProgress(), 100);
    }
}

void tst_QWebView::backAndForward()
{
    TestWebView webView;
    QUrl page1 = makeTestFileUrl("basic_page.html");
    QUrl page2 = makeTestFileUrl("basic_page2.html");

    webView.setUrl(page1);
    QTRY_COMPARE(webView.loadingSpy.size(), 2);
    QVERIFY(!webView.canGoBack());
    QVERIFY(!webView.canGoForward());

    webView.setUrl(page2);
    QTRY_COMPARE(webView.loadingSpy.size(), 4);
    QVERIFY(webView.canGoBack());
    QVERIFY(!webView.canGoForward());

    webView.goBack();
    QTRY_COMPARE(webView.loadingSpy.size(), 6);
    QVERIFY(!webView.canGoBack());
    QVERIFY(webView.canGoForward());

    webView.goForward();
    QTRY_COMPARE(webView.loadingSpy.size(), 8);
    QVERIFY(webView.canGoBack());
    QVERIFY(!webView.canGoForward());
}

void tst_QWebView::reload()
{
    TestWebView webView;
    QUrl url = makeTestFileUrl("basic_page.html");
    webView.setUrl(url);
    QTRY_COMPARE(webView.loadingSpy.size(), 2);

    webView.reload();
    QTRY_COMPARE(webView.loadingSpy.size(), 4);

    QCOMPARE(webView.loadingSpy[3][0].value<QWebViewLoadingInfo>().status(), QWebViewLoadingInfo::LoadStatus::Succeeded);
    QCOMPARE(webView.url(), url);
}

void tst_QWebView::runJavaScript()
{
    ANDROID_REQUIRES_API_LEVEL(19)

    QWebView view;
    view.show();
    view.loadHtml(QString("<html><head><title>WebViewTitle</title></head><body/></html>"));
    QTRY_COMPARE(view.loadProgress(), 100);
    QTRY_VERIFY(!view.isLoading());
    QTRY_COMPARE(view.title(), "WebViewTitle");

    bool callbackCalled = false;
    view.runJavaScript(QString(QLatin1String("document.title")), [&callbackCalled](const QVariant &result) {
        QCOMPARE(result.toString(), "WebViewTitle");
        callbackCalled = true;
    });

    QTRY_VERIFY(callbackCalled);
}

void tst_QWebView::setAndDeleteCookie()
{
    QWebView view;
    view.settings()->setAttribute(QWebViewSettings::WebAttribute::AllowFileAccess, true);
    view.settings()->setAttribute(QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls,
                                  true);
    view.settings()->setAttribute(QWebViewSettings::WebAttribute::LocalStorageEnabled, true);

    QSignalSpy cookieAddedSpy(&view, SIGNAL(cookieAdded(QString,QString)));
    QSignalSpy cookieRemovedSpy(&view, SIGNAL(cookieRemoved(QString,QString)));

    view.setCookie(".example.com", "TestCookie", "testValue");
    view.setCookie(".example2.com", "TestCookie2", "testValue2");
    view.setCookie(".example3.com", "TestCookie3", "testValue3");
    QTRY_COMPARE(cookieAddedSpy.size(), 3);

    view.deleteCookie(".example.com", "TestCookie");
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);

    // deleting a cookie using a name that has not been set
    view.deleteCookie(".example.com", "NewCookieName");
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);

    // deleting a cookie using a domain that has not been set
    view.deleteCookie(".new.domain.com", "TestCookie2");
    QTRY_COMPARE(cookieRemovedSpy.size(), 1);

    view.deleteAllCookies();
    if (QWebViewFactory::loadedPluginHasKey("android_view"))
        QEXPECT_FAIL("", "Notification for deleteAllCookies() is not implemented on Android, yet!", Continue); // FIXME
    QTRY_COMPARE(cookieRemovedSpy.size(), 3);
}

QTEST_MAIN(tst_QWebView)

#include "tst_qwebview.moc"
