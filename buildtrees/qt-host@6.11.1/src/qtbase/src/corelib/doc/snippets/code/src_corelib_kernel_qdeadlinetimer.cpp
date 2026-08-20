// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QDeadlineTimer>
#include <QMutex>

void waitForReadyRead(QDeadlineTimer &deadline);
bool readFromDevice(int msecs) { return false; }
void cleanup();

class MyDevice
{
public:
    bool waitForReadyRead(QDeadlineTimer &deadline)
    {
        return deadline.remainingTime() > 0;
    }
};

//! [0]
    void executeOperation(int msecs)
    {
        QDeadlineTimer deadline(msecs);
        do {
            if (readFromDevice(deadline.remainingTime()))
                break;
            waitForReadyRead(deadline);
        } while (!deadline.hasExpired());
    }
//! [0]


bool examples(MyDevice *device, QMutex &mutex)
{
    {
    //! [1]
        using namespace std::chrono;
        using namespace std::chrono_literals;

        QDeadlineTimer deadline(30s);
        device->waitForReadyRead(deadline);
        if (deadline.remainingTimeAsDuration() > 300ms)
            cleanup();
    //! [1]
    }

    {
    //! [2]
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto now = steady_clock::now();
        QDeadlineTimer deadline(now + 1s);
        Q_ASSERT(deadline == now + 1s);
    //! [2]
    }

    {
    //! [3]
        using namespace std::chrono_literals;
        QDeadlineTimer deadline(250ms);
    //! [3]
    }

    QDeadlineTimer deadline;

    {
    //! [4]
        using namespace std::chrono_literals;
        deadline.setRemainingTime(250ms);
    //! [4]
    }

    {
    //! [5]
        mutex.tryLock(deadline.remainingTime());
    //! [5]
    }

    {
    //! [6]
        qint64 realTimeLeft = deadline.deadline();
        if (realTimeLeft != (std::numeric_limits<qint64>::max)()) {
            realTimeLeft -= QDeadlineTimer::current().deadline();
            // or:
            //QElapsedTimer timer;
            //timer.start();
            //realTimeLeft -= timer.msecsSinceReference();
        }
    //! [6]
    }

    {
    //! [7]
        qint64 realTimeLeft = deadline.deadlineNSecs();
        if (realTimeLeft != std::numeric_limits<qint64>::max())
            realTimeLeft -= QDeadlineTimer::current().deadlineNSecs();
    //! [7]
    }

    QDeadlineTimer lhs(1);
    QDeadlineTimer rhs(2);

    {
    //! [8]
        return lhs.deadlineNSecs() == rhs.deadlineNSecs();
    //! [8]
    }

    {
    //! [9]
        return lhs.deadlineNSecs() != rhs.deadlineNSecs();
    //! [9]
    }

    {
    //! [10]
        return lhs.deadlineNSecs() < rhs.deadlineNSecs();
    //! [10]
    }

    {
    //! [11]
        return lhs.deadlineNSecs() <= rhs.deadlineNSecs();
    //! [11]
    }

    {
    //! [12]
        return lhs.deadlineNSecs() > rhs.deadlineNSecs();
    //! [12]
    }

    {
    //! [13]
        return lhs.deadlineNSecs() >= rhs.deadlineNSecs();
    //! [13]
    }
}
