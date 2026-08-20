// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QModelIndex>

// Helper class which can be used by tests for starting a task and
// waiting for its completion. It takes care of running an event
// loop while waiting, until finished() method is called (or the
// timeout is reached).
class WaitHelper : public QObject
{
    Q_OBJECT

public:
    WaitHelper() { m_promise.start(); }

    ~WaitHelper()
    {
        if (m_promise.future().isRunning())
            m_promise.finish();
    }

    /*
        Starts an event loop and waits until finish() method is called
        or the timeout is reached.
    */
    bool wait(int timeout = 30000)
    {
        if (m_promise.future().isFinished())
            return true;

        QFutureWatcher<void> watcher;
        QSignalSpy watcherSpy(&watcher, &QFutureWatcher<void>::finished);
        watcher.setFuture(m_promise.future());
        return watcherSpy.wait(timeout);
    }

protected:
    /*
        The derived classes need to call this method to stop waiting.
    */
    void finish() { m_promise.finish(); }

private:
    QPromise<void> m_promise;
};

namespace {

inline bool compareIndices(const QModelIndex &lhs, const QModelIndex &rhs)
{
    QModelIndex left = lhs;
    QModelIndex right = rhs;
    while (left.row() == right.row() && left.column() == right.column() && left.isValid() && right.isValid()) {
        left = left.parent();
        right = right.parent();
    }
    if (left.isValid() || right.isValid())
        return false;
    return true;
}

struct WaitForDataChanged : public WaitHelper
{
    WaitForDataChanged(const QAbstractItemModel *model, const QList<QModelIndex> &pending)
        : WaitHelper(), m_model(model), m_pending(pending)
    {
        connect(m_model, &QAbstractItemModel::dataChanged, this,
                [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                       const QList<int> &roles) {
                    Q_UNUSED(roles)

                    checkAndRemoveRange(topLeft, bottomRight);
                    if (m_pending.isEmpty())
                        finish();
                });
    }

    void checkAndRemoveRange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
    {
        QVERIFY(topLeft.parent() == bottomRight.parent());
        const auto isInRange = [topLeft, bottomRight] (const QModelIndex &pending) noexcept -> bool {
            if (pending.isValid()  && compareIndices(pending.parent(), topLeft.parent())) {
                const bool fitLeft = topLeft.column() <= pending.column();
                const bool fitRight = bottomRight.column() >= pending.column();
                const bool fitTop = topLeft.row() <= pending.row();
                const bool fitBottom = bottomRight.row() >= pending.row();
                if (fitLeft && fitRight && fitTop && fitBottom)
                    return true;
            }
            return false;
        };
        m_pending.erase(std::remove_if(m_pending.begin(), m_pending.end(), isInRange),
                        m_pending.end());
    }

private:
    const QAbstractItemModel *m_model = nullptr;
    QList<QModelIndex> m_pending;
};

} // namespace
