// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobufQtGuiTypes/qtprotobufqtguitypes.h>

#include <QtProtobufQtGuiTypes/private/QtGui.qpb.h>

#include <QtProtobufQtCoreTypes/private/qtprotobufqttypescommon_p.h>

#include <QtGui/qrgb.h>
#include <QtGui/qrgba64.h>
#include <QtGui/qcolor.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qtransform.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qimage.h>
#include <QtGui/qimagereader.h>

#include <QtCore/qbuffer.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static QRgba64 convert(const QtProtobufPrivate::QtGui::QRgba64 &from)
{
    return QRgba64::fromRgba64(from.rgba64());
}

static QtProtobufPrivate::QtGui::QRgba64 convert(const QRgba64 &from)
{
    QtProtobufPrivate::QtGui::QRgba64 rgba64;
    rgba64.setRgba64((quint64)from);
    return rgba64;
}

static std::optional<QColor> convert(const QtProtobufPrivate::QtGui::QColor &from)
{
    if (from.hasRgba64()) {
        return QColor(convert(from.rgba64()));
    } else if (from.hasRgba()) {
        return QColor(qRgba(qRed(from.rgba()), qGreen(from.rgba()),
                            qBlue(from.rgba()), qAlpha(from.rgba())));
    }
    return std::nullopt;
}

static std::optional<QtProtobufPrivate::QtGui::QColor> convert(const QColor &from)
{
    if (!from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtGui::QRgba64 rgba64;
    rgba64.setRgba64((quint64)from.rgba64());

    QtProtobufPrivate::QtGui::QColor color;
    color.setRgba64(rgba64);
    return color;
}

static std::optional<QMatrix4x4> convert(const QtProtobufPrivate::QtGui::QMatrix4x4 &from)
{
    QtProtobuf::floatList list = from.m();
    if (list.size() == 16) {
        return QMatrix4x4(list[0], list[1], list[2], list[3],
                          list[4], list[5], list[6], list[7],
                          list[8], list[9], list[10], list[11],
                          list[12], list[13], list[14], list[15]);
    }
    qWarning() << "Input for QMatrix4x4 should provide 16 values, but size = "
               << list.size();

    return std::nullopt;
}

static QtProtobufPrivate::QtGui::QMatrix4x4 convert(const QMatrix4x4 &from)
{
    const float *matrixData = from.data();
    QtProtobufPrivate::QtGui::QMatrix4x4 matrix;
    // QMatrix4x4::data returned in column-major format
    matrix.setM({matrixData[0], matrixData[4], matrixData[8], matrixData[12],
                 matrixData[1], matrixData[5], matrixData[9], matrixData[13],
                 matrixData[2], matrixData[6], matrixData[10], matrixData[14],
                 matrixData[3], matrixData[7], matrixData[11], matrixData[15]});
    return matrix;
}

static QVector2D convert(const QtProtobufPrivate::QtGui::QVector2D &from)
{
    return QVector2D(from.xPos(), from.yPos());;
}

static std::optional<QtProtobufPrivate::QtGui::QVector2D> convert(const QVector2D &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtGui::QVector2D vector2D;
    vector2D.setXPos(from.x());
    vector2D.setYPos(from.y());
    return vector2D;
}

static QVector3D convert(const QtProtobufPrivate::QtGui::QVector3D &from)
{
    return QVector3D(from.xPos(), from.yPos(), from.zPos());
}

static std::optional<QtProtobufPrivate::QtGui::QVector3D> convert(const QVector3D &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtGui::QVector3D vector3D;
    vector3D.setXPos(from.x());
    vector3D.setYPos(from.y());
    vector3D.setZPos(from.z());
    return vector3D;
}

static QVector4D convert(const QtProtobufPrivate::QtGui::QVector4D &from)
{
    return QVector4D(from.xPos(), from.yPos(), from.zPos(), from.wPos());
}

static std::optional<QtProtobufPrivate::QtGui::QVector4D> convert(const QVector4D &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtGui::QVector4D vector4D;
    vector4D.setXPos(from.x());
    vector4D.setYPos(from.y());
    vector4D.setZPos(from.z());
    vector4D.setWPos(from.w());
    return vector4D;
}

static std::optional<QTransform> convert(const QtProtobufPrivate::QtGui::QTransform &from)
{
    QtProtobuf::doubleList list = from.m();
    if (list.size() == 9) {
        return QTransform(list[0], list[1], list[2],
                list[3], list[4], list[5],
                list[6], list[7], list[8]);
    }
    qWarning() << "Input list for QTransform should provide 9 members. But size = "
               << list.size();
    return std::nullopt;
}

static QtProtobufPrivate::QtGui::QTransform convert(const QTransform &from)
{
    QtProtobufPrivate::QtGui::QTransform transform;
    transform.setM({from.m11(), from.m12(), from.m13(),
                    from.m21(), from.m22(), from.m23(),
                    from.m31(), from.m32(), from.m33()});
    return transform;
}

static QQuaternion convert(const QtProtobufPrivate::QtGui::QQuaternion &from)
{
    return QQuaternion(from.scalar(), from.x(), from.y(), from.z());
}

static std::optional<QtProtobufPrivate::QtGui::QQuaternion> convert(const QQuaternion &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtGui::QQuaternion quaternion;
    quaternion.setScalar(from.scalar());
    quaternion.setX(from.x());
    quaternion.setY(from.y());
    quaternion.setZ(from.z());
    return quaternion;
}

static QImage convert(const QtProtobufPrivate::QtGui::QImage &from)
{
    return QImage::fromData(from.data(), from.format().toLatin1().data());
}

static bool isFloatingPointImageFormat(QImage::Format format)
{
    switch (format)
    {
    case QImage::Format_RGBX16FPx4:
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
    case QImage::Format_RGBX32FPx4:
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        return true;
    default:
        return false;
    }
}

static std::optional<QtProtobufPrivate::QtGui::QImage> convert(const QImage &from)
{
    if (from.isNull())
        return std::nullopt;

    static bool tiffSupported = QImageReader::supportedImageFormats().contains("tiff");
    const auto extension = (isFloatingPointImageFormat(from.format())
                            && tiffSupported) ? "tiff"_L1 : "png"_L1;
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    if (from.save(&buffer, extension.data())) {
        QtProtobufPrivate::QtGui::QImage image;
        image.setData(data);
        image.setFormat(extension);
        return image;
    }
    return std::nullopt;
}

namespace QtProtobuf {
/*!
    Registers serializers for the Qt::ProtobufQtGuiTypes library.
*/
void registerProtobufQtGuiTypes() {
    QtProtobufPrivate::registerQtTypeHandler<QRgba64, QtProtobufPrivate::QtGui::QRgba64>();
    QtProtobufPrivate::registerQtTypeHandler<QColor, QtProtobufPrivate::QtGui::QColor>();
    QtProtobufPrivate::registerQtTypeHandler<QMatrix4x4, QtProtobufPrivate::QtGui::QMatrix4x4>();
    QtProtobufPrivate::registerQtTypeHandler<QVector2D, QtProtobufPrivate::QtGui::QVector2D>();
    QtProtobufPrivate::registerQtTypeHandler<QVector3D, QtProtobufPrivate::QtGui::QVector3D>();
    QtProtobufPrivate::registerQtTypeHandler<QVector4D, QtProtobufPrivate::QtGui::QVector4D>();
    QtProtobufPrivate::registerQtTypeHandler<QTransform, QtProtobufPrivate::QtGui::QTransform>();
    QtProtobufPrivate::registerQtTypeHandler<QQuaternion, QtProtobufPrivate::QtGui::QQuaternion>();
    QtProtobufPrivate::registerQtTypeHandler<QImage, QtProtobufPrivate::QtGui::QImage>();
}
}

QT_END_NAMESPACE
