// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btdelegates_p.h"

#if defined(Q_OS_MACOS)

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

DeviceInquiryDelegate::~DeviceInquiryDelegate()
{
}

PairingDelegate::~PairingDelegate()
{
}

SDPInquiryDelegate::~SDPInquiryDelegate()
{
}

ChannelDelegate::~ChannelDelegate()
{
}

ConnectionMonitor::~ConnectionMonitor()
{
}

SocketListener::~SocketListener()
{
}

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif
