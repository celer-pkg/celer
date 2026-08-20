// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESHAPEDATAUTILS_P_H
#define QQUICK3DPARTICLESHAPEDATAUTILS_P_H

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

#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <QtCore/QCborStreamReader>
#if QT_CONFIG(cborstreamwriter)
#include <QtCore/QCborStreamWriter>
#endif
#include <QtCore/QVariant>
#include <QtCore/QMetaType>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleShapeDataUtils
{
public:
    static QVariant readValue(QCborStreamReader &reader, QMetaType::Type type);
    static QString readString(QCborStreamReader &reader);
    static double readReal(QCborStreamReader &reader);
    static int readShapeHeader(QCborStreamReader &reader);
#if QT_CONFIG(cborstreamwriter)
    static void writeShapeHeader(QCborStreamWriter &writer, int version = 1);
    static void writeValue(QCborStreamWriter &writer, const QVariant &value);
#endif
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLESHAPEDATAUTILS_P_H
