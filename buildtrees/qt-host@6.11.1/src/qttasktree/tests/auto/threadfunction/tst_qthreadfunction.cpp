// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTaskTree/qthreadfunctiontask.h>

#include <QTest>

using namespace QtTaskTree;

using namespace std::chrono;
using namespace std::chrono_literals;

class MyObject
{
public:
    static void staticMember(QPromise<double> &promise, int n)
    {
        for (int i = 0; i < n; ++i)
            promise.addResult(0);
    }

    void member(QPromise<double> &promise, int n) const
    {
        for (int i = 0; i < n; ++i)
            promise.addResult(0);
    }
};

class tst_QThreadFunction : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void taskTree_data();
    void taskTree();
    void futureWatcher();

private:
    QThreadPool m_threadPool;
    MyObject m_myObject;
};

struct TestData
{
    Storage<bool> storage;
    Group root;
};

void report3(QPromise<int> &promise)
{
    promise.addResult(0);
    promise.addResult(2);
    promise.addResult(1);
}

static void staticReport3(QPromise<int> &promise)
{
    promise.addResult(0);
    promise.addResult(2);
    promise.addResult(1);
}

void reportN(QPromise<double> &promise, int n)
{
    for (int i = 0; i < n; ++i)
        promise.addResult(0);
}

static void staticReportN(QPromise<double> &promise, int n)
{
    for (int i = 0; i < n; ++i)
        promise.addResult(0);
}

class Functor {
public:
    void operator()(QPromise<double> &promise, int n) const
    {
        for (int i = 0; i < n; ++i)
            promise.addResult(0);
    }
};

void multiplyBy2(QPromise<int> &promise, int input) { promise.addResult(input * 2); }

template <typename...>
struct FutureArgType;

template <typename Arg>
struct FutureArgType<QFuture<Arg>>
{
    using Type = Arg;
};

template <typename...>
struct FunctionResultType;

template<typename Function, typename ...Args>
struct FunctionResultType<Function, Args...>
{
    using Type = typename FutureArgType<decltype(QtConcurrent::run(
        std::declval<Function>(), std::declval<Args>()...))>::Type;
};

template <typename Function, typename ...Args,
          typename ResultType = typename FunctionResultType<Function, Args...>::Type>
TestData createTestData(const QList<ResultType> &expectedResults, Function &&function, Args &&...args)
{
    Storage<bool> storage;

    const auto onSetup = [=](QThreadFunction<ResultType> &task) {
        task.setThreadFunctionData(function, args...);
    };
    const auto onDone = [storage, expectedResults](const QThreadFunction<ResultType> &task) {
        *storage = task.results() == expectedResults;
    };

    const Group root {
        storage,
        QThreadFunctionTask<ResultType>(onSetup, onDone)
    };

    return TestData{storage, root};
}

void tst_QThreadFunction::taskTree_data()
{
    QTest::addColumn<TestData>("testData");

    const QList<int> report3Result{0, 2, 1};
    const QList<double> reportNResult{0, 0};

    auto lambda = [](QPromise<double> &promise, int n) {
        for (int i = 0; i < n; ++i)
            promise.addResult(0);
    };
    const std::function<void(QPromise<double> &, int)> fun = [](QPromise<double> &promise, int n) {
        for (int i = 0; i < n; ++i)
            promise.addResult(0);
    };

    QTest::newRow("RefGlobalNoArgs")
        << createTestData(report3Result, &report3);
    QTest::newRow("GlobalNoArgs")
        << createTestData(report3Result, report3);
    QTest::newRow("RefStaticNoArgs")
        << createTestData(report3Result, &staticReport3);
    QTest::newRow("StaticNoArgs")
        << createTestData(report3Result, staticReport3);
    QTest::newRow("RefGlobalIntArg")
        << createTestData(reportNResult, &reportN, 2);
    QTest::newRow("GlobalIntArg")
        << createTestData(reportNResult, reportN, 2);
    QTest::newRow("RefStaticIntArg")
        << createTestData(reportNResult, &staticReportN, 2);
    QTest::newRow("StaticIntArg")
        << createTestData(reportNResult, staticReportN, 2);
    QTest::newRow("Lambda")
        << createTestData(reportNResult, lambda, 2);
    QTest::newRow("Function")
        << createTestData(reportNResult, fun, 2);
    QTest::newRow("Functor")
        << createTestData(reportNResult, Functor(), 2);
    QTest::newRow("StaticMemberFunction")
        << createTestData(reportNResult, &MyObject::staticMember, 2);
    QTest::newRow("MemberFunction")
        << createTestData(reportNResult, &MyObject::member, &m_myObject, 2);

    {
        Storage<bool> storage;
        Storage<int> internalStorage;

        const auto onSetup = [internalStorage](QThreadFunction<int> &task) {
            task.setThreadFunctionData(multiplyBy2, *internalStorage);
        };
        const auto onDone = [internalStorage](const QThreadFunction<int> &task) {
            *internalStorage = task.result();
        };

        const Group root {
            storage,
            internalStorage,
            onGroupSetup([internalStorage] { *internalStorage = 1; }),
            QThreadFunctionTask<int>(onSetup, onDone, CallDoneFlag::OnSuccess),
            QThreadFunctionTask<int>(onSetup, onDone, CallDoneFlag::OnSuccess),
            QThreadFunctionTask<int>(onSetup, onDone, CallDoneFlag::OnSuccess),
            QThreadFunctionTask<int>(onSetup, onDone, CallDoneFlag::OnSuccess),
            onGroupDone([storage, internalStorage] { *storage = *internalStorage == 16; })
        };

        QTest::newRow("Sequential") << TestData{storage, root};
    }
}

void tst_QThreadFunction::taskTree()
{
    QFETCH(TestData, testData);

    QTaskTree taskTree({testData.root.withTimeout(1000ms)});
    bool actualResult = false;
    const auto collectResult = [&actualResult](const bool &storage) {
        actualResult = storage;
    };
    taskTree.onStorageDone(testData.storage, collectResult);
    const DoneWith result = taskTree.runBlocking();
    QCOMPARE(taskTree.isRunning(), false);
    QCOMPARE(result, DoneWith::Success);
    QVERIFY(actualResult);
}

void tst_QThreadFunction::futureWatcher()
{
    QThreadFunction<int> task;
    QObject::connect(task.futureWatcher(), &QFutureWatcherBase::progressValueChanged,
                     qApp, [](int progress) { qDebug() << "progress" << progress; });
}

QTEST_GUILESS_MAIN(tst_QThreadFunction)

#include "tst_qthreadfunction.moc"
