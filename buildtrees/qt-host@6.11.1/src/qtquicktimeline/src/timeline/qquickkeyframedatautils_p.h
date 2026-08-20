// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKKEYFRAMEDATAUTILS_H
#define QQUICKKEYFRAMEDATAUTILS_H

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

#include <QtCore/QCborStreamWriter>
#include <QtCore/QVariant>
#include <QtCore/QMetaType>

#include <QtCore/QRect>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>
#include <QtGui/QQuaternion>
#include <QtGui/QColor>
#include <QtCore/private/qglobal_p.h>

// This file contains useful methods to write keyframes
// CBOR binary files and understand the format.

void writeKeyframesHeader(QCborStreamWriter &writer, QMetaType::Type type, int version = 1)
{
    // Root array
    writer.startArray();
    // header name
    writer.append("QTimelineKeyframes");
    // file version
    writer.append(version);
    // property type
    writer.append(type);
}

// Write QVariant value into CBOR in correct type.
void writeValue(QCborStreamWriter &writer, const QVariant &value)
{
    const QMetaType type = value.metaType();
    switch (type.id()) {
    case QMetaType::Bool: {
        bool b = value.toBool();
        writer.append(b);
        break;
    }
    case QMetaType::Int: {
        int i = value.toInt();
        writer.append(i);
        break;
    }
    case QMetaType::Float: {
        float f = value.toFloat();
        writer.append(f);
        break;
    }
    case QMetaType::Double: {
        double d = value.toDouble();
        writer.append(d);
        break;
    }
    case QMetaType::QVector2D: {
        QVector2D v = value.value<QVector2D>();
        writer.append(v.x());
        writer.append(v.y());
        break;
    }
    case QMetaType::QVector3D: {
        QVector3D v = value.value<QVector3D>();
        writer.append(v.x());
        writer.append(v.y());
        writer.append(v.z());
        break;
    }
    case QMetaType::QVector4D: {
        QVector4D v = value.value<QVector4D>();
        writer.append(v.x());
        writer.append(v.y());
        writer.append(v.z());
        writer.append(v.w());
        break;
    }
    case QMetaType::QQuaternion: {
        QQuaternion q = value.value<QQuaternion>();
        writer.append(q.scalar());
        writer.append(q.x());
        writer.append(q.y());
        writer.append(q.z());
        break;
    }
    case QMetaType::QColor: {
        QColor c = value.value<QColor>();
        writer.append(c.red());
        writer.append(c.green());
        writer.append(c.blue());
        writer.append(c.alpha());
        break;
    }
    case QMetaType::QRect: {
        QRect r = value.value<QRect>();
        writer.append(r.x());
        writer.append(r.y());
        writer.append(r.width());
        writer.append(r.height());
        break;
    }
    default: {
        qDebug() << "Not able to add:" << value << "of type:" << type.name();
        qDebug() << "Please add support for this type into generator.";
        break;
    }
    }
}

#endif // QQUICKKEYFRAMEDATAUTILS_H
