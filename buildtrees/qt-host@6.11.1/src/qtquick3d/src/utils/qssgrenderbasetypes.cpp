// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderbasetypes_p.h"

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

const char *QSSGRenderTextureFormat::toString() const
{
    switch (format) {
    case QSSGRenderTextureFormat::R8:
        return "R8";
    case QSSGRenderTextureFormat::R16:
        return "R16";
    case QSSGRenderTextureFormat::R16F:
        return "R16F";
    case QSSGRenderTextureFormat::R32I:
        return "R32I";
    case QSSGRenderTextureFormat::R32UI:
        return "R32UI";
    case QSSGRenderTextureFormat::R32F:
        return "R32F";
    case QSSGRenderTextureFormat::RG8:
        return "RG8";
    case QSSGRenderTextureFormat::RGBA8:
        return "RGBA8";
    case QSSGRenderTextureFormat::RGB8:
        return "RGB8";
    case QSSGRenderTextureFormat::SRGB8:
        return "SRGB8";
    case QSSGRenderTextureFormat::SRGB8A8:
        return "SRGB8A8";
    case QSSGRenderTextureFormat::RGB565:
        return "RGB565";
    case QSSGRenderTextureFormat::RGBA5551:
        return "RGBA5551";
    case QSSGRenderTextureFormat::Alpha8:
        return "Alpha8";
    case QSSGRenderTextureFormat::Luminance8:
        return "Luminance8";
    case QSSGRenderTextureFormat::Luminance16:
        return "Luminance16";
    case QSSGRenderTextureFormat::LuminanceAlpha8:
        return "LuminanceAlpha8";
    case QSSGRenderTextureFormat::RGBA16F:
        return "RGBA16F";
    case QSSGRenderTextureFormat::RG16F:
        return "RG16F";
    case QSSGRenderTextureFormat::RG32F:
        return "RG32F";
    case QSSGRenderTextureFormat::RGB32F:
        return "RGB32F";
    case QSSGRenderTextureFormat::RGBA32F:
        return "RGBA32F";
    case QSSGRenderTextureFormat::R11G11B10:
        return "R11G11B10";
    case QSSGRenderTextureFormat::RGB9E5:
        return "RGB9E5";
    case QSSGRenderTextureFormat::RGBE8:
        return "RGBE8";
    case QSSGRenderTextureFormat::RGBA_DXT1:
        return "RGBA_DXT1";
    case QSSGRenderTextureFormat::RGB_DXT1:
        return "RGB_DXT1";
    case QSSGRenderTextureFormat::RGBA_DXT3:
        return "RGBA_DXT3";
    case QSSGRenderTextureFormat::RGBA_DXT5:
        return "RGBA_DXT5";
    case QSSGRenderTextureFormat::R11_EAC_UNorm:
        return "R11_EAC_UNorm";
    case QSSGRenderTextureFormat::R11_EAC_SNorm:
        return "R11_EAC_SNorm";
    case QSSGRenderTextureFormat::RG11_EAC_UNorm:
        return "RG11_EAC_UNorm";
    case QSSGRenderTextureFormat::RG11_EAC_SNorm:
        return "RG11_EAC_SNorm";
    case QSSGRenderTextureFormat::RGB8_ETC2:
        return "RGB8_ETC2";
    case QSSGRenderTextureFormat::SRGB8_ETC2:
        return "SRGB8_ETC2";
    case QSSGRenderTextureFormat::RGB8_PunchThrough_Alpha1_ETC2:
        return "RGB8_PunchThrough_Alpha1_ETC2";
    case QSSGRenderTextureFormat::SRGB8_PunchThrough_Alpha1_ETC2:
        return "SRGB8_PunchThrough_Alpha1_ETC2";
    case QSSGRenderTextureFormat::RGBA8_ETC2_EAC:
        return "RGBA8_ETC2_EAC";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ETC2_EAC:
        return "SRGB8_Alpha8_ETC2_EAC";
    case QSSGRenderTextureFormat::RGBA_ASTC_4x4:
        return "RGBA_ASTC_4x4";
    case QSSGRenderTextureFormat::RGBA_ASTC_5x4:
        return "RGBA_ASTC_5x4";
    case QSSGRenderTextureFormat::RGBA_ASTC_5x5:
        return "RGBA_ASTC_5x5";
    case QSSGRenderTextureFormat::RGBA_ASTC_6x5:
        return "RGBA_ASTC_6x5";
    case QSSGRenderTextureFormat::RGBA_ASTC_6x6:
        return "RGBA_ASTC_6x6";
    case QSSGRenderTextureFormat::RGBA_ASTC_8x5:
        return "RGBA_ASTC_8x5";
    case QSSGRenderTextureFormat::RGBA_ASTC_8x6:
        return "RGBA_ASTC_8x6";
    case QSSGRenderTextureFormat::RGBA_ASTC_8x8:
        return "RGBA_ASTC_8x8";
    case QSSGRenderTextureFormat::RGBA_ASTC_10x5:
        return "RGBA_ASTC_10x5";
    case QSSGRenderTextureFormat::RGBA_ASTC_10x6:
        return "RGBA_ASTC_10x6";
    case QSSGRenderTextureFormat::RGBA_ASTC_10x8:
        return "RGBA_ASTC_10x8";
    case QSSGRenderTextureFormat::RGBA_ASTC_10x10:
        return "RGBA_ASTC_10x10";
    case QSSGRenderTextureFormat::RGBA_ASTC_12x10:
        return "RGBA_ASTC_12x10";
    case QSSGRenderTextureFormat::RGBA_ASTC_12x12:
        return "RGBA_ASTC_12x12";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_4x4:
        return "SRGB8_Alpha8_ASTC_4x4";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_5x4:
        return "SRGB8_Alpha8_ASTC_5x4";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_5x5:
        return "SRGB8_Alpha8_ASTC_5x5";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_6x5:
        return "SRGB8_Alpha8_ASTC_6x5";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_6x6:
        return "SRGB8_Alpha8_ASTC_6x6";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x5:
        return "SRGB8_Alpha8_ASTC_8x5";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x6:
        return "SRGB8_Alpha8_ASTC_8x6";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_8x8:
        return "SRGB8_Alpha8_ASTC_8x8";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x5:
        return "SRGB8_Alpha8_ASTC_10x5";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x6:
        return "SRGB8_Alpha8_ASTC_10x6";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x8:
        return "SRGB8_Alpha8_ASTC_10x8";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_10x10:
        return "SRGB8_Alpha8_ASTC_10x10";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_12x10:
        return "SRGB8_Alpha8_ASTC_12x10";
    case QSSGRenderTextureFormat::SRGB8_Alpha8_ASTC_12x12:
        return "SRGB8_Alpha8_ASTC_12x12";
    case QSSGRenderTextureFormat::BC1:
        return "BC1";
    case QSSGRenderTextureFormat::BC2:
        return "BC2";
    case QSSGRenderTextureFormat::BC3:
        return "BC3";
    case QSSGRenderTextureFormat::BC4:
        return "BC4";
    case QSSGRenderTextureFormat::BC5:
        return "BC5";
    case QSSGRenderTextureFormat::BC6H:
        return "BC6H";
    case QSSGRenderTextureFormat::BC7:
        return "BC7";
    case QSSGRenderTextureFormat::Depth16:
        return "Depth16";
    case QSSGRenderTextureFormat::Depth24:
        return "Depth24";
    case QSSGRenderTextureFormat::Depth32:
        return "Depth32";
    case QSSGRenderTextureFormat::Depth24Stencil8:
        return "Depth24Stencil8";
    case QSSGRenderTextureFormat::RGB10_A2:
        return "RGB10_A2";
    case QSSGRenderTextureFormat::RGB16F:
        return "RGB16F";
    case QSSGRenderTextureFormat::RGBA32UI:
        return "RGBA32UI";
    case QSSGRenderTextureFormat::RGB32UI:
        return "RGB32UI";
    case QSSGRenderTextureFormat::RGBA16UI:
        return "RGBA16UI";
    case QSSGRenderTextureFormat::RGB16UI:
        return "RGB16UI";
    case QSSGRenderTextureFormat::RGBA8UI:
        return "RGBA8UI";
    case QSSGRenderTextureFormat::RGB8UI:
        return "RGB8UI";
    case QSSGRenderTextureFormat::RGBA32I:
        return "RGBA32I";
    case QSSGRenderTextureFormat::RGB32I:
        return "RGB32I";
    case QSSGRenderTextureFormat::RGBA16I:
        return "RGBA16I";
    case QSSGRenderTextureFormat::RGB16I:
        return "RGB16I";
    case QSSGRenderTextureFormat::RGBA8I:
        return "RGBA8I";
    case QSSGRenderTextureFormat::RGB8I:
        return "RGB8I";
    case QSSGRenderTextureFormat::Unknown:
        return "Unknown";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

qint32 QSSGRenderTextureFormat::getSizeofFormat() const noexcept
{
    switch (format) {
    case R8:
        return 1;
    case R16F:
        return 2;
    case R16:
        return 2;
    case R32I:
        return 4;
    case R32F:
        return 4;
    case RGBE8:
    case RGBA8:
        return 4;
    case RGB8:
        return 3;
    case RGB565:
        return 2;
    case RGBA5551:
        return 2;
    case Alpha8:
        return 1;
    case Luminance8:
        return 1;
    case LuminanceAlpha8:
        return 1;
    case Depth16:
        return 2;
    case Depth24:
        return 3;
    case Depth32:
        return 4;
    case Depth24Stencil8:
        return 4;
    case RGB9E5:
        return 4;
    case SRGB8:
        return 3;
    case SRGB8A8:
        return 4;
    case RGBA16F:
        return 8;
    case RG16F:
        return 4;
    case RG32F:
        return 8;
    case RGBA32F:
        return 16;
    case RGB32F:
        return 12;
    case R11G11B10:
        return 4;
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

qint32 QSSGRenderTextureFormat::getNumberOfComponent() const noexcept
{
    switch (format) {
    case R8:
        return 1;
    case R16F:
        return 1;
    case R16:
        return 1;
    case R32I:
        return 1;
    case R32F:
        return 1;
    case RGBA8:
        return 4;
    case RGB8:
        return 3;
    case RGB565:
        return 3;
    case RGBA5551:
        return 4;
    case Alpha8:
        return 1;
    case Luminance8:
        return 1;
    case LuminanceAlpha8:
        return 2;
    case Depth16:
        return 1;
    case Depth24:
        return 1;
    case Depth32:
        return 1;
    case Depth24Stencil8:
        return 2;
    case RGB9E5:
        return 3;
    case SRGB8:
        return 3;
    case SRGB8A8:
        return 4;
    case RGBA16F:
        return 4;
    case RG16F:
        return 2;
    case RG32F:
        return 2;
    case RGBA32F:
        return 4;
    case RGB32F:
        return 3;
    case R11G11B10:
        return 3;
    case RGBE8:
        return 4;
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

void QSSGRenderTextureFormat::decodeToFloat(void *inPtr, qint32 byteOfs, float *outPtr) const
{
    Q_ASSERT(byteOfs >= 0);
    outPtr[0] = 0.0f;
    outPtr[1] = 0.0f;
    outPtr[2] = 0.0f;
    outPtr[3] = 0.0f;
    quint8 *src = reinterpret_cast<quint8 *>(inPtr);
    switch (format) {
    case Alpha8:
        outPtr[0] = (float(src[byteOfs])) / 255.0f;
        break;

    case Luminance8:
    case LuminanceAlpha8:
    case R8:
    case RG8:
    case RGB8:
    case RGBA8:
    case SRGB8:
    case SRGB8A8:
        for (qint32 i = 0; i < getSizeofFormat(); ++i) {
            float val = (float(src[byteOfs + i])) / 255.0f;
            outPtr[i] = (i < 3) ? std::pow(val, 0.4545454545f) : val;
        }
        break;
    case RGBE8:
    {
        float pwd = powf(2.0f, int(src[byteOfs + 3]) - 128);
        outPtr[0] = float(src[byteOfs + 0]) * pwd / 255.0;
        outPtr[1] = float(src[byteOfs + 1]) * pwd / 255.0;
        outPtr[2] = float(src[byteOfs + 2]) * pwd / 255.0;
        outPtr[3] = 1.0f;
    } break;

    case R32F:
        outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
        break;
    case RG32F:
        outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
        outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
        break;
    case RGBA32F:
        outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
        outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
        outPtr[2] = reinterpret_cast<float *>(src + byteOfs)[2];
        outPtr[3] = reinterpret_cast<float *>(src + byteOfs)[3];
        break;
    case RGB32F:
        outPtr[0] = reinterpret_cast<float *>(src + byteOfs)[0];
        outPtr[1] = reinterpret_cast<float *>(src + byteOfs)[1];
        outPtr[2] = reinterpret_cast<float *>(src + byteOfs)[2];
        break;

    case R16F:
    case RG16F:
    case RGBA16F:
        for (qint32 i = 0; i < (getSizeofFormat() >> 1); ++i) {
            // NOTE : This only works on the assumption that we don't have any denormals,
            // Infs or NaNs.
            // Every pixel in our source image should be "regular"
            quint16 h = reinterpret_cast<quint16 *>(src + byteOfs)[i];
            quint32 sign = (h & 0x8000u) << 16u;
            quint32 exponent = (((((h & 0x7c00u) >> 10) - 15) + 127) << 23);
            quint32 mantissa = ((h & 0x3ffu) << 13);
            quint32 result = sign | exponent | mantissa;

            if (h == 0 || h == 0x8000)
                result = 0;
            memcpy(outPtr + i, &result, 4);
        }
        break;

    case R11G11B10:
        // place holder
        Q_ASSERT(false);
        break;

    default:
        outPtr[0] = 0.0f;
        outPtr[1] = 0.0f;
        outPtr[2] = 0.0f;
        outPtr[3] = 0.0f;
        break;
    }
}
void QSSGRenderTextureFormat::encodeToPixel(float *inPtr, void *outPtr, qint32 byteOfs) const
{
    struct M8E8
    {
        quint8 m;
        quint8 e;
        M8E8() : m(0), e(0){
        }
        M8E8(const float val) {
            float l2 = 1.f + std::floor(log2f(val));
            float mm = val / powf(2.f, l2);
            m = quint8(mm * 255.f);
            e = quint8(l2 + 128);
        }
        M8E8(const float val, quint8 exp) {
            if (val <= 0) {
                m = e = 0;
                return;
            }
            float mm = val / powf(2.f, exp - 128);
            m = quint8(mm * 255.f);
            e = exp;
        }
    };

    Q_ASSERT(byteOfs >= 0);
    quint8 *dest = reinterpret_cast<quint8 *>(outPtr);
    switch (format) {
    case QSSGRenderTextureFormat::Alpha8:
        dest[byteOfs] = quint8(inPtr[0] * 255.0f);
        break;

    case Luminance8:
    case LuminanceAlpha8:
    case R8:
    case RG8:
    case RGB8:
    case RGBA8:
    case SRGB8:
    case SRGB8A8:
        for (qint32 i = 0; i < getSizeofFormat(); ++i) {
            inPtr[i] = (inPtr[i] > 1.0f) ? 1.0f : inPtr[i];
            if (i < 3)
                dest[byteOfs + i] = quint8(powf(inPtr[i], 2.2f) * 255.0f);
            else
                dest[byteOfs + i] = quint8(inPtr[i] * 255.0f);
        }
        break;
    case RGBE8:
    {
        float max = qMax(inPtr[0], qMax(inPtr[1], inPtr[2]));
        M8E8 ex(max);
        M8E8 a(inPtr[0], ex.e);
        M8E8 b(inPtr[1], ex.e);
        M8E8 c(inPtr[2], ex.e);
        quint8 *dst = reinterpret_cast<quint8 *>(outPtr) + byteOfs;
        dst[0] = a.m;
        dst[1] = b.m;
        dst[2] = c.m;
        dst[3] = ex.e;
    } break;

    case R32F:
        reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
        break;
    case RG32F:
        reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
        reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
        break;
    case RGBA32F:
        reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
        reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
        reinterpret_cast<float *>(dest + byteOfs)[2] = inPtr[2];
        reinterpret_cast<float *>(dest + byteOfs)[3] = inPtr[3];
        break;
    case RGB32F:
        reinterpret_cast<float *>(dest + byteOfs)[0] = inPtr[0];
        reinterpret_cast<float *>(dest + byteOfs)[1] = inPtr[1];
        reinterpret_cast<float *>(dest + byteOfs)[2] = inPtr[2];
        break;

    case R16F:
    case RG16F:
    case RGBA16F:
        for (qint32 i = 0; i < (getSizeofFormat() >> 1); ++i) {
            // NOTE : This also has the limitation of not handling  infs, NaNs and
            // denormals, but it should be
            // sufficient for our purposes.
            if (inPtr[i] > 65519.0f)
                inPtr[i] = 65519.0f;
            if (std::fabs(inPtr[i]) < 6.10352E-5f)
                inPtr[i] = 0.0f;
            quint32 f = reinterpret_cast<quint32 *>(inPtr)[i];
            quint32 sign = (f & 0x80000000) >> 16;
            qint32 exponent = (f & 0x7f800000) >> 23;
            quint32 mantissa = (f >> 13) & 0x3ff;
            exponent = exponent - 112;
            if (exponent > 31)
                exponent = 31;
            if (exponent < 0)
                exponent = 0;
            exponent = exponent << 10;
            reinterpret_cast<quint16 *>(dest + byteOfs)[i] = quint16(sign | quint32(exponent) | mantissa);
        }
        break;

    case R11G11B10:
        // place holder
        Q_ASSERT(false);
        break;

    default:
        dest[byteOfs] = 0;
        dest[byteOfs + 1] = 0;
        dest[byteOfs + 2] = 0;
        dest[byteOfs + 3] = 0;
        break;
    }
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderWinding value)
{
    switch (value) {
    case QSSGRenderWinding::Clockwise:
        return "Clockwise";
    case QSSGRenderWinding::CounterClockwise:
        return "CounterClockwise";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGCullFaceMode value)
{
    switch (value) {
    case QSSGCullFaceMode::Front:
        return "Front";
    case QSSGCullFaceMode::Back:
        return "Back";
    case QSSGCullFaceMode::FrontAndBack:
        return "FrontAndBack";
    case QSSGCullFaceMode::Unknown:
        return "Unknown";
    case QSSGCullFaceMode::Disabled:
        return "Disabled";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderTextureCubeFace value)
{
    switch (value) {
    case QSSGRenderTextureCubeFace::NegX:
        return "NegX";
    case QSSGRenderTextureCubeFace::NegZ:
        return "NegZ";
    case QSSGRenderTextureCubeFace::NegY:
        return "NegY";
    case QSSGRenderTextureCubeFace::PosY:
        return "PosY";
    case QSSGRenderTextureCubeFace::PosX:
        return "PosX";
    case QSSGRenderTextureCubeFace::PosZ:
        return "PosZ";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderTextureTypeValue value)
{
    switch (value) {
    case QSSGRenderTextureTypeValue::Unknown:
        return "Unknown";
    case QSSGRenderTextureTypeValue::Diffuse:
        return "Diffuse";
    case QSSGRenderTextureTypeValue::Specular:
        return "Specular";
    case QSSGRenderTextureTypeValue::Environment:
        return "Environment";
    case QSSGRenderTextureTypeValue::Bump:
        return "Bump";
    case QSSGRenderTextureTypeValue::Normal:
        return "Normal";
    case QSSGRenderTextureTypeValue::Emissive:
        return "Emissive";
    case QSSGRenderTextureTypeValue::Anisotropy:
        return "Anisotropy";
    case QSSGRenderTextureTypeValue::Translucent:
        return "Translucent";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGDepthDrawMode value)
{
    switch (value) {
    case QSSGDepthDrawMode::OpaqueOnly:
        return "OpaqueOnly";
    case QSSGDepthDrawMode::Always:
        return "Always";
    case QSSGDepthDrawMode::Never:
        return "Never";
    case QSSGDepthDrawMode::OpaquePrePass:
        return "OpaquePrePass";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderComponentType value)
{
    switch (value) {
    case QSSGRenderComponentType::UnsignedInt8:
        return "UnsignedInt8";
    case QSSGRenderComponentType::Int8:
        return "Int8";
    case QSSGRenderComponentType::UnsignedInt16:
        return "UnsignedInt16";
    case QSSGRenderComponentType::Int16:
        return "Int16";
    case QSSGRenderComponentType::UnsignedInt32:
        return "UnsignedInt32";
    case QSSGRenderComponentType::Int32:
        return "Int32";
    case QSSGRenderComponentType::UnsignedInt64:
        return "UnsignedInt64";
    case QSSGRenderComponentType::Int64:
        return "Int64";
    case QSSGRenderComponentType::Float16:
        return "Float16";
    case QSSGRenderComponentType::Float32:
        return "Float32";
    case QSSGRenderComponentType::Float64:
        return "Float64";
    }

    Q_UNREACHABLE_RETURN("Unknown");
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderTextureFormat::Format value)
{
    return QSSGRenderTextureFormat(value).toString();
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderTextureCoordOp value)
{
    switch (value) {
    case QSSGRenderTextureCoordOp::ClampToEdge:
        return "ClampToEdge";
    case QSSGRenderTextureCoordOp::MirroredRepeat:
        return "MirroredRepeat";
    case QSSGRenderTextureCoordOp::Repeat:
        return "Repeat";
    case QSSGRenderTextureCoordOp::Unknown:
        return "Unknown";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

const char *QSSGBaseTypeHelpers::toString(QSSGRenderTextureFilterOp value)
{
    switch (value) {
    case QSSGRenderTextureFilterOp::Nearest:
        return "Nearest";
    case QSSGRenderTextureFilterOp::Linear:
        return "Linear";
    case QSSGRenderTextureFilterOp::None:
        return "None";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

QByteArray QSSGBaseTypeHelpers::toString(QSSGRenderSamplerType value)
{
    switch (value) {
    case QSSGRenderSamplerType::Unknown:
        return "unknown"_ba;
    case QSSGRenderSamplerType::Sampler2D:
        return "sampler2D"_ba;
    case QSSGRenderSamplerType::Sampler2DArray:
        return "sampler2DArray"_ba;
    case QSSGRenderSamplerType::Sampler3D:
        return "sampler3D"_ba;
    case QSSGRenderSamplerType::SamplerCube:
        return "samplerCube"_ba;
    case QSSGRenderSamplerType::SamplerCubeArray:
        return "samplerCubeArray"_ba;
    case QSSGRenderSamplerType::SamplerBuffer:
        return "samplerBuffer"_ba;
    }

    Q_UNREACHABLE_RETURN(QByteArray());
}

const char *QSSGBaseTypeHelpers::displayName(QSSGRenderTextureCubeFace face)
{
    switch (face) {
    case QSSGRenderTextureCubeFace::NegX:
        return "-X";
    case QSSGRenderTextureCubeFace::NegZ:
        return "-Z";
    case QSSGRenderTextureCubeFace::NegY:
        return "-Y";
    case QSSGRenderTextureCubeFace::PosY:
        return "+Y";
    case QSSGRenderTextureCubeFace::PosX:
        return "+X";
    case QSSGRenderTextureCubeFace::PosZ:
        return "+Z";
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

size_t QSSGBaseTypeHelpers::getSizeOfType(QSSGRenderComponentType type)
{
    switch (type) {
    case QSSGRenderComponentType::UnsignedInt8:
        return sizeof(quint8);
    case QSSGRenderComponentType::Int8:
        return sizeof(qint8);
    case QSSGRenderComponentType::UnsignedInt16:
        return sizeof(quint16);
    case QSSGRenderComponentType::Int16:
        return sizeof(qint16);
    case QSSGRenderComponentType::UnsignedInt32:
        return sizeof(quint32);
    case QSSGRenderComponentType::Int32:
        return sizeof(qint32);
    case QSSGRenderComponentType::UnsignedInt64:
        return sizeof(quint64);
    case QSSGRenderComponentType::Int64:
        return sizeof(qint64);
    case QSSGRenderComponentType::Float16:
        return sizeof(qfloat16);
    case QSSGRenderComponentType::Float32:
        return sizeof(float);
    case QSSGRenderComponentType::Float64:
        return sizeof(double);
    }
    Q_UNREACHABLE_RETURN(0);
}

QT_END_NAMESPACE
