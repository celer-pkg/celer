// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHUTILS_WINRT_P_H
#define QBLUETOOTHUTILS_WINRT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Workaround for Windows SDK bug.
// See https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/issues/47
 #include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <QtCore/QtGlobal>
#include <QtCore/private/qglobal_p.h>

#include <wrl/client.h>

namespace ABI {
    namespace Windows {
        namespace Storage {
            namespace Streams {
                struct IBuffer;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

using NativeBuffer = ABI::Windows::Storage::Streams::IBuffer;
QByteArray byteArrayFromBuffer(const Microsoft::WRL::ComPtr<NativeBuffer> &buffer,
                               bool isWCharString = false);

// The calls to Co(Un)init must be balanced
void mainThreadCoInit(void* caller);
void mainThreadCoUninit(void* caller);

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_WINRT_P_H
