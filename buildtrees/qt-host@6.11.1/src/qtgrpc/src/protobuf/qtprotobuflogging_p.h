// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPROTOBUFLOGGING_P_H
#define QTPROTOBUFLOGGING_P_H

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

#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE
QT_DECLARE_EXPORTED_QT_LOGGING_CATEGORY(Protobuf, Q_PROTOBUF_EXPORT)
QT_END_NAMESPACE

#define qProtoDebug(...) qCDebug(Protobuf, __VA_ARGS__)
#define qProtoInfo(...) qCInfo(Protobuf, __VA_ARGS__)
#define qProtoWarning(...) qCWarning(Protobuf, __VA_ARGS__)
#define qProtoCritical(...) qCCritical(Protobuf, __VA_ARGS__)

#endif // QTPROTOBUFLOGGING_P_H
