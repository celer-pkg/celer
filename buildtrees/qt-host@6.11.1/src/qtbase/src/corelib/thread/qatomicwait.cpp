// Copyright (C) 2025 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qatomicwait_p.h"

#include <qendian.h>
#include "qfutex_p.h"
#include "qwaitcondition_p.h"

#include <array>
#include <condition_variable>
#include <mutex>

QT_BEGIN_NAMESPACE

/*
 * Implementation details:
 *
 * Normally, a call from atomic_notify_one() or atomic_notify_all() corresponds
 * to a call to cond.notify_one() or cond.notify_all(). This simple
 * implementation would require that we keep a distinct wait condition variable
 * per atomic variable, of which there could be an arbitrary number.
 *
 * Instead, we have a limited set of std::condition_variable, which are
 * selected based on the address of the variable being waited/notified on.
 * Because multiple, distinct variables could be sharing the same condition
 * variable, we need to perform a cond.notify_all() call in case the lock is
 * contended by waiters to more than one address, as we can't be sure which one
 * would be woken up by the cond.notify_one() call.
 *
 * We recover some of the performance of notifying a single waker by also
 * storing the address of the variable that was being waited on. If it matches
 * the address of the variable being notified, we can perform a notify_one()
 * call. This also allows us to avoid any system call in case no waiter has yet
 * joined the queue. In case of contention, we store a sentinel address
 * instead, indicating there are waiters to multiple variables. The last waiter
 * to leave a wait queue is responsible for resetting the watched address back
 * to nullptr.
 *
 * Performance details:
 *
 * This implementation is designed for systems where neither the Standard
 * Library's own std::atomic_wait nor operating system futexes are available
 * (read: usually, not the systems Qt is used most frequently on). Therefore,
 * it's designed for simplicity, not performance. Simplifications applied
 * include:
 * - the total number of possible condition variables
 * - the hashing algorithm to select a condition variable (simple XOR)
 * - no attempt to wait for a variable change before calling cond.wait()
 *   (no spinning, no HW-assisted wait)
 *
 * Other limitations:
 *
 * We only support 8-, 16-, 32- and 64-bit variables and we require bit-exact
 * comparisons for all bits. This means we do not support objects with padding
 * bits or without unique representations.
 */

namespace {
struct QAtomicWaitLocks
{
    // Sentinel address used to indicate this Lock is being used by waiters to
    // multiple addresses, implying we must make a notify_all() call.
    static void *contendedWatchAddress() { return reinterpret_cast<void *>(-1); }

    struct Lock {
        alignas(QtPrivate::IdealMutexAlignment) std::mutex mutex;
        alignas(QtPrivate::IdealMutexAlignment) std::condition_variable cond;

        // Can assume values:
        // - nullptr: no waiter is waiting
        // - contendedWatchAddress(): waiters to distinct addresses
        // - any other value: all waiters are waiting on the same address
        const void *watchedAddress = nullptr;
        qsizetype watcherCount = 0;
    };

    static constexpr int LockCount = 16;
    std::array<Lock, LockCount> locks;

    int indexFor(const void *ptr)
    {
        static_assert((LockCount & (LockCount - 1)) == 0, "LockCount is not a power of two");

        quintptr value = quintptr(ptr) / sizeof(int);
        quintptr idx = value % LockCount;
#ifndef QATOMICWAIT_USE_FALLBACK
        // XOR some higher bits too to avoid hashing collisions
        // (we don't do it in the unit test because we *want* those collisions)
        idx ^= (value / LockCount) % LockCount;
        idx ^= (value / LockCount / LockCount) % LockCount;
#endif
        return int(idx);
    }

    Lock &lockFor(const void *ptr) { return locks[indexFor(ptr)]; }
};
} // unnamed namespace

static inline void checkFutexUse()
{
#if !defined(QATOMICWAIT_USE_FALLBACK)
    if (QtFutex::futexAvailable()) {
        // This will disable the code and data on systems where futexes are
        // always available (currently: FreeBSD, Linux, Windows).
        qFatal("Implementation should have used futex!");
    }
#endif
}

static QAtomicWaitLocks &atomicLocks() noexcept
{
    checkFutexUse();

    static QAtomicWaitLocks global {};
    return global;
}

template <typename T> static bool isEqual(const void *address, const void *old)
{
    auto atomic = static_cast<const std::atomic<T> *>(address);
    auto expected = static_cast<const T *>(old);
    return atomic->load(std::memory_order_relaxed) == *expected;
}

static bool isEqual(const void *address, const void *old, qsizetype size) noexcept
{
    switch (size) {
    case 1: return isEqual<quint8 >(address, old);
    case 2: return isEqual<quint16>(address, old);
    case 4: return isEqual<quint32>(address, old);
    case 8: return isEqual<quint64>(address, old);
    }
    Q_UNREACHABLE_RETURN(false);
}

void QtPrivate::_q_atomicWait(const void *address, const void *old, qsizetype size) noexcept
{
    auto &locker = atomicLocks().lockFor(address);

    // NOT noexcept; we'll terminate if locking the mutex fails
    std::unique_lock lock(locker.mutex);

    // is the value still current?
    if (!isEqual(address, old, size))
        return;                     // no, failed to wait

    if (locker.watchedAddress && locker.watchedAddress != address)
        locker.watchedAddress = QAtomicWaitLocks::contendedWatchAddress();
    else
        locker.watchedAddress = address;
    ++locker.watcherCount;

    do {
        locker.cond.wait(lock);
    } while (isEqual(address, old, size));

    if (--locker.watcherCount == 0)
        locker.watchedAddress = nullptr;
}

void QtPrivate::_q_atomicWake(void *address, WakeMode mode) noexcept
{
    auto &locker = atomicLocks().lockFor(address);

    // NOT noexcept; we'll terminate if locking the mutex fails
    std::unique_lock lock(locker.mutex);

    // can we wake just one?
    if (mode == WakeMode::One && locker.watchedAddress == address)
        locker.cond.notify_one();
    else if (locker.watchedAddress != nullptr)
        locker.cond.notify_all();
    else
        qt_noop();          // no one was waiting
}

QT_END_NAMESPACE
