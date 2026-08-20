// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHADERPOKER_H
#define SHADERPOKER_H

#include <QObject>
#include <QQmlEngine>

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>

#include "defaultmaterialshaderproperties.h"

class ShaderPoker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int shaderIndex READ shaderIndex WRITE setShaderIndex NOTIFY shaderIndexChanged FINAL)
    Q_PROPERTY(QString vertexShader READ vertexShader NOTIFY vertexShaderChanged)
    Q_PROPERTY(QString fragmentShader READ fragmentShader NOTIFY fragmentShaderChanged)
    Q_PROPERTY(QString shaderKey READ shaderKey NOTIFY shaderKeyChanged)
    Q_PROPERTY(QStringList shaderKeysList READ shaderKeysList NOTIFY shaderKeysListChanged)
    Q_PROPERTY(MaterialType materialType READ materialType WRITE setMaterialType NOTIFY materialTypeChanged)
    Q_PROPERTY(quint32 featureBitfield READ featureBitfield WRITE setFeatureBitfield NOTIFY featureBitfieldChanged)
    Q_PROPERTY(QStringList availableShadersList READ availableShadersList NOTIFY availableShadersListChanged FINAL)
    Q_PROPERTY(DefaultMaterialShaderProperties* shaderProperties READ shaderProperties WRITE setShaderProperties NOTIFY shaderPropertiesChanged FINAL)
    QML_ELEMENT
public:

    enum class MaterialType {
        Default,
        Principled,
        SpecularGlossy,
    };
    Q_ENUM(MaterialType);

    explicit ShaderPoker(QObject *parent = nullptr);
    ~ShaderPoker() override;

    Q_INVOKABLE void generateShader();

    QString vertexShader() const;
    QString fragmentShader() const;
    QString shaderKey() const;

    QStringList shaderKeysList() const;

    MaterialType materialType() const;
    void setMaterialType(const MaterialType &newMaterialType);

    quint32 featureBitfield() const;
    void setFeatureBitfield(quint32 newFeatureBitfield);

    QStringList availableShadersList() const;

    int shaderIndex() const;
    void setShaderIndex(int newShaderIndex);

    DefaultMaterialShaderProperties *shaderProperties() const;
    void setShaderProperties(DefaultMaterialShaderProperties *newShaderProperties);

signals:
    void vertexShaderChanged();
    void fragmentShaderChanged();
    void shaderKeyChanged();
    void shaderKeysListChanged();
    void materialTypeChanged();
    void featureBitfieldChanged();
    void availableShadersListChanged();
    void shaderIndexChanged();
    void shaderPropertiesChanged();

private slots:
    void shaderPropertiesDirty();

private:
    void updateAvailableShadersList();

    QRhi *m_rhi = nullptr;
    std::shared_ptr<QSSGRenderContextInterface> m_renderContext;

    QSSGShaderFeatures m_featureSet;
    DefaultMaterialShaderProperties m_builtinProperties;
    DefaultMaterialShaderProperties *m_shaderProperties;
    QMetaObject::Connection m_shaderPropertiesDirtyConnection;
    QSSGRenderDefaultMaterial *m_material = nullptr;
    QString m_vertexShader;
    QString m_fragmentShader;
    QString m_shaderKey;
    QStringList m_shaderKeysList;
    MaterialType m_materialType = MaterialType::Default;
    quint32 m_featureBitfield = 0;

    QHash<QSSGShaderCacheKey, std::pair<QString, QString>> m_generatedShaderCodeCache;
    std::shared_ptr<QSSGRhiShaderPipeline> m_currentShaderPipeline;
    QStringList m_availableShadersList;
    int m_shaderIndex = 0;
};

#endif // SHADERPOKER_H
