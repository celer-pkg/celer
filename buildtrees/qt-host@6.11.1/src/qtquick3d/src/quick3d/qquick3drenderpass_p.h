// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDERPASS_P_H
#define QQUICK3DRENDERPASS_P_H

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

#include <QtQuick3D/qtquick3dglobal.h>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dshaderutils_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DRenderPass : public QQuick3DObject, public QQuick3DPropertyChangedTracker
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> commands READ commands)
    Q_PROPERTY(MaterialModes materialMode READ materialMode WRITE setMaterialMode NOTIFY materialModeChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
    Q_PROPERTY(float depthClearValue READ depthClearValue WRITE setDepthClearValue NOTIFY depthClearValueChanged)
    Q_PROPERTY(quint32 stencilClearValue READ stencilClearValue WRITE setStencilClearValue NOTIFY stencilClearValueChanged)
    Q_PROPERTY(QQuick3DMaterial* overrideMaterial READ overrideMaterial WRITE setOverrideMaterial NOTIFY overrideMaterialChanged)
    Q_PROPERTY(QUrl augmentShader READ augmentShader WRITE setAugmentShader NOTIFY augmentShaderChanged)
    Q_PROPERTY(PassMode passMode READ passMode WRITE setPassMode NOTIFY passModeChanged)
    Q_PROPERTY(RenderTargetFlags renderTargetFlags READ renderTargetFlags WRITE setRenderTargetFlags NOTIFY renderTargetFlagsChanged)
    QML_NAMED_ELEMENT(RenderPass)
    QML_ADDED_IN_VERSION(6, 11)
public:
    enum MaterialModes {
        OriginalMaterial,
        AugmentMaterial,
        OverrideMaterial
    };
    Q_ENUM(MaterialModes)

    enum PassMode {
        UserPass,
        SkyboxPass,
        Item2DPass,
    };
    Q_ENUM(PassMode)

    enum class RenderTargetFlag {
        None = 0,
        PreserveColorContents = 1 << 0,
        PreserveDepthStencilContents = 1 << 1,
        DoNotStoreDepthStencilContents = 1 << 2
    };
    Q_DECLARE_FLAGS(RenderTargetFlags, RenderTargetFlag)
    Q_FLAGS(RenderTargetFlags)

    explicit QQuick3DRenderPass(QQuick3DObject *parent = nullptr);

    QQmlListProperty<QQuick3DShaderUtilsRenderCommand> commands();

    QColor clearColor() const;
    void setClearColor(const QColor &newClearColor);

    MaterialModes materialMode() const;
    void setMaterialMode(MaterialModes newMaterialMode);

    QQuick3DMaterial *overrideMaterial() const;
    void setOverrideMaterial(QQuick3DMaterial *newOverrideMaterial);

    QUrl augmentShader() const;
    void setAugmentShader(const QUrl &newAugmentShader);

    PassMode passMode() const;
    void setPassMode(PassMode newPassMode);

    float depthClearValue() const;
    void setDepthClearValue(float newDepthClearValue);

    quint32 stencilClearValue() const;
    void setStencilClearValue(quint32 newStencilClearValue);

    void markAllDirty() override { m_dirtyAttributes = AllDirty; }

    RenderTargetFlags renderTargetFlags() const;
    void setRenderTargetFlags(RenderTargetFlags newRenderTargetFlags);

Q_SIGNALS:
    void outputChanged();
    void materialModeChanged();
    void clearColorChanged();
    void overrideMaterialChanged();
    void augmentShaderChanged();
    void passModeChanged();
    void depthClearValueChanged();
    void stencilClearValueChanged();
    void renderTargetFlagsChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void itemChange(QQuick3DObject::ItemChange , const QQuick3DObject::ItemChangeData &) override;
    void markTrackedPropertyDirty(QMetaProperty property, QQuick3DPropertyChangedTracker::DirtyPropertyHint hint) override;

private Q_SLOTS:
    void onMaterialDestroyed(QObject *object);

private:
    static void qmlAppendCommand(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, QQuick3DShaderUtilsRenderCommand *command);
    static QQuick3DShaderUtilsRenderCommand *qmlCommandAt(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, qsizetype index);
    static qsizetype qmlCommandCount(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list);
    static void qmlCommandClear(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list);

    enum Dirty {
        TextureDirty = 0x1,
        PropertyDirty = 0x2,
        CommandsDirty = 0x8,
        ClearDirty = 0x10,
        MaterialModeDirty = 0x20,
        OverrideMaterialDirty = 0x40,
        AugmentShaderDirty = 0x80,
        PassTypeDirty = 0x100
    };

    using DirtyT = std::underlying_type_t<Dirty>;
    static constexpr DirtyT AllDirty { std::numeric_limits<DirtyT>::max() };

    void updateSceneManager(QQuick3DSceneManager *sceneManager);
    void markDirty(QQuick3DRenderPass::Dirty type, bool requestSecondaryUpdate = false);
    void clearDirty(QQuick3DRenderPass::Dirty type);

    quint32 m_dirtyAttributes = AllDirty;
    QVector<QQuick3DShaderUtilsRenderCommand *> m_commands;
    QColor m_clearColor = Qt::black;
    MaterialModes m_materialMode = OriginalMaterial;
    QQuick3DMaterial *m_overrideMaterial = nullptr;
    bool m_overrideMaterialRefed = false;
    QUrl m_augmentShader;
    PassMode m_passMode = UserPass;
    float m_depthClearValue = 1.0f;
    quint32 m_stencilClearValue = 0;
    RenderTargetFlags m_renderTargetFlags = RenderTargetFlag::None;
};

QT_END_NAMESPACE

#endif // QQUICK3DRENDERPASS_P_H
