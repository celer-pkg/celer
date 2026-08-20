// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERUSERPASS_P_H
#define QSSGRENDERUSERPASS_P_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QSSGCommand;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderUserPass : public QSSGRenderGraphObject
{
public:
    enum MaterialModes {
        OriginalMaterial,
        AugmentMaterial,
        OverrideMaterial
    };

    enum PassModes {
        UserPass,
        SkyboxPass,
        Item2DPass,
        InfiniteGridPass
    };

    enum class DirtyFlag : quint8
    {
        ShaderDirty = 0x1
    };
    using FlagT = std::underlying_type_t<DirtyFlag>;

    enum class State : quint8
    {
        None,
        Ready,
        Error,
    };

    static constexpr DirtyFlag DirtyMask { std::numeric_limits<FlagT>::max() };

    QSSGRenderUserPass();
    ~QSSGRenderUserPass();

    bool isDirty(DirtyFlag flag = DirtyMask) const;
    void markDirty(DirtyFlag flag);
    void clearDirty(DirtyFlag flag);

    bool isReady() const { return (m_state == State::Ready); }

    void finalizeShaders(const QSSGRenderContextInterface &ctx);
    void setDependencyIndex(quint32 index);

    QVector<QSSGCommand *> commands;

    QString key;
    QColor clearColor = Qt::black;
    QRhiTextureRenderTarget::Flags renderTargetFlags = {};

    // Material Mode
    MaterialModes materialMode = OriginalMaterial;
    QSSGRenderGraphObject *overrideMaterial = nullptr;
    QSSGUserShaderAugmentation shaderAugmentation;
    QRhiDepthStencilClearValue depthStencilClearValue = { };
    PassModes passMode = UserPass;

    FlagT m_dirtyFlags = 0;
    State m_state = State::None;
    quint32 m_dependencyIndex = 0;

    void resetCommands();
};

using QSSGRenderUserPassPtr = std::shared_ptr<QSSGRenderUserPass>;

QT_END_NAMESPACE

#endif // QSSGRENDERUSERPASS_P_H
