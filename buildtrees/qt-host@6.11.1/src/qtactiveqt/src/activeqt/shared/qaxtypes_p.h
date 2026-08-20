// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXTYPES_H
#define QAXTYPES_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>

#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

#ifdef QAX_SERVER
#   define QVariantToVARIANTFunc QVariantToVARIANT_server
#   define VARIANTToQVariantFunc VARIANTToQVariant_server
#else
#   define QVariantToVARIANTFunc QVariantToVARIANT_container
#   define VARIANTToQVariantFunc VARIANTToQVariant_container
#endif

extern bool QVariantToVARIANTFunc(const QVariant &var, VARIANT &arg, const QByteArray &typeName = QByteArray(), bool out = false);
extern QVariant VARIANTToQVariantFunc(const VARIANT &arg, const QByteArray &typeName,
                                      int type = 0);

inline bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName = QByteArray(), bool out = false)
{
    return QVariantToVARIANTFunc(var, arg, typeName, out);
}

inline QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName,
                                  int type = 0)
{
    return VARIANTToQVariantFunc(arg, typeName, type);
}

#undef QVariantToVARIANTFunc
#undef VARIANTToQVariantFunc

QT_END_NAMESPACE

#endif // QAXTYPES_H
