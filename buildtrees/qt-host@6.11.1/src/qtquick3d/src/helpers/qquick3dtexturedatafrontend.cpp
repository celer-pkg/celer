// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dtexturedatafrontend_p.h"
#include <QSize>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ProceduralTextureData
    \inqmlmodule QtQuick3D.Helpers
    \inherits TextureData
    \brief Allows creation and population of TextureData from QML.
    \since 6.6

    ProceduralTextureData is a helper type that allows creation of TextureData from QML.
    The TextureData component is Abstract, and is usually created from C++. With
    ProceduralTextureData, it is possible to populate a TextureData from QML.

    \qml
    ProceduralTextureData {
        id: dynamicTextureData
        property color color1: "red"
        property color color2: "black"
        width: 32
        height: 32
        hasTransparency: false
        format: TextureData.RGBA8
        textureData: generateTextureData(color1, color2)

        function generateTextureData(newColor1: color, newColor2 : color) : ArrayBuffer {
            let dataBuffer = new ArrayBuffer(width * height * 4)
            let data = new Uint8Array(dataBuffer)
            // Create a checkered pattern using newColor1 and newColor2
            for (let x = 0; x < width; x++) {
                for (let y = 0; y < height; y++) {
                    let index = (x + y * width) * 4
                    let color = (x % 2 === y % 2) ? newColor1 : newColor2
                    data[index + 0] = color.r * 255
                    data[index + 1] = color.g * 255
                    data[index + 2] = color.b * 255
                    data[index + 3] = 255
                }
            }
            return dataBuffer
        }
    }
    \endqml

    In the above code snippet, the function generateTextureData is used to generate a
    checkerboard pattern using the two colors color1 and color2.  By filling an
    ArrayBuffer with the generated data, the textureData property of the TextureData
    is populated.
*/

/*!
    \qmlproperty int ProceduralTextureData::width

    This property holds the width of the texture data in pixels.
    \default 0

*/

/*!
    \qmlproperty int ProceduralTextureData::height

    This property holds the height of the texture data in pixels.
    \default 0
*/

/*!
    \qmlproperty int ProceduralTextureData::depth

    This property holds the depth of the texture data in pixels.
    \default 0
    Setting the depth above 0 means that the texture is handled as a 3D texture.
*/

/*!
    \qmlproperty bool ProceduralTextureData::hasTransparency

    This property holds whether the texture data has transparency.
*/

/*!
    \qmlproperty enumeration ProceduralTextureData::format

    This property holds the format of the texture data.

    \default TextureData.RGBA8

    \value TexureData.None The color format is not defined
    \value TexureData.RGBA8 The color format is considered as 8-bit integer in R, G, B and alpha channels.
    \value TexureData.RGBA16F The color format is considered as 16-bit float in R,G,B and alpha channels.
    \value TexureData.RGBA32F The color format is considered as 32-bit float in R, G, B and alpha channels.
    \value TexureData.RGBE8 The color format is considered as 8-bit mantissa in the R, G, and B channels and 8-bit shared exponent.
    \value TexureData.R8 The color format is considered as 8-bit integer in R channel.
    \value TexureData.R16 The color format is considered as 16-bit integer in R channel.
    \value TexureData.R16F The color format is considered as 16-bit float in R channel.
    \value TexureData.R32F The color format is considered as 32-bit float R channel.
    \value TexureData.BC1 The color format is considered as BC1 compressed format with R, G, B, and alpha channels.
    \value TexureData.BC2 The color format is considered as BC2 compressed format with R, G, B, and alpha channels.
    \value TexureData.BC3 The color format is considered as BC3 compressed format with R, G, B, and alpha channels.
    \value TexureData.BC4 The color format is considered as BC4 compressed format with one color channel.
    \value TexureData.BC5 The color format is considered as BC5 compressed format with two color channels.
    \value TexureData.BC6H The color format is considered as BC6H compressed format with three high dynamic range color channels.
    \value TexureData.BC7 The color format is considered as BC7 compressed format with R, G, B, and alpha channels.
    \value TexureData.DXT1_RGBA The color format is considered as DXT1 compressed format with R, G, B and alpha channels.
    \value TexureData.DXT1_RGB The color format is considered as DXT1 compressed format with R, G and B channels.
    \value TexureData.DXT3_RGBA The color format is considered as DXT3 compressed format with R, G, B and alpha channels.
    \value TexureData.DXT5_RGBA The color format is considered as DXT5 compressed format with R, G, B and alpha channels.
    \value TexureData.ETC2_RGB8 The color format is considered as ETC2 compressed format for RGB888 data
    \value TexureData.ETC2_RGB8A1 The color format is considered as ETC2 compressed format for RGBA data where alpha is 1-bit.
    \value TexureData.ETC2_RGBA8 The color format is considered as ETC2 compressed format with RGBA8888 data.
    \value TexureData.ASTC_4x4 The color format is considered as ASTC compressed format with 4x4 block footprint.
    \value TexureData.ASTC_5x4 The color format is considered as ASTC compressed format with 5x4 block footprint.
    \value TexureData.ASTC_5x5 The color format is considered as ASTC compressed format with 5x5 block footprint.
    \value TexureData.ASTC_6x5 The color format is considered as ASTC compressed format with 6x5 block footprint.
    \value TexureData.ASTC_6x6 The color format is considered as ASTC compressed format with 6x6 block footprint.
    \value TexureData.ASTC_8x5 The color format is considered as ASTC compressed format with 8x5 block footprint.
    \value TexureData.ASTC_8x6 The color format is considered as ASTC compressed format with 8x6 block footprint.
    \value TexureData.ASTC_8x8 The color format is considered as ASTC compressed format with 8x8 block footprint.
    \value TexureData.ASTC_10x5 The color format is considered as ASTC compressed format with 10x5 block footprint.
    \value TexureData.ASTC_10x6 The color format is considered as ASTC compressed format with 10x6 block footprint.
    \value TexureData.ASTC_10x8 The color format is considered as ASTC compressed format with 10x8 block footprint.
    \value TexureData.ASTC_10x10 The color format is considered as ASTC compressed format with 10x10 block footprint.
    \value TexureData.ASTC_12x10 The color format is considered as ASTC compressed format with 12x10 block footprint.
    \value TexureData.ASTC_12x12 The color format is considered as ASTC compressed format with 12x12 block footprint.

    \note With the exception of \c TexureData.RGBA8, not every format is supported at runtime as this
    depends on which backend is being used as well which hardware is being used.

    \note \c TexureData.RGBE is internally represented as an \c TexureData.RGBA8 but is interpreted as described when used
    as a lightProbe or skybox texture.

    \note Using the value \c TexureData.None will assume the default value of \c TexureData.RGBA8
*/

/*!
    \qmlproperty ArrayBuffer ProceduralTextureData::textureData

    This property holds the texture data.
*/

QQuick3DTextureDataFrontend::QQuick3DTextureDataFrontend()
{

}


QQuick3DTextureData::Format QQuick3DTextureDataFrontend::format() const
{
    return QQuick3DTextureData::format();
}

void QQuick3DTextureDataFrontend::setFormat(const QQuick3DTextureData::Format &newFormat)
{
    if (newFormat == QQuick3DTextureData::format())
        return;

    QQuick3DTextureData::setFormat(newFormat);

    Q_EMIT formatChanged();
}

int QQuick3DTextureDataFrontend::depth() const
{
    return QQuick3DTextureData::depth();
}

void QQuick3DTextureDataFrontend::setDepth(int newDepth)
{
    if (newDepth == QQuick3DTextureData::depth())
        return;


    QQuick3DTextureData::setDepth(newDepth);

    Q_EMIT depthChanged();
}

bool QQuick3DTextureDataFrontend::hasTransparency() const
{
    return QQuick3DTextureData::hasTransparency();
}

void QQuick3DTextureDataFrontend::setHasTransparency(bool newHasTransparency)
{
    if (newHasTransparency == QQuick3DTextureData::hasTransparency())
        return;

    QQuick3DTextureData::setHasTransparency(newHasTransparency);

    Q_EMIT hasTransparencyChanged();
}

QByteArray QQuick3DTextureDataFrontend::textureData() const
{
    return QQuick3DTextureData::textureData();
}

void QQuick3DTextureDataFrontend::setTextureData(const QByteArray &newTextureData)
{
    QQuick3DTextureData::setTextureData(newTextureData);
    Q_EMIT textureDataChanged();
}

int QQuick3DTextureDataFrontend::width() const
{
    return QQuick3DTextureData::size().width();
}

void QQuick3DTextureDataFrontend::setWidth(int newWidth)
{
    const auto size = QQuick3DTextureData::size();

    if (size.width() == newWidth)
        return;

    QQuick3DTextureData::setSize(QSize(newWidth, size.height()));

    Q_EMIT widthChanged();
}

int QQuick3DTextureDataFrontend::height() const
{
    return QQuick3DTextureData::size().height();
}

void QQuick3DTextureDataFrontend::setHeight(int newHeight)
{
    const auto size = QQuick3DTextureData::size();

    if (size.height() == newHeight)
        return;

    QQuick3DTextureData::setSize(QSize(size.width(), newHeight));

    Q_EMIT heightChanged();
}

QT_END_NAMESPACE
