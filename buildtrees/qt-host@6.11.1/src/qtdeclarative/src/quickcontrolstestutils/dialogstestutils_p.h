// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DIALOGSTESTUTILS_H
#define DIALOGSTESTUTILS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>

#include <QtTest/qtest.h>
#include <QtTest/private/qtestresult_p.h>
#include <QtQml/qqmlfileselector.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QtQuickTest/QtQuickTest>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

// We need these for Windows, because FolderListModel returns a lowercase drive letter; e.g.:
// "file:///c:/blah.txt", whereas other API returns "file:///C:/blah.txt".
#define COMPARE_URL(url1, url2) \
    QCOMPARE(QFileInfo(url1.toLocalFile()).canonicalFilePath(), QFileInfo(url2.toLocalFile()).canonicalFilePath());

// Store a copy of the arguments in case { ... } list initializer syntax is used as an argument,
// which could result in two different lists being created and passed to std::transform()
// (and would also require it to be enclosed in parentheses everywhere it's used).
#define COMPARE_URLS(actualUrls, expectedUrls) \
{ \
    const QList<QUrl> actualUrlsCopy = actualUrls; \
    QList<QString> actualPaths; \
    std::transform(actualUrlsCopy.begin(), actualUrlsCopy.end(), std::back_insert_iterator(actualPaths), \
        [](const QUrl &url) { return QFileInfo(url.toLocalFile()).canonicalFilePath(); }); \
    const QList<QUrl> expectedUrlsCopy = expectedUrls; \
    QList<QString> expectedPaths; \
    std::transform(expectedUrlsCopy.begin(), expectedUrlsCopy.end(), std::back_insert_iterator(expectedPaths), \
        [](const QUrl &url) { return QFileInfo(url.toLocalFile()).canonicalFilePath(); }); \
    QCOMPARE(actualPaths, expectedPaths); \
}

#define OPEN_QUICK_DIALOG() \
QVERIFY2(dialogHelper.isWindowInitialized(), dialogHelper.failureMessage()); \
QVERIFY(dialogHelper.waitForWindowActive()); \
QVERIFY(dialogHelper.openDialog()); \
QTRY_VERIFY(dialogHelper.isQuickDialogOpen());

#define CLOSE_QUICK_DIALOG() \
    do { \
        dialogHelper.dialog->close(); \
        QVERIFY(!dialogHelper.dialog->isVisible()); \
        QTRY_VERIFY(!dialogHelper.quickDialog->isVisible()); \
    } while (false)

QT_BEGIN_NAMESPACE
class QWindow;

class QQuickListView;

class QQuickAbstractButton;

class QQuickDialogButtonBox;
class QQuickFolderBreadcrumbBar;

namespace QQuickDialogTestUtils
{

// Saves duplicating a bunch of code in every test.
template<typename DialogType, typename QuickDialogType>
class DialogTestHelper
{
public:
    DialogTestHelper(QQmlDataTest *testCase, const QString &testFilePath,
            const QStringList &qmlImportPaths = {}, const QVariantMap &initialProperties = {}) :
        appHelper(testCase, testFilePath, initialProperties, qmlImportPaths)
    {
        if (!appHelper.ready)
            return;

        // The QQmlFileSelector constructor will append itself to the typeloader's
        // QQmlTypeLoaderConfiguredData::urlInterceptors list.
        // QQmlApplicationEnginePrivate::init() does the same thing for normal quick applications,
        // but we need to do it again here, to enable file selector functionality for dialog tests.
        fileSelector = new QQmlFileSelector(&appHelper.engine);

        dialog = appHelper.window->property("dialog").value<DialogType*>();
        if (!dialog) {
            appHelper.errorMessage = "\"dialog\" property is not valid";
            return;
        }

        appHelper.window->show();
        appHelper.window->requestActivate();
    }

    ~DialogTestHelper()
    {
        dialog->close();
    }

    Q_REQUIRED_RESULT bool isWindowInitialized() const
    {
        return appHelper.ready;
    }

    Q_REQUIRED_RESULT bool waitForWindowActive()
    {
        return QTest::qWaitForWindowActive(appHelper.window);
    }

    /*
        Opens the dialog. For non-native dialogs, it is necessary to ensure that
        isQuickDialogOpen() returns true before trying to access its internals.
    */
    virtual bool openDialog()
    {
        dialog->open();
        if (!dialog->isVisible()) {
            appHelper.errorMessage = "Dialog is not visible";
            return false;
        }

        // We might want to call this function more than once,
        // and we only need to get these members the first time.
        if (!quickDialog) {
            quickDialog = appHelper.window->findChild<QuickDialogType*>();
            if (!quickDialog) {
                appHelper.errorMessage = "Can't find Qt Quick-based dialog";
                return false;
            }
        }

        return true;
    }

    QQuickWindow *popupWindow() const
    {
        return quickDialog->contentItem()->window();
    }

    Q_REQUIRED_RESULT bool waitForPopupWindowActiveAndPolished()
    {
        if (!isQuickDialogOpen()) {
            QByteArray msg("Dialog wasn't open, when {} was called");
            msg.replace("{}", __func__);
            appHelper.errorMessage = msg;
            return false;
        }
        QQuickWindow *dialogPopupWindow = popupWindow();
        if (!dialogPopupWindow)
            return false;
        dialogPopupWindow->requestActivate();
        if (!QTest::qWaitForWindowActive(dialogPopupWindow))
            return false;
        return QQuickTest::qWaitForPolish(dialogPopupWindow);
    }

    bool isQuickDialogOpen() const
    {
        return quickDialog->isOpened();
    }

    QQuickWindow *window() const
    {
        return appHelper.window;
    }

    const char *failureMessage() const
    {
        return appHelper.errorMessage.constData();
    }

    QQuickVisualTestUtils::QQuickApplicationHelper appHelper;
    DialogType *dialog = nullptr;
    QuickDialogType *quickDialog = nullptr;
    QQmlFileSelector *fileSelector = nullptr;
};

bool verifyFileDialogDelegates(QQuickListView *fileDialogListView, const QStringList &expectedFiles, QString &failureMessage);

bool verifyBreadcrumbDelegates(QQuickFolderBreadcrumbBar *breadcrumbBar, const QUrl &expectedFolder, QString &failureMessage);

QQuickAbstractButton *findDialogButton(QQuickDialogButtonBox *box, const QString &buttonText);

void enterText(QWindow *window, const QString &textToEnter);
}

#define QTEST_QUICKDIALOGS_MAIN(TestCase) \
int main(int argc, char *argv[]) \
{ \
    qputenv("QML_NO_TOUCH_COMPRESSION", "1"); \
    QGuiApplication app(argc, argv); \
    TestCase tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    int res = 0; \
    QTest::qInit(&tc, argc, argv); \
    const QByteArray testObjectName = QTestResult::currentTestObjectName(); \
    QByteArray testName; \
    const QStringList allStyles = {"Basic", "Fusion", "Imagine", "Material", "Universal"}; \
    const QStringList styles = qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_STYLE") ? QStringList{qEnvironmentVariable("QT_QUICK_CONTROLS_STYLE")} : allStyles; \
    for (const QString &style : styles) { \
        qmlClearTypeRegistrations(); \
        QQuickStyle::setStyle(style); \
        testName = testObjectName + "::" + style.toLocal8Bit(); \
        QTestResult::setCurrentTestObject(testName); \
        res += QTest::qRun(); \
    } \
    QTestResult::setCurrentTestObject(testObjectName); \
    QTest::qCleanup(); \
    return res; \
}


QT_END_NAMESPACE

#endif // DIALOGSTESTUTILS_H
