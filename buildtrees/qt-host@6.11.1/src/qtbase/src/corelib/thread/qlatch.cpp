// Copyright (C) 2025 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qlatch_p.h"

#include "qatomicwait_p.h"
#include "qfutex_p.h"

QT_BEGIN_NAMESPACE

using namespace QtFutex;

#if defined(QATOMICWAIT_USE_FALLBACK)
static constexpr bool ForcedFallbackAtomicWait = true;
namespace atomicwait = QtFallbackAtomicWait;
#else
static constexpr bool ForcedFallbackAtomicWait = false;
namespace atomicwait = q20;
#endif

/*!
    \class QLatch
    \internal

    Implements the same API as \c std::latch (C++20), allowing a single
    synchronization between threads.

    \section2 Typical uses
    \section3 Waiting for threaded work to finish

    For this use-case, one or more threads perform some work, which needs to
    finish before the caller thread can proceed. For this, each worker thread
    calls countDown() once they have finished their work, while the caller
    thread suspends execution by calling wait().

    The operation is best seen in:
    \code
        QLatch latch(segments);
        int y = 0;
        for (int i = 0; i < segments; ++i) {
            int yn = (data->height - y) / (segments - i);
            threadPool->start([&, y, yn]() {
                convertSegment(y, y + yn);
                latch.countDown();
            });
            y += yn;
        }
        latch.wait();
    \endcode

    Or, for a single thread:
    \code
        QLatch latch(1);
        QMetaObject::invokeMethod(object, [&]() {
            doSomething();
            latch.countDown();
        }, Qt::QueuedConnection);
        latch.wait();
    \endcode

    In fact, the above is exactly what Qt::BlockingQueued connection does.

    \section3 Synchronizing execution

    For this use-case, multiple threads must reach a particular state before
    any of them may proceed. In this case, all of them call arriveAndWait(),
    causing all but the last one of them to suspend execution until that last
    one also arrives.

    \code
        QLatch latch(n);
        for (int i = 0; i < n; ++i) {
            threadPool->start([] {
                latch.arriveAndWait();
                doStressfulWork();
            });
        }
    \endcode

    \section2 Differences from \c std::latch

    \list
      \li Uses \c{int} in the API instead of \c{ptrdiff_t} (note that the max()
          is the same as libstdc++'s on Linux).
      \li count_down() is not \c{const} (libstdc++ implementation is).
    \endlist

    \omit
    \section2 Implementation details

    countDown() must call wakeUp() if the latch counter reaches zero and there
    are threads waiting to be woken up. Or, conversely, countDown() needs to do
    nothing after decrementing if the latch counter is still non-zero or there
    are no waiters. Therefore, we choose the bits so that a non-zero
    \c{counter} member implies no action required.

    \endomit
*/

/*!
    \fn QLatch::QLatch(int expected) noexcept

    Initializes the QLatch to indicate that countDown() will be called \a
    expected times. You probably want to pass a value greater than zero.
*/

/*!
    \fn int QLatch::pending() noexcept
    \internal

    Returns the counter.

    Don't use; for the unit test only.
*/

/*!
    \fn void QLatch::countDown(int n) noexcept
    \fn void QLatch::count_down(int n) noexcept

    Decrements the internal counter by \a n. If the internal counter drops to
    zero after this operation, any threads currently waiting will be woken up.
    If \a n is greater than the value of the internal counter or is negative,
    the behavior is undefined.

    This function does not block and may be used to notify waiters that this
    thread has reached a particular point and they may proceed. To synchronize
    all threads so they all resume work at the same time, use arriveAndWait().

    This function implements release memory ordering.

    \sa arriveAndWait(), wait()
*/

/*!
    \fn bool QLatch::tryWait() const noexcept
    \fn void QLatch::try_wait() const noexcept

    Returns true if the internal counter in this latch has dropped to zero,
    false otherwise. This function does not block.

    This function implements acquire memory ordering.

    \sa wait(), countDown()
*/

/*!
    \fn void QLatch::wait() noexcept

    Waits for the internal counter in this latch to drop to zero.

    This function implements acquire memory ordering.

    \sa tryWait(), arriveAndWait(), countDown()
*/

/*!
    \fn void QLatch::arriveAndWait(int n) noexcept
    \fn void QLatch::arrive_and_wait(int n) noexcept

    This function decrements the internal counter by \a n. If the counter
    remains non-zero after this operation, it suspends the current thread until
    it does become zero. Otherwise it wakes all other current waiters.

    This function is useful to synchronize multiple threads so they may start
    some execution at (nearly) exactly the same time.

    This function is exactly equivalent to:
    \code
       countDown(n);
       wait();
    \endcode

    This function implements acquire-and-release memory ordering.

    \sa countDown(), wait()
*/

/*!
    \fn int QLatch::max() noexcept

    Returns the maximum number that can be passed to the constructor.
*/

void QLatch::waitInternal(int current) noexcept
{
    // mark that there is a waiter -> clear the bit that there are no waiters
    if (current & NoWaiters) {
#if __has_builtin(__atomic_and_fetch)
        // Modern GCC and Clang are able to generate loop-free code for this
        // operation on x86-64, ARMv8.1 and RISC-V.
        if (__atomic_and_fetch(reinterpret_cast<int *>(&counter._q_value), ~NoWaiters,
                               int(std::memory_order_relaxed)) == 0)
            return;
#else
        // Do it in two steps, which is usually better than a compare_exchange
        // loop. This is not exactly the same as above (it's not atomic!) but
        // is correct for our purposes because the counter never changes from 0
        // once it reaches that.
        counter.fetchAndAndRelaxed(~NoWaiters);
        if (counter.loadRelaxed() == 0)
            return;     // no need to wait!
#endif
    }
    current &= ~NoWaiters;

    auto waitLoop = [&](auto waiter) {
        do {
            waiter(current);
        } while ((current = counter.loadAcquire()) != 0);
    };

    if (futexAvailable() && !ForcedFallbackAtomicWait)
        waitLoop([&](int current) { futexWait(counter, current); });
    else
        waitLoop([&](int current) {
            atomicwait::atomic_wait_explicit(&counter._q_value, current, std::memory_order_relaxed);
        });
}

void QLatch::wakeUp() noexcept
{
    if (futexAvailable() && !ForcedFallbackAtomicWait)
        futexWakeAll(counter);
    else
        atomicwait::atomic_notify_all(&counter._q_value);
}

QT_END_NAMESPACE
