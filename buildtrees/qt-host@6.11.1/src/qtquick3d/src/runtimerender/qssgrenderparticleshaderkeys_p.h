// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_PARTICLE_SHADER_KEY_H
#define QSSG_RENDER_PARTICLE_SHADER_KEY_H

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

#include "qssgrendershaderkeys_p.h"

QT_BEGIN_NAMESPACE

struct QSSGShaderParticleMaterialKeyProperties
{
    qsizetype m_stringBufferSizeHint = 0;
    QSSGShaderKeyBoolean m_isSpriteLinear;
    QSSGShaderKeyBoolean m_isColorTableLinear;
    QSSGShaderKeyBoolean m_hasLighting;
    QSSGShaderKeyBoolean m_isLineParticle;
    QSSGShaderKeyBoolean m_isMapped;
    QSSGShaderKeyBoolean m_isAnimated;
    QSSGShaderKeyBoolean m_oitMSAA;
    QSSGShaderKeyUnsigned<3> m_viewCount;
    QSSGShaderKeyUnsigned<3> m_orderIndependentTransparency;

    QSSGShaderParticleMaterialKeyProperties()
        : m_isSpriteLinear("isSpriteLinear")
        , m_isColorTableLinear("isColorTableLinear")
        , m_hasLighting("hasLighting")
        , m_isLineParticle("isLineParticle")
        , m_isMapped("isMapped")
        , m_isAnimated("isAnimated")
        , m_oitMSAA("oitMSAA")
        , m_viewCount("viewCount")
        , m_orderIndependentTransparency("orderIndependentTransparency")
    {
        init();
    }

    template<typename TVisitor>
    void visitProperties(TVisitor &inVisitor)
    {
        inVisitor.visit(m_isSpriteLinear);
        inVisitor.visit(m_isColorTableLinear);
        inVisitor.visit(m_hasLighting);
        inVisitor.visit(m_isLineParticle);
        inVisitor.visit(m_isMapped);
        inVisitor.visit(m_isAnimated);
        inVisitor.visit(m_oitMSAA);
        inVisitor.visit(m_viewCount);
        inVisitor.visit(m_orderIndependentTransparency);
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

        // If this assert fires, then the material key needs more bits.
        Q_ASSERT(visitor.offsetVisitor.m_offset < 32);
        // This is so we can do some guestimate of how big the string buffer needs
        // to be to avoid doing a lot of allocations when concatenating the strings.
        m_stringBufferSizeHint = visitor.stringSizeVisitor.size;
    }
};

typedef QSSGShaderBaseMaterialKey<QSSGShaderParticleMaterialKeyProperties, 1> QSSGShaderParticleMaterialKey;

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGShaderParticleMaterialKey>::value);

inline size_t qHash(const QSSGShaderParticleMaterialKey &key)
{
    return key.hash();
}

QT_END_NAMESPACE

#endif
