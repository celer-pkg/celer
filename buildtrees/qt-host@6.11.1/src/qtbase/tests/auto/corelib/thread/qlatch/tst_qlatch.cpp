// Copyright (C) 2025 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "private/qlatch_p.h"
#include <QtTest>

#include <thread>

#ifdef QATOMICWAIT_USE_FALLBACK
#  define tst_QLatch tst_QLatch_Fallback
#endif

using namespace std::chrono_literals;

constexpr int Repeats = 16;
constexpr int MaxThreads = 4;

class tst_QLatch : public QObject
{
    Q_OBJECT
private slots:
    void basics();
    void simple();
    void multipleWorkersSingleWaiter_data();
    void multipleWorkersSingleWaiter();
    void singleWorkerMultipleWaiters_data();
    void singleWorkerMultipleWaiters();
    void multipleWorkersAndWaiters_data();
    void multipleWorkersAndWaiters();
};

class JThread
{
    std::thread thr;
public:
    template <typename... Args> JThread(Args &&... args)
        : thr(std::forward<Args>(args)...)
    { }

    ~JThread()
    {
        wait();
    }

    void wait()
    {
        if (thr.joinable())
            thr.join();
    }
};

void maybeSleep(std::chrono::milliseconds ms)
{
    if (ms > 0s)
        QTest::qSleep(ms);
}

void tst_QLatch::basics()
{
    {
        QLatch latch(0);
        QCOMPARE(latch.pending(), 0);
        QVERIFY(latch.tryWait());
        latch.wait();       // doesn't deadlock
    }

    {
        QLatch latch(1);
        QCOMPARE(latch.pending(), 1);
        QVERIFY(!latch.tryWait());
        latch.countDown();
        QVERIFY(latch.tryWait());
        QCOMPARE(latch.pending(), 0);
        latch.wait();       // doesn't deadlock
    }

    {
        QLatch latch(2);
        QCOMPARE(latch.pending(), 2);
        QVERIFY(!latch.tryWait());
        latch.countDown(2);
        QVERIFY(latch.tryWait());
        QCOMPARE(latch.pending(), 0);
        latch.wait();       // doesn't deadlock
    }
}

void tst_QLatch::simple()
{
    // simple thread synchronization
    for (int i = 0; i < Repeats; ++i) {
        int j = 0, k = 0;

        QLatch latch(1);
        auto waiterCode = [&]() { latch.wait(); k = j; };
        auto workerCode = [&]() { j += i; latch.countDown(); };

        if (i & 1) {
            // waiter first
            JThread waiter(waiterCode);
            JThread worker(workerCode);
        } else {
            // worker first
            JThread worker(workerCode);
            JThread waiter(waiterCode);
        }

        QCOMPARE(j, i);
        QCOMPARE(k, i);
    }
}

void tst_QLatch::multipleWorkersSingleWaiter_data()
{
    QTest::addColumn<int>("count");
    QTest::newRow("1") << 1;
    QTest::newRow("2") << 2;
    QTest::newRow("32768") << 32768;
}

void tst_QLatch::multipleWorkersSingleWaiter()
{
    QFETCH(int, count);
    for (int i = 0; i < Repeats; ++i) {
        int pendingValue = 0xbadbad;
        Q_ASSERT(qint64(count) * MaxThreads <= std::numeric_limits<int>::max());

        QLatch latch(MaxThreads * count);
        std::unique_ptr<JThread> workers[MaxThreads];
        std::unique_ptr<JThread> waiter;

        auto waiterCode = [&]() {
            latch.wait();
            pendingValue = latch.pending();
        };
        if (i & 1)
            waiter.reset(new JThread(waiterCode));

        for (auto &ptr : workers)
            ptr.reset(new JThread([&]() { latch.countDown(count); }));

        if ((i & 1) == 0)
            waiter.reset(new JThread(waiterCode));

        waiter->wait();
        QCOMPARE(latch.pending(), 0);
        QCOMPARE(pendingValue, 0);
    }
}

void tst_QLatch::singleWorkerMultipleWaiters_data()
{
    multipleWorkersSingleWaiter_data();
}

void tst_QLatch::singleWorkerMultipleWaiters()
{
    QFETCH(int, count);
    for (int i = 0; i < Repeats; ++i) {
        QLatch latch(count);
        std::unique_ptr<JThread> waiters[MaxThreads];
        std::unique_ptr<JThread> worker;
        std::array<int, MaxThreads> pendingValues;
        pendingValues.fill(0xcdcdcdcd);

        auto workerCode = [&]() { latch.countDown(count); };
        auto waiterCode = [&](int *ptr) {
            latch.wait();
            *ptr = latch.pending();
        };
        if (i & 1)
            worker.reset(new JThread(workerCode));

        for (int i = 0; i < MaxThreads; ++i)
            waiters[i].reset(new JThread(waiterCode, &pendingValues[i]));

        if ((i & 1) == 0)
            worker.reset(new JThread(workerCode));

        // wait for an arbitrary waiter
        waiters[i % MaxThreads]->wait();

        QCOMPARE(latch.pending(), 0);
        QCOMPARE(pendingValues[i % MaxThreads], 0);

        // wait for all waiters to finish
        for (auto &waiter : waiters)
            waiter->wait();
        QCOMPARE(latch.pending(), 0);
        QCOMPARE(pendingValues, decltype(pendingValues){});
    }
}

void tst_QLatch::multipleWorkersAndWaiters_data()
{
    multipleWorkersSingleWaiter_data();
}

void tst_QLatch::multipleWorkersAndWaiters()
{
    QFETCH(int, count);
    for (int i = 0; i < Repeats; ++i) {
        Q_ASSERT(qint64(count) * MaxThreads <= std::numeric_limits<int>::max());
        std::array<int, MaxThreads> pendingValues;
        pendingValues.fill(0xfefefefe);

        QLatch latch(MaxThreads * count);
        std::unique_ptr<JThread> workers[MaxThreads];
        std::unique_ptr<JThread> waiters[MaxThreads];
        auto waiterCode = [&](int *ptr) {
            if (i > MaxThreads / 2)
                QTest::qSleep((i * 2 / MaxThreads) * 1ms);
            latch.wait();
            *ptr = latch.pending();
        };
        auto workerCode = [&]() { latch.countDown(count); };

        if ((i % 4) == 0) {
            // start waiters first
            for (int i = 0; i < MaxThreads; ++i)
                waiters[i].reset(new JThread(waiterCode, &pendingValues[i]));
            for (int i = 0; i < MaxThreads; ++i)
                workers[i].reset(new JThread(workerCode));
        } else if ((i % 4) == 1) {
            // start workers first
            for (int i = 0; i < MaxThreads; ++i)
                workers[i].reset(new JThread(workerCode));
            for (int i = 0; i < MaxThreads; ++i)
                waiters[i].reset(new JThread(waiterCode, &pendingValues[i]));
        } else if ((i % 4) == 2) {
            // interleave, with workers first
            for (int i = 0; i < MaxThreads; ++i) {
                workers[i].reset(new JThread(workerCode));
                waiters[i].reset(new JThread(waiterCode, &pendingValues[i]));
            }
        } else {
            // interleave, with waiters first
            for (int i = 0; i < MaxThreads; ++i) {
                waiters[i].reset(new JThread(waiterCode, &pendingValues[i]));
                workers[i].reset(new JThread(workerCode));
            }
        }

        // wait for one waiter
        int id = i % MaxThreads;
        waiters[id]->wait();

        QCOMPARE(pendingValues[id], 0);
        QCOMPARE(latch.pending(), 0);

        // wait for all waiters to finish
        for (auto &waiter : waiters)
            waiter->wait();
        QCOMPARE(latch.pending(), 0);
        QCOMPARE(pendingValues, decltype(pendingValues){});
    }
}

QTEST_MAIN(tst_QLatch)

#include "tst_qlatch.moc"
