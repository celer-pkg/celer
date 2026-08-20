// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdevicewatcher_winrt_p.h"

#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Enumeration;

QT_BEGIN_NAMESPACE

QBluetoothDeviceWatcherWinRT::QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector)
    : m_id(id),
      m_watcher(DeviceInformation::CreateWatcher(selector))
{
    qRegisterMetaType<winrt::hstring>("winrt::hstring");
}

QBluetoothDeviceWatcherWinRT::QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector,
                                winrt::Windows::Devices::Enumeration::DeviceInformationKind kind)
    : m_id(id)
{
    qRegisterMetaType<winrt::hstring>("winrt::hstring");
    const winrt::param::iterable<winrt::hstring> extra {};
    m_watcher = DeviceInformation::CreateWatcher(selector, extra, kind);
}

QBluetoothDeviceWatcherWinRT::~QBluetoothDeviceWatcherWinRT()
{
    stop();
}

bool QBluetoothDeviceWatcherWinRT::init()
{
    if (!m_watcher) {
        qWarning("Windows failed to create an instance of DeviceWatcher. "
                 "Detection of Bluetooth devices might not work correctly.");
        return false;
    }
    return true;
}

void QBluetoothDeviceWatcherWinRT::start()
{
    if (m_watcher) {
        subscribeToEvents();
        m_watcher.Start();
    }
}

void QBluetoothDeviceWatcherWinRT::stop()
{
    if (m_watcher && canStop()) {
        unsubscribeFromEvents();
        m_watcher.Stop();
    }
}

void QBluetoothDeviceWatcherWinRT::subscribeToEvents()
{
    Q_ASSERT(m_watcher.Status() == DeviceWatcherStatus::Created);
    // The callbacks are triggered from separate threads. So we capture
    // thisPtr to make sure that the object is valid.
    auto thisPtr = shared_from_this();
    m_addedToken = m_watcher.Added([thisPtr](DeviceWatcher, const DeviceInformation &info) {
        emit thisPtr->deviceAdded(info.Id(), thisPtr->m_id);
    });
    m_removedToken =
            m_watcher.Removed([thisPtr](DeviceWatcher, const DeviceInformationUpdate &upd) {
                emit thisPtr->deviceRemoved(upd.Id(), thisPtr->m_id);
            });
    m_updatedToken =
            m_watcher.Updated([thisPtr](DeviceWatcher, const DeviceInformationUpdate &upd) {
                emit thisPtr->deviceUpdated(upd.Id(), thisPtr->m_id);
            });
    // because of ambiguous declaration
    using WinRtInspectable = winrt::Windows::Foundation::IInspectable;
    m_enumerationToken =
            m_watcher.EnumerationCompleted([thisPtr](DeviceWatcher, const WinRtInspectable &) {
                emit thisPtr->enumerationCompleted(thisPtr->m_id);
            });
    m_stoppedToken = m_watcher.Stopped([thisPtr](DeviceWatcher, const WinRtInspectable &) {
        emit thisPtr->watcherStopped(thisPtr->m_id);
    });
}

void QBluetoothDeviceWatcherWinRT::unsubscribeFromEvents()
{
    m_watcher.Added(m_addedToken);
    m_watcher.Removed(m_removedToken);
    m_watcher.Updated(m_updatedToken);
    m_watcher.EnumerationCompleted(m_enumerationToken);
    m_watcher.Stopped(m_stoppedToken);
}

bool QBluetoothDeviceWatcherWinRT::canStop() const
{
    const auto status = m_watcher.Status();
    // Also 'Aborted', but calling Stop() there is a no-op
    return status == DeviceWatcherStatus::Started
            || status == DeviceWatcherStatus::EnumerationCompleted;
}

QT_END_NAMESPACE
