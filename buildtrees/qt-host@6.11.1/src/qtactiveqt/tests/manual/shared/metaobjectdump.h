// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef METAOBJECTDUMP_H
#define METAOBJECTDUMP_H

#include <QtGlobal>

QT_FORWARD_DECLARE_STRUCT(QMetaObject);
QT_FORWARD_DECLARE_CLASS(QTextStream);

QTextStream &operator<<(QTextStream &str, const QMetaObject &o);

#endif
