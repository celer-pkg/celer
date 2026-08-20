// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DELEGATEMODELKINDS_H
#define DELEGATEMODELKINDS_H

#include <QtCore/qobject.h>
#include <QtQml/qqml.h>

namespace Model {
Q_NAMESPACE
QML_ELEMENT
enum Kind : qint8
{
    None = -1,
    Singular,
    List,
    Array,
    Object
};
Q_ENUM_NS(Kind)
}

namespace Delegate {
Q_NAMESPACE
QML_ELEMENT
enum Kind : qint8
{
    None = -1,
    Untyped,
    Typed
};
Q_ENUM_NS(Kind)
}

#endif // DELEGATEMODELKINDS_H
