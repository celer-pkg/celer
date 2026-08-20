// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <emscripten/val.h>
#include <emscripten.h>

#include <QtCore/private/qstdweb_p.h>

namespace {

emscripten::val g_testSupport;

void init() {
    g_testSupport = emscripten::val::object();
    EM_ASM({
        var testSupport = Emval.toValue($0);
        testSupport.resolve = {};
        testSupport.reject = {};
        testSupport.promises = {};
        testSupport.waitConditionPromise = new Promise((resolve, reject) => {
            testSupport.finishWaiting = resolve;
        });

        testSupport.makeTestPromise = (param) => {
            testSupport.promises[param] = new Promise((resolve, reject) => {
                testSupport.resolve[param] = resolve;
                testSupport.reject[param] = reject;
            });

            return testSupport.promises[param];
        };
    }, g_testSupport.as_handle());
}
}

class tst_QWasmPromise : public QObject
{
    Q_OBJECT

public:
    tst_QWasmPromise() = default;

private slots:
    void init();
    void suspendExclusive();
    void simpleResolve();
    void multipleResolve();
    void simpleReject();
    void multipleReject();
    void throwInThen();
    void bareFinally();
    void finallyWithThen();
    void finallyWithThrow();
    void finallyWithThrowInThen();
    void nested();
    void all();
    void allWithThrow();
    void allWithFinally();
    void allWithFinallyAndThrow();
};

static bool g_Done = false;

#define QWASMDONE() g_Done = true;

void tst_QWasmPromise::init() {
    g_Done = false;
    ::init();
}

class BarrierCallback {
public:
    BarrierCallback(int number, std::function<void()> onDone)
        : m_remaining(number), m_onDone(std::move(onDone)) {}

    void operator()() {
        if (!--m_remaining) {
            m_onDone();
        }
    }

private:
    int m_remaining;
    std::function<void()> m_onDone;
};

void tst_QWasmPromise::suspendExclusive()
{
    init();

    {
        auto promise = qstdweb::Promise(
            g_testSupport,
            "makeTestPromise",
            std::string("simpleResolve"))
        .addThenFunction([](emscripten::val result) {
            QVERIFY(result.isString());
            QCOMPARE("Some lovely data", result.as<std::string>());
            QWASMDONE();
        })
        .addCatchFunction([](emscripten::val error) {
            Q_UNUSED(error);
            QFAIL("Unexpected catch");
            QWASMDONE();
        });

        g_testSupport["resolve"].call<void>("simpleResolve", std::string("Some lovely data"));
        promise.suspendExclusive();
    }
    QVERIFY(g_Done);
    QVERIFY(qstdweb::Promise::State::numInstances() == 0);
}

void tst_QWasmPromise::simpleResolve()
{
    init();

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("simpleResolve"))
    .addThenFunction([](emscripten::val result) {
        QVERIFY(result.isString());
        QCOMPARE("Some lovely data", result.as<std::string>());
        QWASMDONE();
    })
    .addCatchFunction([](emscripten::val error) {
        Q_UNUSED(error);
        QFAIL("Unexpected catch");
        QWASMDONE();
    })
    .addFinallyFunction([](){});

    g_testSupport["resolve"].call<void>("simpleResolve", std::string("Some lovely data"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::multipleResolve()
{
    init();
    static constexpr int promiseCount = 1000;

    auto onThen = std::make_shared<BarrierCallback>(promiseCount, []() {
        QWASMDONE();
    });

    for (int i = 0; i < promiseCount; ++i) {
        qstdweb::Promise(
            g_testSupport,
            "makeTestPromise",
            (QStringLiteral("test") + QString::number(i)).toStdString())
        .addThenFunction([=](emscripten::val result) {
            QVERIFY(result.isString());
            QCOMPARE(QString::number(i).toStdString(), result.as<std::string>());
            (*onThen)();
        })
        .addCatchFunction([](emscripten::val error) {
            Q_UNUSED(error);
            QFAIL("Unexpected catch");
            QWASMDONE();
        });
    }

    for (int i = 0; i < promiseCount; ++i)
        g_testSupport["resolve"].call<void>(("test" + std::to_string(i)).c_str(), std::to_string(i));

    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::simpleReject()
{
    init();

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("simpleReject"))
    .addThenFunction([](emscripten::val result) {
        Q_UNUSED(result);
        QFAIL("Unexpected then");
        QWASMDONE();
    })
    .addCatchFunction([](emscripten::val result) {
        QVERIFY(result.isString());
        QCOMPARE("Evil error", result.as<std::string>());
        QWASMDONE();
    });

    g_testSupport["reject"].call<void>("simpleReject", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::multipleReject()
{
    static constexpr int promiseCount = 1000;

    auto onCatch = std::make_shared<BarrierCallback>(promiseCount, []() {
        QWASMDONE();
    });

    for (int i = 0; i < promiseCount; ++i) {
        qstdweb::Promise(
            g_testSupport,
            "makeTestPromise",
            (QStringLiteral("test") + QString::number(i)).toStdString())
        .addThenFunction([](emscripten::val result) {
            Q_UNUSED(result);
            QFAIL("Unexpected then");
        })
        .addCatchFunction([=](emscripten::val error) {
            Q_UNUSED(error);
            (*onCatch)();
        });
    }

    for (int i = 0; i < promiseCount; ++i)
        g_testSupport["reject"].call<void>(("test" + std::to_string(i)).c_str(), std::to_string(i));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::throwInThen()
{
    init();
    QSKIP("Throw not supported");
    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("throwInThen"))
    .addThenFunction([](emscripten::val result) {
        Q_UNUSED(result);
        EM_ASM({
            throw "Expected error";
        });
    })
    .addCatchFunction([](emscripten::val error) {
        QCOMPARE("Expected error", error.as<std::string>());
        QWASMDONE();
    });

    g_testSupport["resolve"].call<void>("throwInThen", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::bareFinally()
{
    init();

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("bareFinally"))
    .addFinallyFunction([]() {
        QWASMDONE();
    });

    g_testSupport["resolve"].call<void>("bareFinally", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::finallyWithThen()
{
    init();

    bool *thenCalled = new bool(false);
    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("finallyWithThen"))
    .addThenFunction([thenCalled] (emscripten::val result) {
        Q_UNUSED(result);
        *thenCalled = true;
    })
    .addFinallyFunction([thenCalled]() {
        QVERIFY(*thenCalled);
        delete thenCalled;
        QWASMDONE();
    });

    g_testSupport["resolve"].call<void>("finallyWithThen", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::finallyWithThrow()
{
    init();

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("finallyWithThrow"))
    .addCatchFunction([](emscripten::val error) {
        Q_UNUSED(error);
    })
    .addFinallyFunction([]() {
        QWASMDONE();
    });

    g_testSupport["reject"].call<void>("finallyWithThrow", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::finallyWithThrowInThen()
{
    init();
    QSKIP("Throw not supported");

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("finallyWithThrowInThen"))
    .addThenFunction([](emscripten::val result) {
        Q_UNUSED(result);
        EM_ASM({
            throw "Expected error";
        });
    })
    .addCatchFunction([](emscripten::val result) {
        QVERIFY(result.isString());
        QCOMPARE("Expected error", result.as<std::string>());
    })
    .addFinallyFunction([]() {
        QWASMDONE();
    });

    g_testSupport["resolve"].call<void>("finallyWithThrowInThen", std::string("Evil error"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::nested()
{
    init();

    qstdweb::Promise(
        g_testSupport,
        "makeTestPromise",
        std::string("outer"))
    .addThenFunction([](emscripten::val result) {
        QVERIFY(result.isString());
        QCOMPARE("Outer data", result.as<std::string>());
        qstdweb::Promise(
            g_testSupport,
            "makeTestPromise",
            std::string("inner"))
        .addThenFunction([](emscripten::val innerResult) {
            QVERIFY(innerResult.isString());
            QCOMPARE("Inner data", innerResult.as<std::string>());
            qstdweb::Promise(
                g_testSupport,
                "makeTestPromise",
                 std::string("innermost"))
            .addThenFunction([](emscripten::val innerResult) {
                QVERIFY(innerResult.isString());
                QCOMPARE("Innermost data", innerResult.as<std::string>());
                QWASMDONE();
            })
            .addCatchFunction([](emscripten::val error) {
                Q_UNUSED(error);
                QFAIL("Unexpected catch");
            });
            g_testSupport["resolve"].call<void>("innermost", std::string("Innermost data"));
        });
        g_testSupport["resolve"].call<void>("inner", std::string("Inner data"));
    })
    .addCatchFunction([](emscripten::val error) {
        Q_UNUSED(error);
        QFAIL("Unexpected catch");
    });

    g_testSupport["resolve"].call<void>("outer", std::string("Outer data"));
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::all()
{
    init();

    {
        static constexpr int promiseCount = 1000;
        auto thenCalledOnce = std::make_shared<bool>(true);

        std::vector<qstdweb::Promise> promises;
        promises.reserve(promiseCount);

        for (int i = 0; i < promiseCount; ++i) {
            promises.push_back(
                qstdweb::Promise(
                    g_testSupport,
                "makeTestPromise",
                emscripten::val(("all" + QString::number(i)).toStdString())));
        }

        qstdweb::Promise(
            promises)
        .addThenFunction([thenCalledOnce](emscripten::val result) {
            QVERIFY(*thenCalledOnce);
            *thenCalledOnce = false;

            QVERIFY(result.isArray());
            QCOMPARE(promiseCount, result["length"].as<int>());
            for (int i = 0; i < promiseCount; ++i)
                QCOMPARE(QStringLiteral("Data %1").arg(i).toStdString(), result[i].as<std::string>());

            QWASMDONE();
        })
        .addCatchFunction([](emscripten::val error) {
            Q_UNUSED(error);
            QFAIL("Unexpected catch");
        });

        for (int i = promiseCount - 1; i >= 0; --i)
            g_testSupport["resolve"].call<void>(("all" + std::to_string(i)).c_str(), "Data " + std::to_string(i));
    }
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::allWithThrow()
{
    init();

    {
        auto promise1 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise1"));
        auto promise2 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise2"));
        auto promise3 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise3"));
        auto catchCalledOnce = std::make_shared<bool>(true);

        qstdweb::Promise(
            {promise1, promise2, promise3})
        .addThenFunction([](emscripten::val result) {
            Q_UNUSED(result);
            QFAIL("Unexpected then");
        })
        .addCatchFunction([catchCalledOnce](emscripten::val result) {
            QVERIFY(*catchCalledOnce);
            *catchCalledOnce = false;
            QVERIFY(result.isString());
            QCOMPARE("Error 2", result.as<std::string>());
            QWASMDONE();
        });

        g_testSupport["resolve"].call<void>("promise3", std::string("Data 3"));
        g_testSupport["resolve"].call<void>("promise1", std::string("Data 1"));
        g_testSupport["reject"].call<void>("promise2", std::string("Error 2"));
    }

    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::allWithFinally()
{
    init();
    {
        auto promise1 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise1"));
        auto promise2 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise2"));
        auto promise3 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise3"));

        auto finallyCalledOnce = std::make_shared<bool>(true);

        qstdweb::Promise(
            {promise1, promise2, promise3})
        .addThenFunction([](emscripten::val result) {
            Q_UNUSED(result);
        })
        .addFinallyFunction([finallyCalledOnce]() {
            QVERIFY(*finallyCalledOnce);
            *finallyCalledOnce = false;
            QWASMDONE();
        });

        g_testSupport["resolve"].call<void>("promise3", std::string("Data 3"));
        g_testSupport["resolve"].call<void>("promise1", std::string("Data 1"));
        g_testSupport["resolve"].call<void>("promise2", std::string("Data 2"));
    }
    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

void tst_QWasmPromise::allWithFinallyAndThrow()
{
    init();
    QSKIP("Throw not supported");
    {
        auto promise1 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise1"));
        auto promise2 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise2"));
        auto promise3 = qstdweb::Promise(g_testSupport, "makeTestPromise", std::string("promise3"));
        auto finallyCalledOnce = std::make_shared<bool>(true);

        qstdweb::Promise(
            {promise1, promise2, promise3})
        .addThenFunction([](emscripten::val result) {
            Q_UNUSED(result);
            EM_ASM({
                throw "This breaks it all!!!";
            });
        })
        .addCatchFunction([](emscripten::val) { ; })
        .addFinallyFunction([finallyCalledOnce]() {
            QVERIFY(*finallyCalledOnce);
            *finallyCalledOnce = false;
            QWASMDONE();
        });

        g_testSupport["resolve"].call<void>("promise3", std::string("Data 3"));
        g_testSupport["resolve"].call<void>("promise1", std::string("Data 1"));
        g_testSupport["resolve"].call<void>("promise2", std::string("Data 2"));
    }

    QVERIFY(QTest::qWaitFor([]() { return g_Done; }));
    QVERIFY(QTest::qWaitFor([]() { return qstdweb::Promise::State::numInstances() == 0; }));
}

QTEST_MAIN(tst_QWasmPromise)
#include "tst_qwasmpromise.moc"
