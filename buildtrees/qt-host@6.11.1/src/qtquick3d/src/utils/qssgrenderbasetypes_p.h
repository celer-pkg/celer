// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERBASETYPES_P_H
#define QSSGRENDERBASETYPES_P_H

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

#include <QtQuick3DUtils/qtquick3dutilsexports.h>

#include <QtCore/qvariant.h>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>
#include <QFloat16>

#include <cmath>

QT_BEGIN_NAMESPACE

enum class QSSGRenderComponentType // stored in mesh files, the values must not change, must match Mesh::ComponentType
{
    UnsignedInt8 = 1,
    Int8,
    UnsignedInt16,
    Int16,
    UnsignedInt32,
    Int32,
    UnsignedInt64,
    Int64,
    Float16,
    Float32,
    Float64
};

enum class QSSGRenderDrawMode // stored in mesh files, the values must not change, must match Mesh::DrawMode
{
    Points = 1,
    LineStrip,
    LineLoop, // Not supported
    Lines,
    TriangleStrip,
    TriangleFan,
    Triangles
};

enum class QSSGRenderWinding // stored in mesh files, the values must not change, must match Mesh::Winding
{
    Clockwise = 1,
    CounterClockwise
};

enum class QSSGRenderSamplerType
{
    Unknown,
    Sampler2D,
    Sampler2DArray,
    Sampler3D,
    SamplerCube,
    SamplerCubeArray,
    SamplerBuffer,
};

struct Q_QUICK3DUTILS_EXPORT QSSGRenderTextureFormat
{
    static constexpr quint8 DepthTextureFlag = 1u << 6;
    static constexpr quint8 CompressedTextureFlag = 1u << 7;

    enum Format : quint8 {
        // Non-compressed formats
        Unknown = 0,
        R8,
        R16,
        R16F,
        R32I,
        R32UI,
        R32F,
        RG8,
        RGBA8,
        RGB8,
        SRGB8,
        SRGB8A8,
        RGB565,
        RGBA5551,
        Alpha8,
        Luminance8,
        Luminance16,
        LuminanceAlpha8,
        RGBA16F,
        RG16F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11G11B10,
        RGB9E5,
        RGB10_A2,
        RGB16F,
        RGBA32UI,
        RGB32UI,
        RGBA16UI,
        RGB16UI,
        RGBA8UI,
        RGB8UI,
        RGBA32I,
        RGB32I,
        RGBA16I,
        RGB16I,
        RGBA8I,
        RGB8I,
        RGBE8,

        // Depth textures
        Depth16 = DepthTextureFlag + 1,
        Depth24,
        Depth32,
        Depth24Stencil8,

        // Compressed formats
        RGBA_DXT1 = CompressedTextureFlag + 1,
        RGB_DXT1,
        RGBA_DXT3,
        RGBA_DXT5,
        R11_EAC_UNorm,
        R11_EAC_SNorm,
        RG11_EAC_UNorm,
        RG11_EAC_SNorm,
        RGB8_ETC2,
        SRGB8_ETC2,
        RGB8_PunchThrough_Alpha1_ETC2,
        SRGB8_PunchThrough_Alpha1_ETC2,
        RGBA8_ETC2_EAC,
        SRGB8_Alpha8_ETC2_EAC,
        RGBA_ASTC_4x4,
        RGBA_ASTC_5x4,
        RGBA_ASTC_5x5,
        RGBA_ASTC_6x5,
        RGBA_ASTC_6x6,
        RGBA_ASTC_8x5,
        RGBA_ASTC_8x6,
        RGBA_ASTC_8x8,
        RGBA_ASTC_10x5,
        RGBA_ASTC_10x6,
        RGBA_ASTC_10x8,
        RGBA_ASTC_10x10,
        RGBA_ASTC_12x10,
        RGBA_ASTC_12x12,
        SRGB8_Alpha8_ASTC_4x4,
        SRGB8_Alpha8_ASTC_5x4,
        SRGB8_Alpha8_ASTC_5x5,
        SRGB8_Alpha8_ASTC_6x5,
        SRGB8_Alpha8_ASTC_6x6,
        SRGB8_Alpha8_ASTC_8x5,
        SRGB8_Alpha8_ASTC_8x6,
        SRGB8_Alpha8_ASTC_8x8,
        SRGB8_Alpha8_ASTC_10x5,
        SRGB8_Alpha8_ASTC_10x6,
        SRGB8_Alpha8_ASTC_10x8,
        SRGB8_Alpha8_ASTC_10x10,
        SRGB8_Alpha8_ASTC_12x10,
        SRGB8_Alpha8_ASTC_12x12,
        BC1,
        BC2,
        BC3,
        BC4,
        BC5,
        BC6H,
        BC7,
    };
    Format format;

    constexpr QSSGRenderTextureFormat(Format f) : format(f) {}

    [[nodiscard]] constexpr bool isCompressedTextureFormat() const noexcept
    {
        return (format & CompressedTextureFlag);
    }

    [[nodiscard]] constexpr bool isUncompressedTextureFormat() const noexcept
    {
        return !isCompressedTextureFormat();
    }

    [[nodiscard]] bool isDepthTextureFormat() const noexcept
    {
        return (format & DepthTextureFlag);
    }

    [[nodiscard]] const char *toString() const;

    [[nodiscard]] qint32 getSizeofFormat() const noexcept;

    [[nodiscard]] qint32 getNumberOfComponent() const noexcept;

    void decodeToFloat(void *inPtr, qint32 byteOfs, float *outPtr) const;
    void encodeToPixel(float *inPtr, void *outPtr, qint32 byteOfs) const;

    bool operator==(const QSSGRenderTextureFormat &other) const { return format == other.format; }
    bool operator!=(const QSSGRenderTextureFormat &other) const { return format != other.format; }
};

enum class QSSGRenderTextureFilterOp
{
    None = 0,
    Nearest,
    Linear
};

enum class QSSGRenderTextureCoordOp : quint8
{
    Unknown = 0,
    ClampToEdge,
    MirroredRepeat,
    Repeat
};

enum class QSSGCullFaceMode
{
    Unknown = 0,
    Back,
    Front,
    Disabled,
    FrontAndBack, // Not exposed in the front-end
};

enum class QSSGDepthDrawMode
{
    OpaqueOnly,
    Always,
    Never,
    OpaquePrePass
};

template<typename TDataType>
struct QSSGRenderGenericVec2
{
    TDataType x;
    TDataType y;
    QSSGRenderGenericVec2(TDataType _x, TDataType _y) : x(_x), y(_y) {}
    QSSGRenderGenericVec2() {}
    bool operator==(const QSSGRenderGenericVec2 &inOther) const { return x == inOther.x && y == inOther.y; }
};

template<typename TDataType>
struct QSSGRenderGenericVec3
{
    TDataType x;
    TDataType y;
    TDataType z;
    QSSGRenderGenericVec3(TDataType _x, TDataType _y, TDataType _z) : x(_x), y(_y), z(_z) {}
    QSSGRenderGenericVec3() {}
    bool operator==(const QSSGRenderGenericVec3 &inOther) const
    {
        return x == inOther.x && y == inOther.y && z == inOther.z;
    }
};

template<typename TDataType>
struct QSSGRenderGenericVec4
{
    TDataType x;
    TDataType y;
    TDataType z;
    TDataType w;
    QSSGRenderGenericVec4(TDataType _x, TDataType _y, TDataType _z, TDataType _w) : x(_x), y(_y), z(_z), w(_w) {}
    QSSGRenderGenericVec4() {}
    bool operator==(const QSSGRenderGenericVec4 &inOther) const
    {
        return x == inOther.x && y == inOther.y && z == inOther.z && w == inOther.w;
    }
};

namespace QSSGRenderShaderValue
{
    enum Type : quint32
    {
        Unknown = 0,
        Integer, // qint32,
        IntegerVec2, // qint32_2,
        IntegerVec3, // qint32_3,
        IntegerVec4, // qint32_4,
        Boolean, // bool
        BooleanVec2, // bool_2,
        BooleanVec3, // bool_3,
        BooleanVec4, // bool_4,
        Float, // float,
        Vec2, // QVector2D,
        Vec3, // QVector3D,
        Vec4, // QVector4D,
        UnsignedInteger, // quint32,
        UnsignedIntegerVec2, // quint32_2,
        UnsignedIntegerVec3, // quint32_3,
        UnsignedIntegerVec4, // quint32_4,
        Matrix3x3, // QMatrix3x3,
        Matrix4x4, // QMatrix4x4,
        Rgba, // QColor
        Size, // QSize
        SizeF, // QSizeF
        Point, // QPoint
        PointF, // QPointF
        Rect, // QRect
        RectF, // QRectF
        Quaternion, // QQuaternion
        Texture,
    };

    using vec2 = QVector2D;
    using vec3 = QVector3D;
    using vec4 = QVector4D;
    using bvec2 = QSSGRenderGenericVec2<bool>;
    using bvec3 = QSSGRenderGenericVec3<bool>;
    using bvec4 = QSSGRenderGenericVec4<bool>;
    using ivec2 = QSSGRenderGenericVec2<qint32>;
    using ivec3 = QSSGRenderGenericVec3<qint32>;
    using ivec4 = QSSGRenderGenericVec4<qint32>;
    using uvec2 = QSSGRenderGenericVec2<quint32>;
    using uvec3 = QSSGRenderGenericVec3<quint32>;
    using uvec4 = QSSGRenderGenericVec4<quint32>;
}

enum class QSSGRenderTextureTypeValue
{
    Unknown = 0,
    Diffuse,
    Specular,
    Environment,
    Bump,
    Normal,
    Emissive,
    Anisotropy,
    Translucent
};

enum class QSSGRenderTextureCubeFace : quint8
{
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ
};

using QSSGRenderTextureCubeFaceT = std::underlying_type_t<QSSGRenderTextureCubeFace>;

// Same order as expected by QRHI!
static constexpr QSSGRenderTextureCubeFace QSSGRenderTextureCubeFaces[] {
    QSSGRenderTextureCubeFace::PosX, QSSGRenderTextureCubeFace::NegX,
    QSSGRenderTextureCubeFace::PosY, QSSGRenderTextureCubeFace::NegY,
    QSSGRenderTextureCubeFace::PosZ, QSSGRenderTextureCubeFace::NegZ
};

constexpr QSSGRenderTextureCubeFaceT QSSGRenderTextureCubeFaceMask { 0xf };
constexpr QSSGRenderTextureCubeFace QSSGRenderTextureCubeFaceNone { QSSGRenderTextureCubeFaceT(1 << 4) };

class Q_QUICK3DUTILS_EXPORT QSSGBaseTypeHelpers
{
    QSSGBaseTypeHelpers() = default;
public:
    // Enum as string
    static const char *toString(QSSGRenderTextureCubeFace value);
    static const char *toString(QSSGRenderTextureTypeValue value);
    static const char *toString(QSSGDepthDrawMode value);
    static const char *toString(QSSGRenderWinding value);
    static const char *toString(QSSGCullFaceMode value);
    static const char *toString(QSSGRenderComponentType value);
    static const char *toString(QSSGRenderTextureFormat::Format value);
    static const char *toString(QSSGRenderTextureCoordOp value);
    static const char *toString(QSSGRenderTextureFilterOp value);
    static QByteArray toString(QSSGRenderSamplerType value);

    static const char *displayName(QSSGRenderTextureCubeFace face);

    static size_t getSizeOfType(QSSGRenderComponentType type);

    // Note: These will wrap around
    static constexpr QSSGRenderTextureCubeFace next(QSSGRenderTextureCubeFace face)
    { return (face == QSSGRenderTextureCubeFaces[5]) ? QSSGRenderTextureCubeFaces[0] : QSSGRenderTextureCubeFace(quint8(face) + 1); }
    static constexpr QSSGRenderTextureCubeFace prev(QSSGRenderTextureCubeFace face)
    { return (face == QSSGRenderTextureCubeFaces[0]) ? QSSGRenderTextureCubeFaces[5] : QSSGRenderTextureCubeFace(quint8(face) - 1); }

    static constexpr QSSGRenderTextureCubeFaceT indexOfCubeFace(QSSGRenderTextureCubeFace face) noexcept { return QSSGRenderTextureCubeFaceT(face) & 0xf; }
};

class QSSGBaseTypeProperty
{
public:
    QSSGBaseTypeProperty() = default;
    QSSGBaseTypeProperty(const QByteArray &name, const QByteArray &typeName, const QVariant &value, QSSGRenderShaderValue::Type shaderDataType, int pid = -1)
        : name(name), typeName(typeName), value(value), shaderDataType(shaderDataType), pid(pid)
    { }
    QByteArray name;
    QByteArray typeName;
    mutable QVariant value;
    QSSGRenderShaderValue::Type shaderDataType;
    int pid;
};

QT_END_NAMESPACE

#endif // QSSGRENDERBASETYPES_P_H
