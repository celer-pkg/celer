// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPROTOBUFDEFS_P_H
#define QTPROTOBUFDEFS_P_H

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

QT_BEGIN_NAMESPACE

constexpr int ProtobufFieldNumMin = 1;
constexpr int ProtobufFieldNumMax = 536870911;
constexpr int ProtobufFieldMaxCount = ProtobufFieldNumMax - ProtobufFieldNumMin + 1;

QT_END_NAMESPACE

#endif // QTPROTOBUFDEFS_P_H
