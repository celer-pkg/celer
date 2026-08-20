// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothutils_winrt_p.h"
#include <QtBluetooth/private/qtbluetoothglobal_p.h>
#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/QLoggingCategory>

#include <robuffer.h>
#include <wrl.h>
#include <winrt/windows.foundation.metadata.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace winrt::Windows::Foundation::Metadata;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QByteArray byteArrayFromBuffer(const ComPtr<NativeBuffer> &buffer, bool isWCharString)
{
    if (!buffer) {
        qErrnoWarning("nullptr passed to byteArrayFromBuffer");
        return QByteArray();
    }
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    HRESULT hr = buffer.As(&byteAccess);
    RETURN_IF_FAILED("Could not cast buffer", return QByteArray())
    char *data;
    hr = byteAccess->Buffer(reinterpret_cast<byte **>(&data));
    RETURN_IF_FAILED("Could not obtain buffer data", return QByteArray())
    UINT32 size;
    hr = buffer->get_Length(&size);
    RETURN_IF_FAILED("Could not obtain buffer size", return QByteArray())
    if (isWCharString) {
        QString valueString = QString::fromUtf16(reinterpret_cast<char16_t *>(data)).left(size / 2);
        return valueString.toUtf8();
    }
    return QByteArray(data, qint32(size));
}

static QSet<void*> successfulInits;
static QThread *mainThread = nullptr;

void mainThreadCoInit(void* caller)
{
    Q_ASSERT(caller);

    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        qCWarning(QT_BT_WINDOWS) << "Main thread COM init tried from another thread";
        return;
    }

    if (successfulInits.contains(caller)) {
        qCWarning(QT_BT_WINDOWS) << "Multiple COM inits by same object";
        return;
    }

    Q_ASSERT_X(!mainThread || mainThread == QThread::currentThread(),
               __FUNCTION__, "QCoreApplication's thread has changed!");

    // This executes in main thread which may run Gui => use apartment threaded
    if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
        qCWarning(QT_BT_WINDOWS) << "Unexpected COM initialization result";
        return;
    }
    successfulInits.insert(caller);
    if (!mainThread)
        mainThread = QThread::currentThread();
}

void mainThreadCoUninit(void* caller)
{
    Q_ASSERT(caller);

    if (!successfulInits.contains(caller)) {
        qCWarning(QT_BT_WINDOWS) << "COM uninitialization without initialization";
        return;
    }

    if (QThread::currentThread() != mainThread) {
        qCWarning(QT_BT_WINDOWS) << "Main thread COM uninit tried from another thread";
        return;
    }

    CoUninitialize();
    successfulInits.remove(caller);

}

QT_END_NAMESPACE
