// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qregularexpression.h>
#include <QtCore/qcoreapplication.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqmlcomponent.h>

#include <QtTest/qtest.h>

Q_IMPORT_QML_PLUGIN(MockExtensionPlugin)

class tst_qqmlextensionplugin_static : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void testExtensionPluginInstantiation();
    void testExtensionPluginInstantiation_initializeEngineCalledOnSecondEngine();
    void testExtensionPluginInstantiation_reInitializeInSecondApplication();
};

namespace {

template <typename Functor>
auto withQCoreApplication(Functor &&f)
{
    static int argc = 1;
    static char *argv[] = { (char *)"test" };
    std::unique_ptr<QCoreApplication> app = std::make_unique<QCoreApplication>(argc, argv);
    return f();
}

using namespace Qt::Literals;

auto itemWithComponent = R"(
    import MockExtension
    TestObject {
        id: testObj
    }
)"_ba;

static QRegularExpression operator""_re(const char16_t *str, size_t size) noexcept
{
    return QRegularExpression(operator""_s(str, qsizetype(size)));
}

void ignorePluginCtorOnce()
{
    static bool firstPluginCtorExpectationInitialized = false;
    if (!firstPluginCtorExpectationInitialized) {
        QTest::ignoreMessage(QtMsgType::QtWarningMsg, "MockExtensionPlugin instantiated");
        firstPluginCtorExpectationInitialized = true;
    } else {
        QTest::failOnWarning("MockExtensionPlugin instantiated");
    }
}
} // namespace

void tst_qqmlextensionplugin_static::initTestCase()
{
    // We don't want to see the functions called more often than expected.
    QTest::failOnWarning();
}

void tst_qqmlextensionplugin_static::init()
{
    // Each test needs to start with a clean slate so that we can see the registerTypes().
    qmlClearTypeRegistrations();
}

void tst_qqmlextensionplugin_static::testExtensionPluginInstantiation()
{
    withQCoreApplication([] {
        QQmlEngine engine;
        QQmlComponent component(&engine);

        ignorePluginCtorOnce();
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::registerTypes called with URI.*"_re);
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::initializeEngine called with URI.*"_re);
        component.setData(itemWithComponent, QUrl());

        QVERIFY2(component.isReady(), qPrintable(component.errorString()));

        QTest::ignoreMessage(QtMsgType::QtWarningMsg, "TestObject created");
        std::unique_ptr<QObject> obj{ component.create() };
        QVERIFY(obj);
    });
}

void tst_qqmlextensionplugin_static::
        testExtensionPluginInstantiation_initializeEngineCalledOnSecondEngine()
{
    withQCoreApplication([] {
        {
            QQmlEngine engine;
            QQmlComponent component(&engine);

            ignorePluginCtorOnce();
            QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                                 u"MockExtension::registerTypes called with URI.*"_re);
            QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                                 u"MockExtension::initializeEngine called with URI.*"_re);
            component.setData(itemWithComponent, QUrl());

            QVERIFY2(component.isReady(), qPrintable(component.errorString()));

            QTest::ignoreMessage(QtMsgType::QtWarningMsg, "TestObject created");
            std::unique_ptr<QObject> obj{ component.create() };
            QVERIFY(obj);
        }

        {
            QQmlEngine engine;
            QQmlComponent component(&engine);

            // types should be registered only once
            QTest::failOnWarning(u"MockExtension::registerTypes called with URI.*"_re);
            QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                                 u"MockExtension::initializeEngine called with URI.*"_re);
            component.setData(itemWithComponent, QUrl());

            QVERIFY2(component.isReady(), qPrintable(component.errorString()));

            QTest::ignoreMessage(QtMsgType::QtWarningMsg, "TestObject created");
            std::unique_ptr<QObject> obj{ component.create() };
            QVERIFY(obj);
        }
    });
}

void tst_qqmlextensionplugin_static::
        testExtensionPluginInstantiation_reInitializeInSecondApplication()
{
    withQCoreApplication([] {
        QQmlEngine engine;
        QQmlComponent component(&engine);

        ignorePluginCtorOnce();
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::registerTypes called with URI.*"_re);
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::initializeEngine called with URI.*"_re);
        component.setData(itemWithComponent, QUrl());

        QVERIFY2(component.isReady(), qPrintable(component.errorString()));

        QTest::ignoreMessage(QtMsgType::QtWarningMsg, "TestObject created");
        std::unique_ptr<QObject> obj{ component.create() };
        QVERIFY(obj);
    });

    qmlClearTypeRegistrations();

    withQCoreApplication([] {
        QQmlEngine engine;
        QQmlComponent component(&engine);

        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::registerTypes called with URI.*"_re);
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                             u"MockExtension::initializeEngine called with URI.*"_re);
        component.setData(itemWithComponent, QUrl());

        QVERIFY2(component.isReady(), qPrintable(component.errorString()));

        QTest::ignoreMessage(QtMsgType::QtWarningMsg, "TestObject created");
        std::unique_ptr<QObject> obj{ component.create() };
        QVERIFY(obj);
    });
}

QTEST_APPLESS_MAIN(tst_qqmlextensionplugin_static)

#include "tst_qqmlextensionplugin_static.moc"
