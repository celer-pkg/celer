// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WINRTCOMMON_H
#define WINRTCOMMON_H

#include <QtCore/QLoggingCategory>

namespace ABI {
    namespace Windows {
        namespace Foundation {
            struct DateTime;
        }
    }
}

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcWinRtSensors)

quint64 dateTimeToMsSinceEpoch(const ABI::Windows::Foundation::DateTime &dateTime);

QT_END_NAMESPACE

#endif // WINRTCOMMON_H

