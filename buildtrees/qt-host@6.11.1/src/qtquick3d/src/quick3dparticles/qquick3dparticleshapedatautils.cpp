// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleshapedatautils_p.h"
#include <QtCore/QRect>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>
#include <QtGui/QQuaternion>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

// This file contains useful methods to read & write shape
// CBOR binary files and understand the format.

// Read property 'type' value from CBOR and return it as QVariant.
QVariant QQuick3DParticleShapeDataUtils::readValue(QCborStreamReader &reader, QMetaType::Type type)
{
    switch (type) {
    case QMetaType::Bool: {
        bool b = reader.toBool();
        reader.next();
        return QVariant(b);
    }
    case QMetaType::Int: {
        int i = reader.toInteger();
        reader.next();
        return QVariant(i);
    }
    case QMetaType::Float: {
        float f = reader.toFloat();
        reader.next();
        return QVariant(f);
    }
    case QMetaType::Double: {
        double d = reader.toDouble();
        reader.next();
        return QVariant(d);
    }
    case QMetaType::QString: {
        QString s = reader.readString().data;
        return QVariant(s);
    }
    case QMetaType::QVector2D: {
        QVector2D v;
        v.setX(reader.toFloat());
        reader.next();
        v.setY(reader.toFloat());
        reader.next();
        return QVariant(v);
    }
    case QMetaType::QVector3D: {
        QVector3D v;
        v.setX(reader.toFloat());
        reader.next();
        v.setY(reader.toFloat());
        reader.next();
        v.setZ(reader.toFloat());
        reader.next();
        return QVariant(v);
    }
    case QMetaType::QVector4D: {
        QVector4D v;
        v.setX(reader.toFloat());
        reader.next();
        v.setY(reader.toFloat());
        reader.next();
        v.setZ(reader.toFloat());
        reader.next();
        v.setW(reader.toFloat());
        reader.next();
        return QVariant(v);
    }
    case QMetaType::QQuaternion: {
        QQuaternion q;
        q.setScalar(reader.toFloat());
        reader.next();
        q.setX(reader.toFloat());
        reader.next();
        q.setY(reader.toFloat());
        reader.next();
        q.setZ(reader.toFloat());
        reader.next();
        return QVariant(q);
    }
    case QMetaType::QColor: {
        QColor c;
        c.setRed(reader.toInteger());
        reader.next();
        c.setGreen(reader.toInteger());
        reader.next();
        c.setBlue(reader.toInteger());
        reader.next();
        c.setAlpha(reader.toInteger());
        reader.next();
        return QVariant(c);
    }
    case QMetaType::QRect: {
        QRect r;
        r.setX(reader.toInteger());
        reader.next();
        r.setY(reader.toInteger());
        reader.next();
        r.setWidth(reader.toInteger());
        reader.next();
        r.setHeight(reader.toInteger());
        reader.next();
        return QVariant(r);
    }
    default: {
        qWarning() << "Property type not handled:" << type;
    }
    }

    return QVariant();
}

// Read a string from CBOR.
QString QQuick3DParticleShapeDataUtils::readString(QCborStreamReader &reader)
{
    QString result;
    auto r = reader.readString();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readString();
    }

    if (r.status == QCborStreamReader::Error) {
        // handle error condition
        result.clear();
    }
    return result;
}

// Read a real (double or float) from CBOR.
double QQuick3DParticleShapeDataUtils::readReal(QCborStreamReader &reader)
{
    double result = 0.0;
    if (reader.isDouble()) {
        result = reader.toDouble();
        reader.next();
    } else if (reader.isFloat()) {
        result = reader.toFloat();
        reader.next();
    }
    return result;
}

// Read shape file header and return the version.
// If header is not valid, -1 is returned.
int QQuick3DParticleShapeDataUtils::readShapeHeader(QCborStreamReader &reader)
{
    int version = -1;
    if (reader.lastError() == QCborError::NoError && reader.isArray()) {
        // Start root array
        reader.enterContainer();
        if (reader.isString()) {
            QString header = readString(reader);
            if (header == QStringLiteral("QQ3D_SHAPE")) {
                if (reader.isInteger()) {
                    version = reader.toInteger();
                    reader.next();
                } else {
                    qWarning() << "Invalid shape version";
                }
            } else {
                qWarning() << "Invalid shape header";
            }
        } else {
            qWarning() << "Invalid shape container";
        }
    }
    return version;
}

#if QT_CONFIG(cborstreamwriter)
void QQuick3DParticleShapeDataUtils::writeShapeHeader(QCborStreamWriter &writer, int version)
{
    // Root array
    writer.startArray();
    // header name
    writer.append("QQ3D_SHAPE");
    // file version
    writer.append(version);
}

// Write QVariant value into CBOR in correct type.
void QQuick3DParticleShapeDataUtils::writeValue(QCborStreamWriter &writer, const QVariant &value)
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
#endif

QT_END_NAMESPACE
