// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderpass_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderuserpass_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuick3DRenderPass, "qt.quick3d.renderpass")

/*!
    \qmltype RenderPass
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Defines a custom render pass for rendering 3D content.
    \since 6.11

    A RenderPass defines a rendering step and the render target it writes
    into. It is the combination of three concerns:

    \list
    \li \b {Where to render} — one or more output textures declared as
        \l ColorAttachment or \l DepthTextureAttachment commands in
        \l {RenderPass::commands}{commands}.
    \li \b {What to render} — which scene objects the pass draws,
        controlled by \l RenderablesFilter commands. Sub-divisions of
        work within the same render target are described using
        \l SubRenderPass commands.
    \li \b {How to render} — the \l materialMode and any
        \l PipelineStateOverride commands.
    \endlist

    A RenderPass becomes active for a scene when it is placed as a child
    of a \l View3D or a \l Node.

    The following example sets up a simple off-screen pass that renders
    all scene objects into a custom texture, which can then be consumed
    by a material or a \l SimpleQuadRenderer:

    \qml
    import QtQuick3D

    View3D {
        // Declare the off-screen color buffer
        RenderPassTexture {
            id: myColorTexture
            format: RenderPassTexture.RGBA8
        }

        // The render pass: where + what + how
        RenderPass {
            id: myRenderPass
            commands: [
                // Where: attach the texture as the color output
                ColorAttachment {
                    name: "color0"
                    target: myColorTexture
                },
                // What: render all objects into it
                RenderablesFilter {
                    renderableTypes: RenderablesFilter.Opaque | RenderablesFilter.Transparent
                }
            ]
        }
    }
    \endqml

    \section1 Exposing data to the shaders

    As with Effects and Custom Materials, the RenderPass will expose and
    update user-defined properties to the shaders automatically. Any QML
    properties declared on a RenderPass subtype will be available as
    uniforms in the shader.

    \sa SubRenderPass, RenderOutputProvider, RenderablesFilter
*/

QQuick3DRenderPass::QQuick3DRenderPass(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::RenderPass, QQuick3DObjectPrivate::Flags::RequiresSecondaryUpdate)), parent)
    , QQuick3DPropertyChangedTracker(this, QQuick3DSuperClassInfo<QQuick3DRenderPass>())
{
}

QSSGRenderGraphObject *QQuick3DRenderPass::updateSpatialNode(QSSGRenderGraphObject *node)
{
    QSSGRenderUserPass *renderPassNode = static_cast<QSSGRenderUserPass *>(node);

    bool newBackendNode = false;
    if (!renderPassNode) {
        renderPassNode = new QSSGRenderUserPass;
        newBackendNode = true;
    }

    const bool fullUpdate = newBackendNode  || (m_dirtyAttributes & Dirty::TextureDirty) || (m_dirtyAttributes & CommandsDirty);

    auto &shaderAugmentation = renderPassNode->shaderAugmentation;
    auto &uniformProps = shaderAugmentation.propertyUniforms;

    if (fullUpdate) {
        markAllDirty();

        // Properties -> uniforms.
        // NOTE: Calling extractProperties clears existing properties
        extractProperties(uniformProps);

        // Commands
        renderPassNode->resetCommands();
        clearDirty(Dirty::CommandsDirty);
        for (QQuick3DShaderUtilsRenderCommand *command : std::as_const(m_commands)) {
            if (auto *cmd = command->cloneCommand())
                renderPassNode->commands.push_back(cmd);
            else
                markDirty(CommandsDirty, true); // Try again next time
        }
    }

    // Update the property values
    if (m_dirtyAttributes & Dirty::PropertyDirty) {
        for (const auto &prop : std::as_const(uniformProps)) {
            auto p = metaObject()->property(prop.pid);
            if (Q_LIKELY(p.isValid())) {
                QVariant v = p.read(this);
                if (v.isValid()) {
                    if (v.metaType().id() == qMetaTypeId<QQuick3DTexture *>()) {
                        QQuick3DTexture *tex = v.value<QQuick3DTexture *>();
                        auto *po = QQuick3DObjectPrivate::get(tex);
                        QSSGRenderImage *ri = static_cast<QSSGRenderImage *>(po->spatialNode);
                        prop.value = QVariant::fromValue(ri);
                    } else {
                        prop.value = v;
                    }
                }
            }
        }

        clearDirty(Dirty(Dirty::PropertyDirty | Dirty::TextureDirty));
    }

    // Clear Dirty
    if (m_dirtyAttributes & Dirty::ClearDirty) {
        renderPassNode->renderTargetFlags = QRhiTextureRenderTarget::Flags(m_renderTargetFlags.toInt());
        renderPassNode->clearColor = m_clearColor;
        renderPassNode->depthStencilClearValue = { m_depthClearValue, m_stencilClearValue };

        clearDirty(Dirty::ClearDirty);
    }

    if (m_dirtyAttributes & Dirty::PassTypeDirty) {
        switch (m_passMode) {
        case UserPass:
            renderPassNode->passMode = QSSGRenderUserPass::UserPass;
            break;
        case SkyboxPass:
            renderPassNode->passMode = QSSGRenderUserPass::SkyboxPass;
            break;
        case Item2DPass:
            renderPassNode->passMode = QSSGRenderUserPass::Item2DPass;
            break;
        }

        clearDirty(Dirty::PassTypeDirty);
    }

    // If not a user pass, we're done
    if (m_passMode != UserPass)
        return renderPassNode;

    renderPassNode->materialMode = QSSGRenderUserPass::MaterialModes(m_materialMode);
    clearDirty(Dirty::MaterialModeDirty);

    if (renderPassNode->materialMode == QSSGRenderUserPass::OverrideMaterial) {
        clearDirty(Dirty::OverrideMaterialDirty);
        if (m_overrideMaterial) {
            // Set the backend material
            QSSGRenderGraphObject *graphObject = QQuick3DObjectPrivate::get(m_overrideMaterial)->spatialNode;
            if (graphObject)
                renderPassNode->overrideMaterial = graphObject;
            else
                markDirty(OverrideMaterialDirty, true); // Try again next time
        } else {
            // Set nullptr
            renderPassNode->overrideMaterial = nullptr;
        }
    } else if (renderPassNode->materialMode == QSSGRenderUserPass::OriginalMaterial) {
        // Nothing to do
    } else if (renderPassNode->materialMode == QSSGRenderUserPass::AugmentMaterial) {
        // Augment Shaders
        if (!m_augmentShader.isEmpty()) {
            const QQmlContext *context = qmlContext(this);
            QByteArray shaderPathKey("augment material --");
            QByteArray augment = QSSGShaderUtils::resolveShader(m_augmentShader, context, shaderPathKey);
            QByteArray augmentSnippet;
            QByteArray augmentPreamble;

            // We have to pick apart the shader string such that the contents of the:
            // void MAIN_FRAGMENT_AUGMENT() { }
            // function are taken out, and will get added to the end of the shader generation
            // and the goal is to overwrite the "output" of the shader

            // We also need to scan the who shader code for certain "keywords" so that we know
            // what features to enable in the original material.

            // Everything else outsode of MAIN_FRAGMENT_AUGMENT function ends up being preamble code
            // that will get pasted in before the real main().  So that will include helper functions and
            // resolvable #includes etc.

            static const char *mainFuncStart = "void MAIN_FRAGMENT_AUGMENT()";
            qsizetype mainFuncIdx = augment.indexOf(mainFuncStart);
            if (mainFuncIdx != -1) {
                qsizetype braceOpenIdx = augment.indexOf('{', mainFuncIdx + int(strlen(mainFuncStart)));
                if (braceOpenIdx != -1) {
                    qsizetype braceCloseIdx = braceOpenIdx;
                    qsizetype openBraces = 1;
                    while (openBraces > 0 && braceCloseIdx + 1 < augment.size()) {
                        braceCloseIdx++;
                        if (augment[braceCloseIdx] == '{')
                            openBraces++;
                        else if (augment[braceCloseIdx] == '}')
                            openBraces--;
                    }
                    if (openBraces == 0) {
                        // We found the closing brace
                        augmentSnippet = augment.mid(braceOpenIdx + 1, braceCloseIdx - braceOpenIdx - 1);
                        augmentPreamble = augment.left(mainFuncIdx);
                        augmentPreamble += augment.mid(braceCloseIdx + 1);
                    } else {
                        qWarning("QQuick3DRenderPass: Could not find the closing brace of MAIN_FRAGMENT_AUGMENT() in shader %s", qPrintable(m_augmentShader.toString()));
                    }
                } else {
                    qWarning("QQuick3DRenderPass: Could not find the opening brace of MAIN_FRAGMENT_AUGMENT() in shader %s", qPrintable(m_augmentShader.toString()));
                }
            } else {
                qWarning("QQuick3DRenderPass: Could not find MAIN_FRAGMENT_AUGMENT() function in shader %s", qPrintable(m_augmentShader.toString()));
            }

            renderPassNode->shaderAugmentation.body = augmentSnippet;
            renderPassNode->shaderAugmentation.preamble = augmentPreamble;
            renderPassNode->markDirty(QSSGRenderUserPass::DirtyFlag::ShaderDirty);
        }
    }

    return renderPassNode;
}

void QQuick3DRenderPass::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DRenderPass::markTrackedPropertyDirty(QMetaProperty property, DirtyPropertyHint hint)
{
    Q_UNUSED(property);

    // FIXME: As with the property tracking for Effects and Custom materials we
    // should really track which property changed and only update that one.
    if (hint == DirtyPropertyHint::Reference) {
        // FIXME: We should verify that the property is actually a texture property.
        markDirty(Dirty::TextureDirty);
    } else {
        markDirty(Dirty::PropertyDirty);
    }
}

void QQuick3DRenderPass::onMaterialDestroyed(QObject *object)
{
    if (m_overrideMaterial == object) {
        m_overrideMaterial = nullptr;
        emit overrideMaterialChanged();
        markDirty(OverrideMaterialDirty);
    }
}

void QQuick3DRenderPass::qmlAppendCommand(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, QQuick3DShaderUtilsRenderCommand *command)
{
    if (!command)
        return;

    QQuick3DRenderPass *that = qobject_cast<QQuick3DRenderPass *>(list->object);

    if (!command->parentItem())
        command->setParentItem(that);

    that->m_commands.push_back(command);
    that->markDirty(CommandsDirty);
}

QQuick3DShaderUtilsRenderCommand *QQuick3DRenderPass::qmlCommandAt(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, qsizetype index)
{
    QQuick3DRenderPass *that = qobject_cast<QQuick3DRenderPass *>(list->object);
    return that->m_commands.at(index);
}

qsizetype QQuick3DRenderPass::qmlCommandCount(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list)
{
    QQuick3DRenderPass *that = qobject_cast<QQuick3DRenderPass *>(list->object);
    return that->m_commands.size();
}

void QQuick3DRenderPass::qmlCommandClear(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list)
{
    QQuick3DRenderPass *that = qobject_cast<QQuick3DRenderPass *>(list->object);
    that->m_commands.clear();
    that->markDirty(CommandsDirty);
}

void QQuick3DRenderPass::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager) {
        // Handle inline override material that may not have had a scene manager when it was set
        if (m_overrideMaterial && !m_overrideMaterial->parentItem() && !QQuick3DObjectPrivate::get(m_overrideMaterial)->sceneManager) {
            if (!m_overrideMaterialRefed) {
                QQuick3DObjectPrivate::refSceneManager(m_overrideMaterial, *sceneManager);
                m_overrideMaterialRefed = true;
            }
        }
    } else {
        // Deref the material when scene manager is removed
        if (m_overrideMaterial && m_overrideMaterialRefed) {
            QQuick3DObjectPrivate::derefSceneManager(m_overrideMaterial);
            m_overrideMaterialRefed = false;
        }
    }
}

void QQuick3DRenderPass::markDirty(Dirty type,  bool requestSecondaryUpdate)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }

    if (requestSecondaryUpdate)
        QQuick3DObjectPrivate::get(this)->requestSecondaryUpdate();
}

void QQuick3DRenderPass::clearDirty(Dirty type)
{
    m_dirtyAttributes &= ~quint32(type);
}

/*!
    \qmlproperty list<RenderCommand> RenderPass::commands
    This property holds the list of render commands for the render pass.

    The commands in the list are executed in the order they appear in the list.

    \note The commands for RenderPass and Effects are similar but not the same, only
    those marked as compatible can be used with this RenderPass.

    \sa SubRenderPass,
        PipelineStateOverride,
        RenderablesFilter,
        RenderPassTexture,
        ColorAttachment,
        DepthTextureAttachment,
        DepthStencilAttachment,
        AddDefine,
        renderTargetBlend
*/

QQmlListProperty<QQuick3DShaderUtilsRenderCommand> QQuick3DRenderPass::commands()
{
    return QQmlListProperty<QQuick3DShaderUtilsRenderCommand>(this,
                                                              nullptr,
                                                              QQuick3DRenderPass::qmlAppendCommand,
                                                              QQuick3DRenderPass::qmlCommandCount,
                                                              QQuick3DRenderPass::qmlCommandAt,
                                                              QQuick3DRenderPass::qmlCommandClear);
}

/*!
    \qmlproperty color RenderPass::clearColor
    This property holds the clear color for the render pass.

    \default Qt.black
*/
QColor QQuick3DRenderPass::clearColor() const
{
    return m_clearColor;
}

void QQuick3DRenderPass::setClearColor(const QColor &newClearColor)
{
    if (m_clearColor == newClearColor)
        return;
    m_clearColor = newClearColor;
    emit clearColorChanged();
    markDirty(ClearDirty);
}

/*!
    \qmlproperty RenderPass::MaterialModes RenderPass::materialMode
    Controls how object materials are handled when rendering into this pass.

    \value RenderPass.OriginalMaterial
        Objects are rendered using their own assigned materials, with full
        lighting, textures, and material properties applied normally. This is
        the standard mode for rendering a faithful copy of the scene into a
        custom render target — for example, a secondary viewpoint for a
        reflection probe, a rear-view camera, or a picture-in-picture effect.
        The \c overrideMaterial, \c augmentShader, and \c shaders properties
        are not used in this mode.

    \value RenderPass.AugmentMaterial
        Each object is rendered with its own material, but the contents of
        the \c {MAIN_FRAGMENT_AUGMENT()} function defined in \l augmentShader
        are injected after the original material's output definition. This
        allows the augment code to read the material's computed color and write
        to additional color outputs defined by \l ColorAttachment commands in
        the pass. This is useful for multi-render-target (MRT) passes that need
        per-material shading, such as writing the lit color to one attachment
        and a world-space normal to another in a single draw call.

    \value RenderPass.OverrideMaterial
        All objects rendered by this pass use the single \l overrideMaterial
        instead of their own. This is useful for depth-only passes, shadow maps,
        silhouette or outline effects, and any other case where you want all
        geometry to be shaded identically regardless of what material is
        assigned to it. The \c augmentShader property is not used in this mode.

    \default RenderPass.OriginalMaterial
*/
QQuick3DRenderPass::MaterialModes QQuick3DRenderPass::materialMode() const
{
    return m_materialMode;
}

void QQuick3DRenderPass::setMaterialMode(MaterialModes newMaterialMode)
{
    if (m_materialMode == newMaterialMode)
        return;
    m_materialMode = newMaterialMode;
    emit materialModeChanged();
    markDirty(MaterialModeDirty);
}

/*!
    \qmlproperty Material RenderPass::overrideMaterial
    This property holds the override material for the render pass when
    \l{RenderPass::materialMode}{materialMode} is set to \c OverrideMaterial.
*/
QQuick3DMaterial *QQuick3DRenderPass::overrideMaterial() const
{
    return m_overrideMaterial;
}

void QQuick3DRenderPass::setOverrideMaterial(QQuick3DMaterial *newOverrideMaterial)
{
    if (m_overrideMaterial == newOverrideMaterial)
        return;

    // Deref the old material if we had ref'd it
    if (m_overrideMaterial && m_overrideMaterialRefed) {
        QQuick3DObjectPrivate::derefSceneManager(m_overrideMaterial);
        m_overrideMaterialRefed = false;
    }

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DRenderPass::setOverrideMaterial, newOverrideMaterial, m_overrideMaterial);

    m_overrideMaterial = newOverrideMaterial;

    // Handle inline material declarations by ensuring they get registered with the scene manager
    if (m_overrideMaterial && m_overrideMaterial->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a QQuick3DObject
        // and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(m_overrideMaterial->parent());
        if (parentItem) {
            m_overrideMaterial->setParentItem(parentItem);
        } else {
            // If no valid parent was found, make sure the material refs our scene manager
            const auto &sceneManager = QQuick3DObjectPrivate::get(this)->sceneManager;
            if (sceneManager) {
                QQuick3DObjectPrivate::refSceneManager(m_overrideMaterial, *sceneManager);
                m_overrideMaterialRefed = true;
            }
            // else: If there's no scene manager, defer until one is set, see itemChange()
        }
    }

    emit overrideMaterialChanged();
    markDirty(OverrideMaterialDirty);
}

/*!
    \qmlproperty url RenderPass::augmentShader
    This property holds the augment shader URL for the render pass when
    \l{RenderPass::materialMode}{materialMode} is set to \c AugmentMaterial.

    The shader file should contain a function with the following signature:
    \badcode
    void MAIN_FRAGMENT_AUGMENT() {
        // Custom shader code here
    }
    \endcode

    This function will be combined with the existing fragment shader of the material
    being used by the object being rendered in this render pass. Allowing users to
    augment the existing material shader with custom code.
*/
QUrl QQuick3DRenderPass::augmentShader() const
{
    return m_augmentShader;
}

void QQuick3DRenderPass::setAugmentShader(const QUrl &newAugmentShader)
{
    if (m_augmentShader == newAugmentShader)
        return;
    m_augmentShader = newAugmentShader;
    emit augmentShaderChanged();
    markDirty(AugmentShaderDirty);
}

/*!
    \qmlproperty RenderPass::PassMode RenderPass::passMode
    This property holds the pass mode for the render pass.

    In addition to standard user render passes, Qt Quick 3D supports
    users to manually triggering internal render passes for rendering
    the skybox and 2D items.

    \value RenderPass.UserPass A user specified render pass.
    \value RenderPass.SkyboxPass Qt Quick 3D's built-in skybox render pass.
    \value RenderPass.Item2DPass Qt Quick 3D's built-in 2D item render pass.
    \default RenderPass.UserPass
*/

QQuick3DRenderPass::PassMode QQuick3DRenderPass::passMode() const
{
    return m_passMode;
}

void QQuick3DRenderPass::setPassMode(PassMode newPassMode)
{
    if (m_passMode == newPassMode)
        return;
    m_passMode = newPassMode;
    emit passModeChanged();
    markDirty(PassTypeDirty);
}

/*!
    \qmlproperty real RenderPass::depthClearValue
    This property holds the depth clear value for the render pass.

    \default 1.0
*/
float QQuick3DRenderPass::depthClearValue() const
{
    return m_depthClearValue;
}

void QQuick3DRenderPass::setDepthClearValue(float newDepthClearValue)
{
    if (qFuzzyCompare(m_depthClearValue, newDepthClearValue))
        return;
    m_depthClearValue = newDepthClearValue;
    emit depthClearValueChanged();
    markDirty(ClearDirty);
}

/*!
    \qmlproperty int RenderPass::stencilClearValue
    This property holds the stencil clear value for the render pass.

    \default 0
*/
quint32 QQuick3DRenderPass::stencilClearValue() const
{
    return m_stencilClearValue;
}

void QQuick3DRenderPass::setStencilClearValue(quint32 newStencilClearValue)
{
    if (m_stencilClearValue == newStencilClearValue)
        return;
    m_stencilClearValue = newStencilClearValue;
    emit stencilClearValueChanged();
    markDirty(ClearDirty);
}

/*!
    \qmlproperty RenderPass::RenderTargetFlags RenderPass::renderTargetFlags
    This property holds the render target flags for the render pass. These flags affect how
    the render target contents are handled at the beginning and end of each frame.

    \value RenderPass.None No special behavior. Color and depth/stencil contents are cleared at the start of each frame.
    \value RenderPass.PreserveColorContents Preserve the color contents of the render target between frames, so the previous frame's output remains until explicitly overwritten.
    \value RenderPass.PreserveDepthStencilContents Preserve the depth and stencil contents of the render target between frames.
    \value RenderPass.DoNotStoreDepthStencilContents Do not store the depth and stencil contents of the render target after rendering (may improve performance on tiled GPUs).

    \default RenderPass.None

    \sa QRhiTextureRenderTarget::Flags
*/

QQuick3DRenderPass::RenderTargetFlags QQuick3DRenderPass::renderTargetFlags() const
{
    return m_renderTargetFlags;
}

void QQuick3DRenderPass::setRenderTargetFlags(RenderTargetFlags newRenderTargetFlags)
{
    if (m_renderTargetFlags == newRenderTargetFlags)
        return;
    m_renderTargetFlags = newRenderTargetFlags;
    emit renderTargetFlagsChanged();
    markDirty(ClearDirty);
}

QT_END_NAMESPACE
