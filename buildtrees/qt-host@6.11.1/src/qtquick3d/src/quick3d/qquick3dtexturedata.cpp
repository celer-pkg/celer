// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dtexturedata.h"
#include "qquick3dtexturedata_p.h"
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TextureData
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \nativetype QQuick3DTextureData
    \brief Base type for custom texture data.

    Custom texture data allows using application-generated texture data, that
    can possibly change dynamically as well. To use custom texture data set the
    \l{Texture::textureData}{textureData} property of \l {Texture} to reference
    a TextureData object.

    Custom Texture data is implemented in C++ by creating a QQuick3DTextureData
    instance, often subclassing it. The QQuick3DTextureData type is registered to
    QML under the name of TextureData. Once the subclass is registered to QML,
    Texture objects can start referencing it.

    An example of when to use this API is when there is a need to procedurally
    generate a texture at runtime rather than loading a static image from a file.

    \code
    import MyCustomTexture 1.0

    Model {
        source: "#Cube"
        materials: [
            DefaultMaterial {
                diffuseMap: diffuseMapCustomTexture
            }
        ]
    }

    Texture {
        id: diffuseMapCustomTexture
        textureData: MyCustomTextureData {

        }
    }

    \endcode

    \sa Texture
*/

/*!
    \class QQuick3DTextureData
    \inmodule QtQuick3D
    \inherits QQuick3DObject
    \since 6.0
    \brief Base class for defining custom texture data.

    The QQuick3DTextureData class can be used to specify custom texture data for a
    \l {Texture} in a Qt Quick 3D scene.

    While not strictly required, the typical usage is to inherit from this class.
    The subclass is then exposed to QML by registering it to the type system. The
    \l{Texture::textureData}{textureData} property of a \l {QtQuick3D::Texture}{Texture}
    can then be set to reference an instance of the registered type.

    Example implementation:

    \code
    class CustomTextureData : public QQuick3DTextureData
    {
        Q_OBJECT
        ... properties ...

    public:
        CustomTextureData() { regenerateTextureData(); }

        void setProperty(...)
        {
            // Change relevant internal data.
            // ...

            // Update the texture data
            regenerateTextureData();

            // Finally, trigger an update. This is relevant in case nothing else
            // is changed in the scene; this way we make sure a new frame will
            // be rendered
            update();
        }
    private:
        void regenerateTextureData()
        {
            QByteArray textureData;
            textureData = generateTextureData();
            setTextureData(textureData);
            setSize(QSize(256, 256));
            setFormat(QQuick3DTextureData::Format::RGBA8)
            setHasTransparency(true);
        }
    };
    \endcode

    This class can then be registered as a QML type and used with \l {QtQuick3D::Texture}{Texture}.

    In Qt 5 type registration happened with qmlRegisterType:
    \code
    qmlRegisterType<MyCustomTextureData>("Example", 1, 0, "MyCustomTextureData");
    \endcode

    In Qt 6 the default approach is to use automatic registration with the help
    of the build system. Instead of calling qmlRegisterType, the \c{.pro} file
    can now contain:

    \code
    CONFIG += qmltypes
    QML_IMPORT_NAME = Example
    QML_IMPORT_MAJOR_VERSION = 1
    \endcode

    With CMake, automatic registration is the default behavior, so no special
    settings are needed beyond basic QML module setup:
    \code
    qt_add_qml_module(application
        URI Example
        VERSION 1.0
    )
    \endcode

    The class implementation should add QML_NAMED_ELEMENT:

    \code
    class CustomTextureData : public QQuick3DTextureData
    {
        Q_OBJECT
        QML_NAMED_ELEMENT(MyCustomTextureData)
        ...
    };
    \endcode

    The QML code can then use the custom type:

    \code
    import Example 1.0

    Model {
        source: "#Cube"
        materials: [
            DefaultMaterial {
                diffuseMap: diffuseMapCustomTexture
            }
        ]
        Texture {
            id: diffuseMapCustomTexture
            textureData: MyCustomTextureData { }
        }
    }
    \endcode
*/

/*!
    \enum QQuick3DTextureData::Format
    Returns the color format of the texture data assigned in \l textureData property.

    \value None The color format is not defined
    \value RGBA8 The color format is considered as 8-bit integer in R, G, B and alpha channels.
    \value RGBA16F The color format is considered as 16-bit float in R,G,B and alpha channels.
    \value RGBA32F The color format is considered as 32-bit float in R, G, B and alpha channels.
    \value RGBE8 The color format is considered as 8-bit mantissa in the R, G, and B channels and 8-bit shared exponent.
    \value R8 The color format is considered as 8-bit integer in R channel.
    \value R16 The color format is considered as 16-bit integer in R channel.
    \value R16F The color format is considered as 16-bit float in R channel.
    \value R32F The color format is considered as 32-bit float R channel.
    \value BC1 The color format is considred as BC1 compressed format with R, G, B, and alpha channels.
    \value BC2 The color format is considred as BC2 compressed format with R, G, B, and alpha channels.
    \value BC3 The color format is considred as BC3 compressed format with R, G, B, and alpha channels.
    \value BC4 The color format is considred as BC4 compressed format with one color channel.
    \value BC5 The color format is considred as BC5 compressed format with two color channels.
    \value BC6H The color format is considred as BC6H compressed format with three high dynamic range color channels.
    \value BC7 The color format is considred as BC7 compressed format with R, G, B, and alpha channels.
    \value DXT1_RGBA The color format is considered as DXT1 compressed format with R, G, B and alpha channels.
    \value DXT1_RGB The color format is considered as DXT1 compressed format with R, G and B channels.
    \value DXT3_RGBA The color format is considered as DXT3 compressed format with R, G, B and alpha channels.
    \value DXT5_RGBA The color format is considered as DXT5 compressed format with R, G, B and alpha channels.
    \value ETC2_RGB8 The color format is considered as ETC2 compressed format for RGB888 data
    \value ETC2_RGB8A1 The color format is considered as ETC2 compressed format for RGBA data where alpha is 1-bit.
    \value ETC2_RGBA8 The color format is considered as ETC2 compressed format with RGBA8888 data.
    \value ASTC_4x4 The color format is considered as ASTC compressed format with 4x4 block footprint.
    \value ASTC_5x4 The color format is considered as ASTC compressed format with 5x4 block footprint.
    \value ASTC_5x5 The color format is considered as ASTC compressed format with 5x5 block footprint.
    \value ASTC_6x5 The color format is considered as ASTC compressed format with 6x5 block footprint.
    \value ASTC_6x6 The color format is considered as ASTC compressed format with 6x6 block footprint.
    \value ASTC_8x5 The color format is considered as ASTC compressed format with 8x5 block footprint.
    \value ASTC_8x6 The color format is considered as ASTC compressed format with 8x6 block footprint.
    \value ASTC_8x8 The color format is considered as ASTC compressed format with 8x8 block footprint.
    \value ASTC_10x5 The color format is considered as ASTC compressed format with 10x5 block footprint.
    \value ASTC_10x6 The color format is considered as ASTC compressed format with 10x6 block footprint.
    \value ASTC_10x8 The color format is considered as ASTC compressed format with 10x8 block footprint.
    \value ASTC_10x10 The color format is considered as ASTC compressed format with 10x10 block footprint.
    \value ASTC_12x10 The color format is considered as ASTC compressed format with 12x10 block footprint.
    \value ASTC_12x12 The color format is considered as ASTC compressed format with 12x12 block footprint.

    \note With the exception of \c RGBA8, not every format is supported at runtime as this
    depends on which backend is being used as well which hardware is being used.

    \note \c RGBE is internally represented as an \c RGBA8 but is intepreted as described when used
    as a lightProbe or skybox texture.

    \note Using the value \c None will assume the default value of \c RGBA8
*/


QQuick3DTextureDataPrivate::QQuick3DTextureDataPrivate()
    : QQuick3DObjectPrivate(QQuick3DTextureDataPrivate::Type::TextureData)
{

}

QQuick3DTextureData::QQuick3DTextureData(QQuick3DObject *parent)
    : QQuick3DObject(*new QQuick3DTextureDataPrivate, parent)
{

}

QQuick3DTextureData::~QQuick3DTextureData()
{

}

/*!
    Returns the current texture data defined by this item.
*/
const QByteArray QQuick3DTextureData::textureData() const
{
    const Q_D(QQuick3DTextureData);
    return d->textureData;

}

/*!
    Sets the texture data. The contents of \a data must respect the \l size and
    \l format properties as the backend will try and upload and use the data as if
    it were a texture of size and format, and if there is any deviation the result
    will be somewhere between incorrect rendering of the texture, or potentially a crash.
*/

void QQuick3DTextureData::setTextureData(const QByteArray &data)
{
    Q_D(QQuick3DTextureData);
    d->textureData = data;
    d->textureDataDirty = true;
    update();
}

/*!
    Returns the size of the texture data in pixels.
*/
QSize QQuick3DTextureData::size() const
{
    const Q_D(QQuick3DTextureData);
    return d->size;
}

/*!
    Sets the \a size of the texture data in pixels.
*/
void QQuick3DTextureData::setSize(const QSize &size)
{
    Q_D(QQuick3DTextureData);
    d->size = size;
    update();
}

/*!
    Returns the depth of the texture data in pixels.
*/
int QQuick3DTextureData::depth() const
{
    const Q_D(QQuick3DTextureData);
    return d->depth;
}

/*!
    Sets the \a depth of the texture data in pixels. Setting the depth above
    0 means that the texture is handled as a 3D texture.
*/
void QQuick3DTextureData::setDepth(int depth)
{
    Q_D(QQuick3DTextureData);
    d->depth = depth;
    update();
}

/*!
    Returns the format of the texture data.
*/
QQuick3DTextureData::Format QQuick3DTextureData::format() const
{
    const Q_D(QQuick3DTextureData);
    return d->format;
}

/*!
    Sets the \a format of the texture data.

    The default format is /c RGBA8
*/
void QQuick3DTextureData::setFormat(QQuick3DTextureData::Format format)
{
    Q_D(QQuick3DTextureData);
    d->format = format;
    update();
}

/*!
    Returns \c true if the texture data has transparency.

    The default value is \c false.
*/
bool QQuick3DTextureData::hasTransparency() const
{
    const Q_D(QQuick3DTextureData);
    return d->hasTransparency;
}

/*!
    Set \a hasTransparency to true if the texture data has an active alpha
    channel with non-opaque values.

    This is used as an optimization by the engine so that for formats that
    do support an alpha channel do not need to have each value checked for
    non-opaque values.
*/
void QQuick3DTextureData::setHasTransparency(bool hasTransparency)
{
    Q_D(QQuick3DTextureData);
    d->hasTransparency = hasTransparency;
    update();
}


static QSSGRenderTextureFormat::Format convertToBackendFormat(QQuick3DTextureData::Format format)
{
    switch (format) {
    case QQuick3DTextureData::None:
    case QQuick3DTextureData::RGBA8:
        return QSSGRenderTextureFormat::RGBA8;
    case QQuick3DTextureData::RGBA16F:
        return QSSGRenderTextureFormat::RGBA16F;
    case QQuick3DTextureData::RGBA32F:
        return QSSGRenderTextureFormat::RGBA32F;
    case QQuick3DTextureData::RGBE8:
        return QSSGRenderTextureFormat::RGBE8;
    case QQuick3DTextureData::R8:
        return QSSGRenderTextureFormat::R8;
    case QQuick3DTextureData::R16:
        return QSSGRenderTextureFormat::R16;
    case QQuick3DTextureData::R16F:
        return QSSGRenderTextureFormat::R16F;
    case QQuick3DTextureData::R32F:
        return QSSGRenderTextureFormat::R32F;
    case QQuick3DTextureData::BC1:
        return QSSGRenderTextureFormat::BC1;
    case QQuick3DTextureData::BC2:
        return QSSGRenderTextureFormat::BC2;
    case QQuick3DTextureData::BC3:
        return QSSGRenderTextureFormat::BC3;
    case QQuick3DTextureData::BC4:
        return QSSGRenderTextureFormat::BC4;
    case QQuick3DTextureData::BC5:
        return QSSGRenderTextureFormat::BC5;
    case QQuick3DTextureData::BC6H:
        return QSSGRenderTextureFormat::BC6H;
    case QQuick3DTextureData::BC7:
        return QSSGRenderTextureFormat::BC7;
    case QQuick3DTextureData::DXT1_RGBA:
        return QSSGRenderTextureFormat::RGBA_DXT1;
    case QQuick3DTextureData::DXT1_RGB:
        return QSSGRenderTextureFormat::RGB_DXT1;
    case QQuick3DTextureData::DXT3_RGBA:
        return QSSGRenderTextureFormat::RGBA_DXT3;
    case QQuick3DTextureData::DXT5_RGBA:
        return QSSGRenderTextureFormat::RGBA_DXT5;
    case QQuick3DTextureData::ETC2_RGB8:
        return QSSGRenderTextureFormat::RGB8_ETC2;
    case QQuick3DTextureData::ETC2_RGB8A1:
        return QSSGRenderTextureFormat::RGB8_PunchThrough_Alpha1_ETC2;
    case QQuick3DTextureData::ETC2_RGBA8:
        return QSSGRenderTextureFormat::RGBA8_ETC2_EAC;
    case QQuick3DTextureData::ASTC_4x4:
        return QSSGRenderTextureFormat::RGBA_ASTC_4x4;
    case QQuick3DTextureData::ASTC_5x4:
        return QSSGRenderTextureFormat::RGBA_ASTC_5x4;
    case QQuick3DTextureData::ASTC_5x5:
        return QSSGRenderTextureFormat::RGBA_ASTC_5x5;
    case QQuick3DTextureData::ASTC_6x5:
        return QSSGRenderTextureFormat::RGBA_ASTC_6x5;
    case QQuick3DTextureData::ASTC_6x6:
        return QSSGRenderTextureFormat::RGBA_ASTC_6x6;
    case QQuick3DTextureData::ASTC_8x5:
        return QSSGRenderTextureFormat::RGBA_ASTC_8x5;
    case QQuick3DTextureData::ASTC_8x6:
        return QSSGRenderTextureFormat::RGBA_ASTC_8x6;
    case QQuick3DTextureData::ASTC_8x8:
        return QSSGRenderTextureFormat::RGBA_ASTC_8x8;
    case QQuick3DTextureData::ASTC_10x5:
        return QSSGRenderTextureFormat::RGBA_ASTC_10x5;
    case QQuick3DTextureData::ASTC_10x6:
        return QSSGRenderTextureFormat::RGBA_ASTC_10x6;
    case QQuick3DTextureData::ASTC_10x8:
        return QSSGRenderTextureFormat::RGBA_ASTC_10x8;
    case QQuick3DTextureData::ASTC_10x10:
        return QSSGRenderTextureFormat::RGBA_ASTC_10x10;
    case QQuick3DTextureData::ASTC_12x10:
        return QSSGRenderTextureFormat::RGBA_ASTC_12x10;
    case QQuick3DTextureData::ASTC_12x12:
        return QSSGRenderTextureFormat::RGBA_ASTC_12x12;
    default:
        return QSSGRenderTextureFormat::RGBA8;
    }
}

/*!
    \internal
*/
QSSGRenderGraphObject *QQuick3DTextureData::updateSpatialNode(QSSGRenderGraphObject *node)
{
    Q_D(QQuick3DTextureData);
    if (!node) {
        markAllDirty();
        node = new QSSGRenderTextureData();
    }
    QQuick3DObject::updateSpatialNode(node);
    auto *textureData = static_cast<QSSGRenderTextureData*>(node);

    bool changed = false;

    // Use a dirty flag so we don't compare large buffer values
    if (d->textureDataDirty) {
        d->textureDataDirty = false;
        textureData->setTextureData(d->textureData);
        changed = true;
    }

    // Can't use qUpdateIfNeeded unfortunately
    if (d->size != textureData->size()) {
        textureData->setSize(d->size);
        changed = true;
    }

    if (d->depth != textureData->depth()) {
        textureData->setDepth(d->depth);
        changed = true;
    }

    QSSGRenderTextureFormat format = convertToBackendFormat(d->format);
    if (format != textureData->format()) {
        textureData->setFormat(format);
        changed = true;
    }

    if (d->hasTransparency != textureData->hasTransparency()) {
        textureData->setHasTransparency(d->hasTransparency);
        changed = true;
    }

    if (changed)
        emit textureDataNodeDirty();

    DebugViewHelpers::ensureDebugObjectName(textureData, this);

    return node;
}

void QQuick3DTextureData::markAllDirty()
{
    QQuick3DObject::markAllDirty();
}


QT_END_NAMESPACE
