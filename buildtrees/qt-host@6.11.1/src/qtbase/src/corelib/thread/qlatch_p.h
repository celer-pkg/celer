// Copyright (C) 2024 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QLATCH_P_H
#define QLATCH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an implementation
// detail. This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#include <QtCore/qbasicatomic.h>
#include <QtCore/qtsan_impl.h>

#include <private/qglobal_p.h>

#include <limits>

QT_BEGIN_NAMESPACE

class QLatch
{
public:
    constexpr explicit QLatch(int expected) noexcept
        : counter(expected | NoWaiters)
    {}

    int pending() const noexcept
    {
        return (counter.loadAcquire() & CounterMask);
    }

    void countDown(int n = 1) noexcept
    {
        QtTsan::latchCountDown(&counter);
        if (counter.fetchAndSubRelease(n) == n) // addAndFetch(n) == 0
            wakeUp();
    }

    bool tryWait() const noexcept
    {
        if (pending() != 0)
            return false;
        QtTsan::latchWait(&counter);
        return true;
    }

    void wait() noexcept // not const
    {
        if (int current = counter.loadAcquire(); (current & CounterMask) != 0) {
            waitInternal(current);
            QtTsan::latchWait(&counter);
        }
    }

    void arriveAndWait(int n = 1) noexcept
    {
        countDown(n);
        wait();
    }

    // API compatible with C++20:
    static constexpr int max() noexcept { return std::numeric_limits<int>::max(); }
    void count_down(int n = 1) noexcept { countDown(n); }
    bool try_wait() const noexcept      { return tryWait(); }
    void arrive_and_wait(int n = 1) noexcept { arriveAndWait(n); }

private:
    static constexpr int NoWaitersBit = 31;
    static constexpr int NoWaiters = 1 << NoWaitersBit;
    static constexpr int CounterMask = ~NoWaiters;
    QBasicAtomicInt counter;

    Q_DISABLE_COPY_MOVE(QLatch)

#ifdef QATOMICWAIT_USE_FALLBACK
#  define Q_LATCH_EXPORT /* being linked into the unit test */
#else
#  define Q_LATCH_EXPORT Q_CORE_EXPORT
#endif

    void Q_LATCH_EXPORT waitInternal(int current) noexcept;
    void Q_LATCH_EXPORT wakeUp() noexcept;

#undef Q_LATCH_EXPORT
};

QT_END_NAMESPACE

#endif // QLATCH_P_H
