// Copyright (C) 2025 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <private/qatomicwait_p.h>

#include <QRandomGenerator>
#include <QSemaphore>
#include <QThread>

#include <QtTest>

#include <condition_variable>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;
using namespace QtFallbackAtomicWait;

constexpr int Repeats = 9;
constexpr int MaxThreads = 8;
constexpr auto Delay = 10ms;

// moc can expand this
#define DEF_TEST(name)                          \
    void name ## _int() { name<int>(); }        \
    void name ## _qint64() { name<qint64>(); }  \
    void name ## _qint8() { name<qint8>(); }    \
    void name ## _voidp() { name<void *>(); }   \
    /* */

class tst_QAtomicWait : public QObject
{
    Q_OBJECT
private slots:
    DEF_TEST(noWaiterWake)
    DEF_TEST(failedWait)
    DEF_TEST(singleWaiterWakeOne)
    DEF_TEST(singleWaiterWakeAll)
    DEF_TEST(multiWaiterWakeAll)
    DEF_TEST(multiWaiterWakeSequentially)
    DEF_TEST(aliasingTest)

private:
    template <typename T> void noWaiterWake();
    template <typename T> void failedWait();
    template <typename T> void singleWaiterWakeOne();
    template <typename T> void singleWaiterWakeAll();
    template <typename T> void multiWaiterWakeAll();
    template <typename T> void multiWaiterWakeSequentially();
    template <typename T> void aliasingTest();
};

class SimpleLatch
{
    std::mutex mutex;
    int counter = 0;
    std::condition_variable cond;

public:
    SimpleLatch(int n) : counter(n) {}

    void countDown(int n = 1)
    {
        std::unique_lock l(mutex);
        counter -= n;
        if (counter == 0)
            cond.notify_all();
    }

    void wait()
    {
        std::unique_lock l(mutex);
        if (counter)
            cond.wait(l);
    }
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
        if (thr.joinable())
            thr.join();
    }

    void wait() { thr.join(); }
};

template <typename T> static T otherValue(int i = 1)
{
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T>(otherValue<quintptr>(i));
    } else if constexpr (sizeof(T) > sizeof(int)) {
        return (T(i) << 32) + ~T(i);
    } else {
        return T(i);
    }
}

template <typename T> static T badValue()
{
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T *>(0xfefefefe);
    } else {
        return T(0xfefefefe);
    }
}

template <typename T> void spinlockWait(std::atomic<T> &a)
{
    while (a.load(std::memory_order_relaxed) == T{}) {
        qYieldCpu();
    }
}

template <typename T> void tst_QAtomicWait::noWaiterWake()
{
    // Verify that there's no side-effect when there are no waiters. These
    // tests should do absolutely nothing (not even call the out-of-line
    // function).
    std::atomic<T> a{};
    q20::atomic_notify_one(&a);
    q20::atomic_notify_all(&a);
}

template<typename T>
void tst_QAtomicWait::failedWait()
{
    // Verify that we don't go into wait if the value in the atomic is
    // different from the value we expected to wait on.

    std::atomic<T> a{};
    q20::atomic_wait_explicit(&a, otherValue<T>(), std::memory_order_relaxed);

    a.store(otherValue<T>(), std::memory_order_relaxed);
    q20::atomic_wait_explicit(&a, {}, std::memory_order_relaxed);
}

template <typename T, void (*wakeFn)(std::atomic<T> *a)> static void singleWaiterWake()
{
    std::atomic<T> a{};

    for (int repetition = 0; repetition < Repeats; ++repetition) {
        SimpleLatch latch(1);
        a.store({}, std::memory_order_relaxed);

        JThread thr([&] {
            latch.countDown();
            QTest::qSleep(Delay);
            a.store(otherValue<T>());
            wakeFn(&a);
        });

        // wait for the thread to start
        latch.wait();

        // wait for the atomic to change
        // Note: it may have already changed if the qSleep() above has already
        // finished.
        q20::atomic_wait_explicit(&a, {}, std::memory_order_relaxed);

        QCOMPARE(a.load(), otherValue<T>());
    }
}

template <typename T> void tst_QAtomicWait::singleWaiterWakeOne()
{
    singleWaiterWake<T, q20::atomic_notify_one<T>>();
}

template <typename T> void tst_QAtomicWait::singleWaiterWakeAll()
{
    singleWaiterWake<T, q20::atomic_notify_all<T>>();
}

template <typename T, typename Wake> void multiWaiterWake(Wake &&wakeFn)
{
    std::atomic<T> a{};

    for (int repetition = 0; repetition < Repeats; ++repetition) {
        std::array<T, MaxThreads> loadedValues;
        loadedValues.fill(badValue<T>());
        a.store({}, std::memory_order_relaxed);

        {
            std::unique_ptr<JThread> threads[MaxThreads];
            SimpleLatch latch(MaxThreads);
            auto l = [&](T *loadedValue) {
                latch.countDown();
                q20::atomic_wait_explicit(&a, {}, std::memory_order_relaxed);
                *loadedValue = a.load(std::memory_order_relaxed);
            };

            for (int i = 0; i < MaxThreads; ++i)
                threads[i] = std::make_unique<JThread>(l, &loadedValues[i]);

            // wait for the threads to start
            latch.wait();

            QTest::qSleep(Delay);
            a.store(otherValue<T>());
            wakeFn(a);

            // wait for the threads to finish
        }

        std::array<T, MaxThreads> expected;
        expected.fill(otherValue<T>());
        QCOMPARE(loadedValues, expected);
    }
}

template <typename T> void tst_QAtomicWait::multiWaiterWakeAll()
{
    multiWaiterWake<T>([](std::atomic<T> &a) {
        q20::atomic_notify_all(&a);
    });
}

template <typename T> void tst_QAtomicWait::multiWaiterWakeSequentially()
{
    multiWaiterWake<T>([](std::atomic<T> &a) {
        auto delay = Delay;
        for (int i = 0; i < MaxThreads; ++i) {
            q20::atomic_notify_one(&a);
            delay /= 4;
            if (delay > 0ms)
                QTest::qSleep(delay);
        }
    });
}

template <typename T> void tst_QAtomicWait::aliasingTest()
{
    // There is a limited number of locks available, so let's try atomics whose
    // addresses are very similar. We can't ask for high alignment values on
    // the stack in all platforms, so we just pad the structure to 256 bytes.
    struct Atomics {
        std::atomic<T> a;
        T loadedValue;
        void reset()
        {
            a.store({}, std::memory_order_relaxed);
            loadedValue = badValue<T>();
        }
    };
    struct Padded : Atomics {
        char padding[256 - sizeof(Atomics)];
    };
    static_assert(sizeof(Padded) == 256);
    Padded atomics[MaxThreads]{};

    for (int repetition = 0; repetition < Repeats; ++repetition) {
        for (Atomics &s : atomics)
            s.reset();

        std::array<int, MaxThreads> order;
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), *QRandomGenerator::global());

        std::unique_ptr<JThread> threads[MaxThreads];
        SimpleLatch latch(MaxThreads);
        auto l = [&](Atomics *atomic) {
            latch.countDown();
            q20::atomic_wait_explicit(&atomic->a, {}, std::memory_order_relaxed);
            atomic->loadedValue = atomic->a.load(std::memory_order_relaxed);
        };
        for (int i = 0; i < MaxThreads; ++i)
            threads[i] = std::make_unique<JThread>(l, &atomics[order[i]]);

        // wait for the threads to start
        latch.wait();

        QTest::qSleep(Delay);

        std::array<T, MaxThreads> expected, actual;
        actual.fill({});
        expected.fill({});
        for (int i = 0; i < MaxThreads; ++i) {
            expected[i] = otherValue<T>(i + MaxThreads);
            Atomics &atomic = atomics[order[i]];
            atomic.a.store(expected[i], std::memory_order_relaxed);
            q20::atomic_notify_one(&atomic.a);

            // wait for this thread
            threads[i]->wait();

            actual[i] = atomic.loadedValue;
            QCOMPARE(actual, expected);
        }
    }
}

QTEST_MAIN(tst_QAtomicWait)

#include "tst_qatomicwait.moc"

