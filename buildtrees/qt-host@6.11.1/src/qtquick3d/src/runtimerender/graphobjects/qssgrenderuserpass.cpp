// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderuserpass_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendercommands_p.h>

#include <QtCore/qregularexpression.h>

#include <ssg/qssgrendercontextcore.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcRenderUserPass, "qt.quick3d.runtimerender.renderuserpass")

static const QHash<const char*, const char*> AugmentMacros = {
    { "BASE_COLOR", "qt_diffuseColor" },
    { "ROUGHNESS", "qt_roughnessAmount" },
    { "METALNESS", "qt_metalnessAmount" },
    { "DIFFUSE_LIGHT", "global_diffuse_light.rgb" },
    { "SPECULAR_LIGHT", "global_specular_light" },
    { "EMISSIVE_LIGHT", "global_emission" },
    { "WORLD_NORMAL", "qt_world_normal" },
    { "WORLD_TANGENT", "qt_tangent" },
    { "WORLD_BINORMAL", "qt_binormal" },
    { "F0", "qt_f0" },
    { "F90", "qt_f90" }
};

// NOTE: While this class doesn't have graphics resources itself, it manages commands that do!
QSSGRenderUserPass::QSSGRenderUserPass()
    : QSSGRenderGraphObject(Type::RenderPass, FlagT(Flags::HasGraphicsResources))
{

}

QSSGRenderUserPass::~QSSGRenderUserPass()
{
    resetCommands();
}

bool QSSGRenderUserPass::isDirty(DirtyFlag flag) const
{
    return (flags & FlagT(flag)) != 0;
}

void QSSGRenderUserPass::markDirty(DirtyFlag flag)
{
    flags |= FlagT(flag);
}

void QSSGRenderUserPass::clearDirty(DirtyFlag flag)
{
    flags &= ~FlagT(flag);
}

QByteArray replaceAugmentMacros(const QByteArray &shaderCode)
{
    QString modifiedCode = QString::fromUtf8(shaderCode);
    for (auto it = AugmentMacros.constBegin(); it != AugmentMacros.constEnd(); ++it) {
        QString pattern = QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(QString::fromUtf8(it.key())));
        QRegularExpression regex(pattern);
        modifiedCode.replace(regex, QString::fromUtf8(it.value()));
    }
    return modifiedCode.toUtf8();
}

void QSSGRenderUserPass::finalizeShaders(const QSSGRenderContextInterface &ctx)
{
    Q_UNUSED(ctx);

    if (materialMode != AugmentMaterial)
        return;

    if (!isDirty(DirtyFlag::ShaderDirty))
        return;

    clearDirty(DirtyFlag::ShaderDirty);

    const bool hasShaderCode = (shaderAugmentation.hasUserAugmentation());

    if (hasShaderCode) {
        qCDebug(lcRenderUserPass) << "Finalizing shaders for user pass with augment shader code.";

        // Replace any instances of the AugmentMacros with the appropriate built-in variable names
        shaderAugmentation.preamble = replaceAugmentMacros(shaderAugmentation.preamble);
        shaderAugmentation.body = replaceAugmentMacros(shaderAugmentation.body);

        // Check if body needs certain features
        shaderAugmentation.needsBaseColor = shaderAugmentation.body.contains(AugmentMacros["BASE_COLOR"]);
        shaderAugmentation.needsRoughness = shaderAugmentation.body.contains(AugmentMacros["ROUGHNESS"]);
        shaderAugmentation.needsMetalness = shaderAugmentation.body.contains(AugmentMacros["METALNESS"]);
        shaderAugmentation.needsDiffuseLight = shaderAugmentation.body.contains(AugmentMacros["DIFFUSE_LIGHT"]);
        shaderAugmentation.needsSpecularLight = shaderAugmentation.body.contains(AugmentMacros["SPECULAR_LIGHT"]);
        shaderAugmentation.needsEmissiveLight = shaderAugmentation.body.contains(AugmentMacros["EMISSIVE_LIGHT"]);
        shaderAugmentation.needsWorldNormal = shaderAugmentation.body.contains(AugmentMacros["WORLD_NORMAL"]);
        shaderAugmentation.needsWorldTangent = shaderAugmentation.body.contains(AugmentMacros["WORLD_TANGENT"]);
        shaderAugmentation.needsWorldBinormal = shaderAugmentation.body.contains(AugmentMacros["WORLD_BINORMAL"]);
        shaderAugmentation.needsF0 = shaderAugmentation.body.contains(AugmentMacros["F0"]);
        shaderAugmentation.needsF90 = shaderAugmentation.body.contains(AugmentMacros["F90"]);

        m_state = State::Ready;

    } else {
        m_state = State::Error;

        qWarning() << "No augment shader code provided for user pass.";
    }

    // Finalize shaders for all commands
    qCDebug(lcRenderUserPass) << "Finalizing shaders for user pass commands";
}

void QSSGRenderUserPass::setDependencyIndex(quint32 index)
{
    m_dependencyIndex = index;
}

void QSSGRenderUserPass::resetCommands()
{
    qDeleteAll(commands);
    commands.clear();
}

QT_END_NAMESPACE
