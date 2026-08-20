// Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef COMMON_H
#define COMMON_H

#include <qnamespace.h>

enum ReceivedFramesModelColumns {
    Number = 0,
    Timestamp,
    Flags,
    CanID,
    DLC,
    Data,
    Count
};

enum : int {
  ClipboardTextRole = Qt::UserRole + 1
};

#endif // COMMON_H
