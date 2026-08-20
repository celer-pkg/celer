// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_SHADER_KEY_H
#define QSSG_RENDER_SHADER_KEY_H

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

#include <QtQuick3DUtils/private/qssgdataref_p.h>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

QT_BEGIN_NAMESPACE
// We have an ever expanding set of properties we like to hash into one or more 32 bit
// quantities.
// Furthermore we would like this set of properties to be convertable to string
// So the shader cache file itself is somewhat human readable/diagnosable.
// To do this we create a set of objects that act as properties to the master shader key.
// These objects are tallied in order to figure out their actual offset into the shader key's
// data store. They are also run through in order to create the string shader cache key.

struct QSSGShaderKeyPropertyBase
{
    QByteArrayView name;
    quint32 offset;
    explicit constexpr QSSGShaderKeyPropertyBase(const char *inName = "") : name(inName), offset(0) {}
    quint32 getOffset() const { return offset; }
    void setOffset(quint32 of) { offset = of; }

    template<quint32 TBitWidth>
    quint32 getMaskTemplate() const
    {
        quint32 bit = offset % 32;
        quint32 startValue = (1 << TBitWidth) - 1;
        quint32 mask = startValue << bit;
        return mask;
    }

    quint32 getIdx() const { return offset / 32; }

protected:
    void internalToString(QByteArray &ioStr, const QByteArrayView &inBuffer) const
    {
        ioStr.append(name);
        ioStr.append('=');
        ioStr.append(inBuffer);
    }

    static void internalToString(QByteArray &ioStr, const QByteArrayView &name, bool inValue)
    {
        if (inValue) {
            ioStr.append(name);
            ioStr.append('=');
            ioStr.append(inValue ? QByteArrayView("true") : QByteArrayView("false"));
        }
    }
    static bool getBoolValue(const QByteArray& str, const QByteArrayView &name)
    {
        const int index = str.indexOf(name);
        if (index < 0)
            return false;
        const qsizetype nameLen = name.size();
        if (str[index + nameLen] != '=')
            return false;
        if (str.mid(index + nameLen + 1, 4) == QByteArrayView("true"))
            return true;
        return false;
    }
};

struct QSSGShaderKeyBoolean : public QSSGShaderKeyPropertyBase
{
    enum {
        BitWidth = 1,
    };

    explicit constexpr QSSGShaderKeyBoolean(const char *inName = "") : QSSGShaderKeyPropertyBase(inName) {}

    quint32 getMask() const { return getMaskTemplate<BitWidth>(); }
    void setValue(QSSGDataRef<quint32> inDataStore, bool inValue) const
    {
        const qint32 idx = qint32(getIdx());
        Q_ASSERT(idx >= 0 && idx <= INT32_MAX);
        Q_ASSERT(inDataStore.size() > idx);
        quint32 mask = getMask();
        quint32 &target = inDataStore[idx];
        if (inValue) {
            target = target | mask;
        } else {
            mask = ~mask;
            target = target & mask;
        }
    }

    bool getValue(QSSGDataView<quint32> inDataStore) const
    {
        quint32 idx = getIdx();
        quint32 mask = getMask();
        const quint32 &target = inDataStore[idx];
        return (target & mask) ? true : false;
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        bool isHigh = getValue(inKeySet);
        internalToString(ioStr, name, isHigh);
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        setValue(inKeySet, getBoolValue(ioStr, name));
    }
};

template<quint32 TBitWidth>
struct QSSGShaderKeyUnsigned : public QSSGShaderKeyPropertyBase
{
    enum {
        BitWidth = TBitWidth,
    };
    explicit constexpr QSSGShaderKeyUnsigned(const char *inName = "") : QSSGShaderKeyPropertyBase(inName) {}
    quint32 getMask() const { return getMaskTemplate<BitWidth>(); }
    void setValue(QSSGDataRef<quint32> inDataStore, quint32 inValue) const
    {
        quint32 startValue = (1 << TBitWidth) - 1;
        // Ensure inValue is within range of bit width.
        inValue = inValue & startValue;
        quint32 bit = offset % 32;
        quint32 mask = getMask();
        quint32 idx = getIdx();
        inValue = inValue << bit;
        quint32 &target = inDataStore[idx];
        // Get rid of existing value
        quint32 inverseMask = ~mask;
        target = target & inverseMask;
        target = target | inValue;
    }

    quint32 getValue(QSSGDataView<quint32> inDataStore) const
    {
        quint32 idx = getIdx();
        quint32 bit = offset % 32;
        quint32 mask = getMask();
        const quint32 &target = inDataStore[idx];

        quint32 retval = target & mask;
        retval = retval >> bit;
        return retval;
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        quint32 value = getValue(inKeySet);
        char buf[64];
        memset(buf, 0, sizeof (buf));
        toStr(value, toDataRef(buf, 64));
        internalToString(ioStr, buf);
    }

    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const qsizetype strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=val */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const QByteArray s = ioStr.right(ioStr.size() - strOffset - nameLen - 1);
            int i = 0;
            while (QChar(QLatin1Char(s[i])).isDigit())
                i++;
            const quint32 value = s.left(i).toInt();
            setValue(inKeySet, value);
        }
    }

private:
    static quint32 toStr(quint32 item, QSSGDataRef<char> buffer)
    {
        // hope the buffer is big enough...
        return static_cast<quint32>(::snprintf(buffer.begin(), buffer.size(), "%u", item));
    }
};

struct QSSGShaderKeyTextureChannel : public QSSGShaderKeyUnsigned<2>
{
    enum TexturChannelBits {
        R = 0,
        G = 1,
        B = 2,
        A = 3,
    };
    explicit QSSGShaderKeyTextureChannel(const char *inName = "") : QSSGShaderKeyUnsigned<2>(inName) {}

    TexturChannelBits getTextureChannel(QSSGDataView<quint32> inKeySet) const
    {
        return TexturChannelBits(getValue(inKeySet));
    }

    void setTextureChannel(TexturChannelBits channel, QSSGDataRef<quint32> inKeySet)
    {
        setValue(inKeySet, quint32(channel));
    }
    static constexpr char textureChannelToChar[4] = {
        'R',
        'G',
        'B',
        'A'
    };
    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append('=');
        ioStr.append(textureChannelToChar[getTextureChannel(inKeySet)]);
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const qsizetype strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=ch */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const char ch = ioStr[strOffset + nameLen + 1];
            if (ch == 'R')
                setValue(inKeySet, TexturChannelBits::R);
            else if (ch == 'G')
                setValue(inKeySet, TexturChannelBits::G);
            else if (ch == 'B')
                setValue(inKeySet, TexturChannelBits::B);
            else if (ch == 'A')
                setValue(inKeySet, TexturChannelBits::A);
        }
    }
};

struct QSSGShaderKeyImageMap : public QSSGShaderKeyUnsigned<7>
{
    enum ImageMapBits {
        Enabled = 1 << 0,
        EnvMap = 1 << 1,
        LightProbe = 1 << 2,
        Identity = 1 << 3,
        UsesUV1 = 1 << 4,
        Linear = 1 << 5,
        PreMultipliedAlpha = 1 << 6,
    };

    explicit QSSGShaderKeyImageMap(const char *inName = "") : QSSGShaderKeyUnsigned<7>(inName) {}

    bool getBitValue(ImageMapBits imageBit, QSSGDataView<quint32> inKeySet) const
    {
        return (getValue(inKeySet) & imageBit) ? true : false;
    }

    void setBitValue(ImageMapBits imageBit, bool inValue, QSSGDataRef<quint32> inKeySet)
    {
        quint32 theValue = getValue(inKeySet);
        quint32 mask = imageBit;
        if (inValue) {
            theValue = theValue | mask;
        } else {
            mask = ~mask;
            theValue = theValue & mask;
        }
        setValue(inKeySet, theValue);
    }

    bool isEnabled(QSSGDataView<quint32> inKeySet) const { return getBitValue(Enabled, inKeySet); }
    void setEnabled(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(Enabled, val, inKeySet); }

    bool isEnvMap(QSSGDataView<quint32> inKeySet) const { return getBitValue(EnvMap, inKeySet); }
    void setEnvMap(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(EnvMap, val, inKeySet); }

    bool isLightProbe(QSSGDataView<quint32> inKeySet) const { return getBitValue(LightProbe, inKeySet); }
    void setLightProbe(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(LightProbe, val, inKeySet); }

    bool isIdentityTransform(QSSGDataView<quint32> inKeySet) const { return getBitValue(Identity, inKeySet); }
    void setIdentityTransform(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(Identity, val, inKeySet); }

    bool isUsingUV1(QSSGDataView<quint32> inKeySet) const { return getBitValue(UsesUV1, inKeySet); }
    void setUsesUV1(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(UsesUV1, val, inKeySet); }

    bool isLinear(QSSGDataView<quint32> inKeySet) const { return getBitValue(Linear, inKeySet); }
    void setLinear(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(Linear, val, inKeySet); }

    bool isPreMultipliedAlpha(QSSGDataView<quint32> inKeySet) const { return getBitValue(PreMultipliedAlpha, inKeySet); }
    void setPreMultipliedAlpha(QSSGDataRef<quint32> inKeySet, bool val) { setBitValue(PreMultipliedAlpha, val, inKeySet); }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append(QByteArrayView("={"));
        internalToString(ioStr, QByteArrayView("enabled"), isEnabled(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("envMap"), isEnvMap(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("lightProbe"), isLightProbe(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("identity"), isIdentityTransform(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("usesUV1"), isUsingUV1(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("linear"), isLinear(inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("preMultipliedAlpha"), isPreMultipliedAlpha(inKeySet));
        ioStr.append('}');
    }
};

struct QSSGShaderKeySpecularModel : QSSGShaderKeyUnsigned<2>
{
    explicit QSSGShaderKeySpecularModel(const char *inName = "") : QSSGShaderKeyUnsigned<2>(inName) {}

    void setSpecularModel(QSSGDataRef<quint32> inKeySet, QSSGRenderDefaultMaterial::MaterialSpecularModel inModel)
    {
        setValue(inKeySet, quint32(inModel));
    }

    QSSGRenderDefaultMaterial::MaterialSpecularModel getSpecularModel(QSSGDataView<quint32> inKeySet) const
    {
        return static_cast<QSSGRenderDefaultMaterial::MaterialSpecularModel>(getValue(inKeySet));
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append('=');
        switch (getSpecularModel(inKeySet)) {
        case QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX:
            ioStr.append(QByteArrayView("SchlickGGX"));
            break;
        case QSSGRenderDefaultMaterial::MaterialSpecularModel::BlinnPhong:
            ioStr.append(QByteArrayView("BlinnPhong"));
            break;
        }
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const int strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=specularMode; */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const int codeOffsetBegin = strOffset + nameLen + 1;
            int codeOffset = 0;
            while (ioStr[codeOffsetBegin + codeOffset] != ';')
                codeOffset++;
            const QByteArray val = ioStr.mid(codeOffsetBegin, codeOffset);
            if (val == QByteArrayView("SchlickGGX"))
                setSpecularModel(inKeySet, QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX);
            if (val == QByteArrayView("BlinnPhong"))
                setSpecularModel(inKeySet, QSSGRenderDefaultMaterial::MaterialSpecularModel::BlinnPhong);
        }
    }
};

struct QSSGShaderKeyDiffuseModel : QSSGShaderKeyUnsigned<2>
{
    explicit QSSGShaderKeyDiffuseModel(const char *inName = "") : QSSGShaderKeyUnsigned<2>(inName) {}

    void setDiffuseModel(QSSGDataRef<quint32> inKeySet, QSSGRenderDefaultMaterial::MaterialDiffuseModel inModel)
    {
        setValue(inKeySet, quint32(inModel));
    }

    QSSGRenderDefaultMaterial::MaterialDiffuseModel getDiffuseModel(QSSGDataView<quint32> inKeySet) const
    {
        return static_cast<QSSGRenderDefaultMaterial::MaterialDiffuseModel>(getValue(inKeySet));
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append('=');
        switch (getDiffuseModel(inKeySet)) {
        case QSSGRenderDefaultMaterial::MaterialDiffuseModel::Burley:
            ioStr.append(QByteArrayView("Burley"));
            break;
        case QSSGRenderDefaultMaterial::MaterialDiffuseModel::Lambert:
            ioStr.append(QByteArrayView("Lambert"));
            break;
        case QSSGRenderDefaultMaterial::MaterialDiffuseModel::LambertWrap:
            ioStr.append(QByteArrayView("LambertWrap"));
            break;
        }
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const int strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=specularMode; */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const int codeOffsetBegin = strOffset + nameLen + 1;
            int codeOffset = 0;
            while (ioStr[codeOffsetBegin + codeOffset] != ';')
                codeOffset++;
            const QByteArray val = ioStr.mid(codeOffsetBegin, codeOffset);
            if (val == QByteArrayView("Burley"))
                setDiffuseModel(inKeySet, QSSGRenderDefaultMaterial::MaterialDiffuseModel::Burley);
            if (val == QByteArrayView("Lambert"))
                setDiffuseModel(inKeySet, QSSGRenderDefaultMaterial::MaterialDiffuseModel::Lambert);
            if (val == QByteArrayView("LambertWrap"))
                setDiffuseModel(inKeySet, QSSGRenderDefaultMaterial::MaterialDiffuseModel::LambertWrap);
        }
    }
};

struct QSSGShaderKeyShadowSoftness : QSSGShaderKeyUnsigned<2>
{
    // possible values:
    //  SoftShadowQuality::Hard  : 00
    //  SoftShadowQuality::PCF4  : 01
    //  SoftShadowQuality::PCF8  : 10
    //  SoftShadowQuality::PCF16 : 11
    //  All others map to PCF16

    explicit QSSGShaderKeyShadowSoftness(const char *inName = "") : QSSGShaderKeyUnsigned<2>(inName) {}

    void setShadowSoftness(QSSGDataRef<quint32> inKeySet, QSSGRenderLight::SoftShadowQuality inQuality)
    {
        quint32 val = 0;
        switch (inQuality) {
        case QSSGRenderLight::SoftShadowQuality::Hard:
            val = 0;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF4:
            val = 1;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF8:
            val = 2;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF16:
        default:
            val = 3;
            break;
        }
        setValue(inKeySet, val);
    }

    QSSGRenderLight::SoftShadowQuality getShadowSoftness(QSSGDataView<quint32> inKeySet) const
    {
        quint32 val = getValue(inKeySet);
        switch (val) {
        case 0:
            return QSSGRenderLight::SoftShadowQuality::Hard;
        case 1:
            return QSSGRenderLight::SoftShadowQuality::PCF4;
        case 2:
            return QSSGRenderLight::SoftShadowQuality::PCF8;
        case 3:
        default:
            return QSSGRenderLight::SoftShadowQuality::PCF16;
        }
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append('=');
        switch (getShadowSoftness(inKeySet)) {
        case QSSGRenderLight::SoftShadowQuality::Hard:
            ioStr.append(QByteArrayView("Hard"));
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF4:
            ioStr.append(QByteArrayView("PCF4"));
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF8:
            ioStr.append(QByteArrayView("PCF8"));
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF16:
        case QSSGRenderLight::SoftShadowQuality::PCF32:
        case QSSGRenderLight::SoftShadowQuality::PCF64:
            ioStr.append(QByteArrayView("PCF16"));
            break;
        }
    }

    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const int strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=shadowSoftness; */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const int codeOffsetBegin = strOffset + nameLen + 1;
            int codeOffset = 0;
            while (ioStr[codeOffsetBegin + codeOffset] != ';')
                codeOffset++;
            const QByteArray val = ioStr.mid(codeOffsetBegin, codeOffset);
            if (val == QByteArrayView("Hard"))
                setShadowSoftness(inKeySet, QSSGRenderLight::SoftShadowQuality::Hard);
            if (val == QByteArrayView("PCF4"))
                setShadowSoftness(inKeySet, QSSGRenderLight::SoftShadowQuality::PCF4);
            if (val == QByteArrayView("PCF8"))
                setShadowSoftness(inKeySet, QSSGRenderLight::SoftShadowQuality::PCF8);
            if (val == QByteArrayView("PCF16"))
                setShadowSoftness(inKeySet, QSSGRenderLight::SoftShadowQuality::PCF16);
        }
    }

};

struct QSSGShaderKeyAlphaMode : QSSGShaderKeyUnsigned<2>
{
    explicit QSSGShaderKeyAlphaMode(const char *inName = "") : QSSGShaderKeyUnsigned<2>(inName) {}

    void setAlphaMode(QSSGDataRef<quint32> inKeySet, QSSGRenderDefaultMaterial::MaterialAlphaMode inMode)
    {
        setValue(inKeySet, quint32(inMode));
    }

    QSSGRenderDefaultMaterial::MaterialAlphaMode getAlphaMode(QSSGDataView<quint32> inKeySet) const
    {
        return static_cast<QSSGRenderDefaultMaterial::MaterialAlphaMode>(getValue(inKeySet));
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append('=');
        switch (getAlphaMode(inKeySet)) {
        case QSSGRenderDefaultMaterial::MaterialAlphaMode::Default:
            ioStr.append(QByteArrayView("Default"));
            break;
        case QSSGRenderDefaultMaterial::MaterialAlphaMode::Mask:
            ioStr.append(QByteArrayView("Mask"));
            break;
        case QSSGRenderDefaultMaterial::MaterialAlphaMode::Blend:
            ioStr.append(QByteArrayView("Blend"));
            break;
        case QSSGRenderDefaultMaterial::MaterialAlphaMode::Opaque:
            ioStr.append(QByteArrayView("Opaque"));
            break;
        }
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const qsizetype strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name=alphaMode; */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            const int codeOffsetBegin = strOffset + nameLen + 1;
            int codeOffset = 0;
            while (ioStr[codeOffsetBegin + codeOffset] != ';')
                codeOffset++;
            const QByteArray val = ioStr.mid(codeOffsetBegin, codeOffset);
            if (val == QByteArrayView("Default"))
                setAlphaMode(inKeySet, QSSGRenderDefaultMaterial::MaterialAlphaMode::Default);
            if (val == QByteArrayView("Mask"))
                setAlphaMode(inKeySet, QSSGRenderDefaultMaterial::MaterialAlphaMode::Mask);
            if (val == QByteArrayView("Blend"))
                setAlphaMode(inKeySet, QSSGRenderDefaultMaterial::MaterialAlphaMode::Blend);
            if (val == QByteArrayView("Opaque"))
                setAlphaMode(inKeySet, QSSGRenderDefaultMaterial::MaterialAlphaMode::Opaque);
        }
    }
};

struct QSSGShaderKeyVertexAttribute : public QSSGShaderKeyUnsigned<9>
{
    enum VertexAttributeBits {
        Position = 1 << 0,
        Normal = 1 << 1,
        TexCoord0 = 1 << 2,
        TexCoord1 = 1 << 3,
        Tangent = 1 << 4,
        Binormal = 1 << 5,
        Color = 1 << 6,
        JointAndWeight = 1 << 7,
        TexCoordLightmap = 1 << 8
    };

    explicit QSSGShaderKeyVertexAttribute(const char *inName = "") : QSSGShaderKeyUnsigned<9>(inName) {}

    bool getBitValue(VertexAttributeBits bit, QSSGDataView<quint32> inKeySet) const
    {
        return (getValue(inKeySet) & bit) ? true : false;
    }
    void setBitValue(VertexAttributeBits bit, QSSGDataRef<quint32> inKeySet, bool value) const
    {
        quint32 v = getValue(inKeySet);
        v = value ? (v | bit) : (v & ~bit);
        setValue(inKeySet, v);
    }

    void toString(QByteArray &ioStr, QSSGDataView<quint32> inKeySet) const
    {
        ioStr.append(name);
        ioStr.append(QByteArrayView("={"));
        internalToString(ioStr, QByteArrayView("position"), getBitValue(Position, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("normal"), getBitValue(Normal, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("texcoord0"), getBitValue(TexCoord0, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("texcoord1"), getBitValue(TexCoord1, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("tangent"), getBitValue(Tangent, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("binormal"), getBitValue(Binormal, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("color"), getBitValue(Color, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("texcoordlightmap"), getBitValue(TexCoordLightmap, inKeySet));
        ioStr.append(';');
        internalToString(ioStr, QByteArrayView("joint&weight"), getBitValue(JointAndWeight, inKeySet));
        ioStr.append('}');
    }
    void fromString(const QByteArray &ioStr, QSSGDataRef<quint32> inKeySet)
    {
        const qsizetype nameLen = name.size();
        const qsizetype strOffset = ioStr.indexOf(name);
        if (strOffset >= 0) {
            /* The key is stored as name={;;;;;;} */
            if (ioStr[strOffset + nameLen] != '=')
                return;
            if (ioStr[strOffset + nameLen + 1] != '{')
                return;
            const int codeOffsetBegin = strOffset + nameLen + 2;
            int codeOffset = 0;
            while (ioStr[codeOffsetBegin + codeOffset] != '}')
                codeOffset++;
            const QByteArray val = ioStr.mid(codeOffsetBegin, codeOffset);
            const QVector<QByteArray> list = val.split(';');
            if (list.size() != 8)
                return;
            setBitValue(Position, inKeySet, getBoolValue(list[0], QByteArrayView("position")));
            setBitValue(Normal, inKeySet, getBoolValue(list[1], QByteArrayView("normal")));
            setBitValue(TexCoord0, inKeySet, getBoolValue(list[2], QByteArrayView("texcoord0")));
            setBitValue(TexCoord1, inKeySet, getBoolValue(list[3], QByteArrayView("texcoord1")));
            setBitValue(Tangent, inKeySet, getBoolValue(list[4], QByteArrayView("tangent")));
            setBitValue(Binormal, inKeySet, getBoolValue(list[5], QByteArrayView("binormal")));
            setBitValue(Color, inKeySet, getBoolValue(list[6], QByteArrayView("color")));
            setBitValue(TexCoordLightmap, inKeySet, getBoolValue(list[7], QByteArrayView("texcoordlightmap")));
        }
    }
};

struct QSSGShaderDefaultMaterialKeyProperties
{
    enum {
        SingleChannelImageCount = 13,
    };
    enum ImageMapNames {
        DiffuseMap = 0,
        BumpMap,
        SpecularMap,
        NormalMap,
        ClearcoatNormalMap,
        // single channel images
        OpacityMap,
        RoughnessMap,
        MetalnessMap,
        OcclusionMap,
        TranslucencyMap,
        HeightMap,
        ClearcoatMap,
        ClearcoatRoughnessMap,
        TransmissionMap,
        ThicknessMap,
        BaseColorMap,
        SpecularAmountMap,
        EmissiveMap,

        ImageMapCount,
        SingleChannelImagesFirst = OpacityMap
    };
    enum ImageChannelNames {
        OpacityChannel = 0,
        RoughnessChannel,
        MetalnessChannel,
        OcclusionChannel,
        TranslucencyChannel,
        HeightChannel,
        ClearcoatChannel,
        ClearcoatRoughnessChannel,
        TransmissionChannel,
        ThicknessChannel,
        BaseColorChannel,
        SpecularAmountChannel,
        EmissiveChannel,
    };

    QSSGShaderKeyBoolean m_hasLighting;
    QSSGShaderKeyBoolean m_hasPunctualLights;
    QSSGShaderKeyBoolean m_hasShadows;
    QSSGShaderKeyBoolean m_hasIbl;
    QSSGShaderKeyBoolean m_specularEnabled;
    QSSGShaderKeyBoolean m_fresnelScaleBiasEnabled;
    QSSGShaderKeyBoolean m_clearcoatFresnelScaleBiasEnabled;
    QSSGShaderKeyBoolean m_fresnelEnabled;
    QSSGShaderKeyBoolean m_baseColorSingleChannelEnabled;
    QSSGShaderKeyBoolean m_specularSingleChannelEnabled;
    QSSGShaderKeyBoolean m_emissiveSingleChannelEnabled;
    QSSGShaderKeyBoolean m_invertOpacityMapValue;
    QSSGShaderKeyBoolean m_vertexColorsEnabled;
    QSSGShaderKeyBoolean m_vertexColorsMaskEnabled;
    QSSGShaderKeyUnsigned<16> m_vertexColorRedMask;
    QSSGShaderKeyUnsigned<16> m_vertexColorGreenMask;
    QSSGShaderKeyUnsigned<16> m_vertexColorBlueMask;
    QSSGShaderKeyUnsigned<16> m_vertexColorAlphaMask;
    QSSGShaderKeySpecularModel m_specularModel;
    QSSGShaderKeyDiffuseModel m_diffuseModel;
    QSSGShaderKeyImageMap m_imageMaps[ImageMapCount];
    QSSGShaderKeyTextureChannel m_textureChannels[SingleChannelImageCount];
    QSSGShaderKeyUnsigned<16> m_boneCount;
    QSSGShaderKeyBoolean m_isDoubleSided;
    QSSGShaderKeyBoolean m_overridesPosition;
    QSSGShaderKeyBoolean m_usesProjectionMatrix;
    QSSGShaderKeyBoolean m_usesInverseProjectionMatrix;
    QSSGShaderKeyBoolean m_usesPointsTopology;
    QSSGShaderKeyBoolean m_usesVarColor;
    QSSGShaderKeyAlphaMode m_alphaMode;
    QSSGShaderKeyVertexAttribute m_vertexAttributes;
    QSSGShaderKeyBoolean m_usesFloatJointIndices;
    qsizetype m_stringBufferSizeHint = 0;
    QSSGShaderKeyBoolean m_usesInstancing;
    QSSGShaderKeyUnsigned<8> m_targetCount;
    QSSGShaderKeyUnsigned<8> m_targetPositionOffset;
    QSSGShaderKeyUnsigned<8> m_targetNormalOffset;
    QSSGShaderKeyUnsigned<8> m_targetTangentOffset;
    QSSGShaderKeyUnsigned<8> m_targetBinormalOffset;
    QSSGShaderKeyUnsigned<8> m_targetTexCoord0Offset;
    QSSGShaderKeyUnsigned<8> m_targetTexCoord1Offset;
    QSSGShaderKeyUnsigned<8> m_targetColorOffset;
    QSSGShaderKeyBoolean m_blendParticles;
    QSSGShaderKeyBoolean m_clearcoatEnabled;
    QSSGShaderKeyBoolean m_transmissionEnabled;
    QSSGShaderKeyBoolean m_specularAAEnabled;
    QSSGShaderKeyBoolean m_lightmapEnabled;
    QSSGShaderKeyBoolean m_specularGlossyEnabled;
    QSSGShaderKeyBoolean m_metallicRoughnessEnabled;
    QSSGShaderKeyUnsigned<4> m_debugMode;
    QSSGShaderKeyBoolean m_fogEnabled;
    QSSGShaderKeyUnsigned<3> m_viewCount;
    QSSGShaderKeyBoolean m_usesViewIndex;
    QSSGShaderKeyBoolean m_oitMSAA;
    QSSGShaderKeyUnsigned<3> m_orderIndependentTransparency;
    QSSGShaderKeyShadowSoftness m_shadowSoftness;

    QSSGShaderDefaultMaterialKeyProperties()
        : m_hasLighting("hasLighting")
        , m_hasPunctualLights("hasPunctualLights")
        , m_hasShadows("hasShadows")
        , m_hasIbl("hasIbl")
        , m_specularEnabled("specularEnabled")
        , m_fresnelScaleBiasEnabled("fresnelScaleBiasEnabled")
        , m_clearcoatFresnelScaleBiasEnabled("clearcoatFresnelScaleBiasEnabled")
        , m_fresnelEnabled("fresnelEnabled")
        , m_baseColorSingleChannelEnabled("baseColorSingleChannelEnabled")
        , m_specularSingleChannelEnabled("specularSingleChannelEnabled")
        , m_emissiveSingleChannelEnabled("emissiveSingleChannelEnabled")
        , m_invertOpacityMapValue("invertOpacityMapValue")
        , m_vertexColorsEnabled("vertexColorsEnabled")
        , m_vertexColorsMaskEnabled("vertexColorsMaskEnabled")
        , m_vertexColorRedMask("vertexColorRedMask")
        , m_vertexColorGreenMask("vertexColorGreenMask")
        , m_vertexColorBlueMask("vertexColorBlueMask")
        , m_vertexColorAlphaMask("vertexColorAlphaMask")
        , m_specularModel("specularModel")
        , m_diffuseModel("diffuseModel")
        , m_boneCount("boneCount")
        , m_isDoubleSided("isDoubleSided")
        , m_overridesPosition("overridesPosition")
        , m_usesProjectionMatrix("usesProjectionMatrix")
        , m_usesInverseProjectionMatrix("usesInverseProjectionMatrix")
        , m_usesPointsTopology("usesPointsTopology")
        , m_usesVarColor("usesVarColor")
        , m_alphaMode("alphaMode")
        , m_vertexAttributes("vertexAttributes")
        , m_usesFloatJointIndices("usesFloatJointIndices")
        , m_usesInstancing("usesInstancing")
        , m_targetCount("targetCount")
        , m_targetPositionOffset("targetPositionOffset")
        , m_targetNormalOffset("targetNormalOffset")
        , m_targetTangentOffset("targetTangentOffset")
        , m_targetBinormalOffset("targetBinormalOffset")
        , m_targetTexCoord0Offset("targetTexCoord0Offset")
        , m_targetTexCoord1Offset("targetTexCoord1Offset")
        , m_targetColorOffset("targetColorOffset")
        , m_blendParticles("blendParticles")
        , m_clearcoatEnabled("clearcoatEnabled")
        , m_transmissionEnabled("transmissionEnabled")
        , m_specularAAEnabled("specularAAEnabled")
        , m_lightmapEnabled("lightmapEnabled")
        , m_specularGlossyEnabled("specularGlossyEnabled")
        , m_metallicRoughnessEnabled("metallicRoughnessEnabled")
        , m_debugMode("debugMode")
        , m_fogEnabled("fogEnabled")
        , m_viewCount("viewCount")
        , m_usesViewIndex("usesViewIndex")
        , m_oitMSAA("oitMSAA")
        , m_orderIndependentTransparency("orderIndependentTransparency")
        , m_shadowSoftness("shadowSoftness")
    {
        m_imageMaps[0].name = "diffuseMap";
        m_imageMaps[1].name = "emissiveMap";
        m_imageMaps[2].name = "specularMap";
        m_imageMaps[3].name = "baseColorMap";
        m_imageMaps[4].name = "bumpMap";
        m_imageMaps[5].name = "specularAmountMap";
        m_imageMaps[6].name = "normalMap";
        m_imageMaps[7].name = "clearcoatNormalMap";
        m_imageMaps[8].name = "opacityMap";
        m_imageMaps[9].name = "roughnessMap";
        m_imageMaps[10].name = "metalnessMap";
        m_imageMaps[11].name = "occlusionMap";
        m_imageMaps[12].name = "translucencyMap";
        m_imageMaps[13].name = "heightMap";
        m_imageMaps[14].name = "clearcoatMap";
        m_imageMaps[15].name = "clearcoatRoughnessMap";
        m_imageMaps[16].name = "transmissionMap";
        m_imageMaps[17].name = "thicknessMap";

        m_textureChannels[0].name = "opacityMap_channel";
        m_textureChannels[1].name = "roughnessMap_channel";
        m_textureChannels[2].name = "metalnessMap_channel";
        m_textureChannels[3].name = "occlusionMap_channel";
        m_textureChannels[4].name = "translucencyMap_channel";
        m_textureChannels[5].name = "heightMap_channel";
        m_textureChannels[6].name = "clearcoatMap_channel";
        m_textureChannels[7].name = "clearcoatRoughnessMap_channel";
        m_textureChannels[8].name = "transmissionMap_channel";
        m_textureChannels[9].name = "thicknessMap_channel";
        m_textureChannels[10].name = "baseColorMap_channel";
        m_textureChannels[11].name = "specularAmountMap_channel";
        m_textureChannels[12].name = "emissiveMap_channel";

        init();
    }

    template<typename TVisitor>
    void visitProperties(TVisitor &inVisitor)
    {
        inVisitor.visit(m_hasLighting);
        inVisitor.visit(m_hasPunctualLights);
        inVisitor.visit(m_hasShadows);
        inVisitor.visit(m_hasIbl);
        inVisitor.visit(m_specularEnabled);
        inVisitor.visit(m_fresnelEnabled);
        inVisitor.visit(m_fresnelScaleBiasEnabled);
        inVisitor.visit(m_clearcoatFresnelScaleBiasEnabled);
        inVisitor.visit(m_baseColorSingleChannelEnabled);
        inVisitor.visit(m_specularSingleChannelEnabled);
        inVisitor.visit(m_emissiveSingleChannelEnabled);
        inVisitor.visit(m_invertOpacityMapValue);
        inVisitor.visit(m_vertexColorsEnabled);
        inVisitor.visit(m_vertexColorsMaskEnabled);
        inVisitor.visit(m_vertexColorRedMask);
        inVisitor.visit(m_vertexColorGreenMask);
        inVisitor.visit(m_vertexColorBlueMask);
        inVisitor.visit(m_vertexColorAlphaMask);
        inVisitor.visit(m_specularModel);
        inVisitor.visit(m_diffuseModel);

        for (quint32 idx = 0, end = ImageMapCount; idx < end; ++idx)
            inVisitor.visit(m_imageMaps[idx]);

        for (auto &textureChannel : m_textureChannels)
            inVisitor.visit(textureChannel);

        inVisitor.visit(m_boneCount);
        inVisitor.visit(m_isDoubleSided);
        inVisitor.visit(m_overridesPosition);
        inVisitor.visit(m_usesProjectionMatrix);
        inVisitor.visit(m_usesInverseProjectionMatrix);
        inVisitor.visit(m_usesPointsTopology);
        inVisitor.visit(m_usesVarColor);
        inVisitor.visit(m_alphaMode);
        inVisitor.visit(m_vertexAttributes);
        inVisitor.visit(m_usesFloatJointIndices);
        inVisitor.visit(m_usesInstancing);
        inVisitor.visit(m_targetCount);
        inVisitor.visit(m_targetPositionOffset);
        inVisitor.visit(m_targetNormalOffset);
        inVisitor.visit(m_targetTangentOffset);
        inVisitor.visit(m_targetBinormalOffset);
        inVisitor.visit(m_targetTexCoord0Offset);
        inVisitor.visit(m_targetTexCoord1Offset);
        inVisitor.visit(m_targetColorOffset);
        inVisitor.visit(m_blendParticles);
        inVisitor.visit(m_clearcoatEnabled);
        inVisitor.visit(m_transmissionEnabled);
        inVisitor.visit(m_specularAAEnabled);
        inVisitor.visit(m_lightmapEnabled);
        inVisitor.visit(m_specularGlossyEnabled);
        inVisitor.visit(m_metallicRoughnessEnabled);
        inVisitor.visit(m_debugMode);
        inVisitor.visit(m_fogEnabled);
        inVisitor.visit(m_viewCount);
        inVisitor.visit(m_usesViewIndex);
        inVisitor.visit(m_oitMSAA);
        inVisitor.visit(m_orderIndependentTransparency);
        inVisitor.visit(m_shadowSoftness);
    }

    struct OffsetVisitor
    {
        quint32 m_offset;
        OffsetVisitor() : m_offset(0) {}
        template<typename TPropType>
        void visit(TPropType &inProp)
        {
            // if we cross the 32 bit border we just move
            // to the next dword.
            // This cost a few extra bits but prevents tedious errors like
            // loosing shader key bits because they got moved beyond the 32 border
            quint32 bit = m_offset % 32;
            if (bit + TPropType::BitWidth > 32) {
                m_offset += 32 - bit;
            }

            inProp.setOffset(m_offset);
            m_offset += TPropType::BitWidth;
        }
    };

    struct StringSizeVisitor
    {
        qsizetype size = 0;
        template<typename P>
        constexpr void visit(const P &prop)
        {
            size += prop.name.size();
        }
    };

    struct InitVisitor
    {
        OffsetVisitor offsetVisitor;
        StringSizeVisitor stringSizeVisitor;

        template<typename P>
        void visit(P &prop)
        {
            offsetVisitor.visit(prop);
            stringSizeVisitor.visit(prop);
        }
    };

    void init()
    {
        InitVisitor visitor;
        visitProperties(visitor);

        // If this assert fires, then the default material key needs more bits.
        Q_ASSERT(visitor.offsetVisitor.m_offset < 800);
        // This is so we can do some guestimate of how big the string buffer needs
        // to be to avoid doing a lot of allocations when concatenating the strings.
        m_stringBufferSizeHint = visitor.stringSizeVisitor.size;
    }
};

template <typename Props, size_t size>
struct QSSGShaderBaseMaterialKey
{
    enum {
        DataBufferSize = size,
    };
    quint32 m_dataBuffer[DataBufferSize];
    size_t m_featureSetHash;

    explicit QSSGShaderBaseMaterialKey(size_t inFeatureSetHash) : m_featureSetHash(inFeatureSetHash)
    {
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            m_dataBuffer[idx] = 0;
    }

    QSSGShaderBaseMaterialKey() : m_featureSetHash(0)
    {
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            m_dataBuffer[idx] = 0;
    }

    size_t hash() const
    {
        size_t retval = 0;
        for (size_t idx = 0; idx < DataBufferSize; ++idx)
            retval = retval ^ qHash(m_dataBuffer[idx]);
        return retval ^ m_featureSetHash;
    }

    bool operator==(const QSSGShaderBaseMaterialKey<Props, size> &other) const
    {
        bool retval = true;
        for (size_t idx = 0; idx < DataBufferSize && retval; ++idx)
            retval = m_dataBuffer[idx] == other.m_dataBuffer[idx];
        return retval && m_featureSetHash == other.m_featureSetHash;
    }

    // Cast operators to make getting properties easier.
    operator QSSGDataRef<quint32>() { return toDataRef(m_dataBuffer, DataBufferSize); }
    operator QSSGDataView<quint32>() const { return toDataView(m_dataBuffer, DataBufferSize); }

    struct StringVisitor
    {
        QByteArray &m_str;
        QSSGDataView<quint32> m_keyStore;
        StringVisitor(QByteArray &s, QSSGDataView<quint32> ks) : m_str(s), m_keyStore(ks) {}
        template<typename TPropType>
        void visit(const TPropType &prop)
        {
            const qsizetype originalSize = m_str.size();
            if (m_str.size())
                m_str.append(';');
            prop.toString(m_str, m_keyStore);
            // if the only thing we added was the semicolon
            // then nuke the semicolon
            if (originalSize && m_str.size() == (originalSize + 1))
                m_str.resize(originalSize);
        }
    };

    struct StringInVisitor
    {
        const QByteArray &m_str;
        QSSGDataRef<quint32> m_keyStore;
        StringInVisitor(const QByteArray &s, QSSGDataRef<quint32> ks) : m_str(s), m_keyStore(ks) {}

        template<typename TPropType>
        void visit(TPropType &prop)
        {
            prop.fromString(m_str, m_keyStore);
        }
    };

    void toString(QByteArray &ioString, const Props &inProperties) const
    {
        ioString.reserve(inProperties.m_stringBufferSizeHint);
        StringVisitor theVisitor(ioString, *this);
        const_cast<Props &>(inProperties).visitProperties(theVisitor);
    }
    void fromString(QByteArray &ioString, Props &inProperties)
    {
        StringInVisitor theVisitor(ioString, *this);
        inProperties.visitProperties(theVisitor);
    }
    QByteArray toByteArray() const
    {
        QByteArray ret;
        ret.resize(sizeof(m_dataBuffer));
        memcpy(ret.data(), m_dataBuffer, sizeof(m_dataBuffer));
        return ret;
    }
    bool fromByteArray(const QByteArray &data) const
    {
        if (data.size() != sizeof(m_dataBuffer))
            return false;
        memcpy((void *)m_dataBuffer, data.data(), sizeof(m_dataBuffer));
        return true;
    }
};

typedef QSSGShaderBaseMaterialKey<QSSGShaderDefaultMaterialKeyProperties, 25> QSSGShaderDefaultMaterialKey;

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGShaderDefaultMaterialKey>::value);


inline size_t qHash(const QSSGShaderDefaultMaterialKey &key)
{
    return key.hash();
}

QT_END_NAMESPACE

#endif
