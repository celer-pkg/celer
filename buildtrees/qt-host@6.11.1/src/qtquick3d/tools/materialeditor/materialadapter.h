// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MATERIALADAPTER_H
#define MATERIALADAPTER_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

#include <QtQml/qqmlregistration.h>

#include <ssg/qssgrenderbasetypes.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>

#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlistmodel.h>

#include <QtQuick3DAssetUtils/private/qssgscenedesc_p.h>

#include "custommaterial.h"
#include "buildmessage.h"
#include "uniformmodel.h"

QT_BEGIN_NAMESPACE

class MaterialAdapter : public QObject
{
    Q_OBJECT

    using CullMode = QQuick3DCustomMaterial::CullMode;
    using DepthDrawMode = QQuick3DCustomMaterial::DepthDrawMode;
    using ShadingMode = QQuick3DCustomMaterial::ShadingMode;
    using BlendMode = QQuick3DCustomMaterial::BlendMode;

    Q_PROPERTY(QQuick3DCustomMaterial * material READ material NOTIFY materialChanged)
    Q_PROPERTY(QQuick3DNode * rootNode READ rootNode WRITE setRootNode NOTIFY rootNodeChanged)
    Q_PROPERTY(QString fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
    Q_PROPERTY(QString vertexShader READ vertexShader WRITE setVertexShader NOTIFY vertexShaderChanged)
    Q_PROPERTY(ShaderBuildMessage vertexStatus READ vertexStatus NOTIFY vertexStatusChanged)
    Q_PROPERTY(ShaderBuildMessage fragmentStatus READ fragmentStatus NOTIFY fragmentStatusChanged)
    Q_PROPERTY(UniformModel * uniformModel READ uniformModel WRITE setUniformModel NOTIFY uniformModelChanged)
    Q_PROPERTY(bool unsavedChanges READ unsavedChanges WRITE setUnsavedChanges NOTIFY unsavedChangesChanged)
    Q_PROPERTY(QUrl materialSaveFile READ materialSaveFile WRITE setMaterialSaveFile NOTIFY materialSaveFileChanged)

    Q_PROPERTY(QQuick3DMaterial::CullMode cullMode READ cullMode WRITE setCullMode NOTIFY cullModeChanged)
    Q_PROPERTY(QQuick3DMaterial::DepthDrawMode depthDrawMode READ depthDrawMode WRITE setDepthDrawMode NOTIFY depthDrawModeChanged)
    Q_PROPERTY(QQuick3DCustomMaterial::ShadingMode shadingMode READ shadingMode WRITE setShadingMode NOTIFY shadingModeChanged)
    Q_PROPERTY(QQuick3DCustomMaterial::BlendMode sourceBlend READ srcBlend WRITE setSrcBlend NOTIFY srcBlendChanged)
    Q_PROPERTY(QQuick3DCustomMaterial::BlendMode destinationBlend READ dstBlend WRITE setDstBlend NOTIFY dstBlendChanged)

    QML_ELEMENT
public:
    explicit MaterialAdapter(QObject *parent = nullptr);

    QQuick3DCustomMaterial *material() const;
    QString fragmentShader() const;
    void setFragmentShader(const QString &newFragmentShader);

    QString vertexShader() const;
    void setVertexShader(const QString &newVertexShader);

    UniformModel *uniformModel() const;

    ShaderBuildMessage vertexStatus() const;
    ShaderBuildMessage fragmentStatus() const;

    bool unsavedChanges() const;
    void setUnsavedChanges(bool newUnsavedChanges);

    const QUrl &materialSaveFile() const;
    void setMaterialSaveFile(const QUrl &newMaterialSaveFile);

    QQuick3DNode *rootNode() const;
    void setRootNode(QQuick3DNode *newResourceNode);

    CullMode cullMode() const;
    void setCullMode(CullMode newCullMode);

    DepthDrawMode depthDrawMode() const;
    void setDepthDrawMode(DepthDrawMode newDepthDrawMode);

    ShadingMode shadingMode() const;
    void setShadingMode(ShadingMode newShadingMode);

    BlendMode srcBlend() const;
    void setSrcBlend(BlendMode newSourceBlend);

    BlendMode dstBlend() const;
    void setDstBlend(BlendMode newDestinationBlend);

    void setUniformModel(UniformModel *newUniformModel);

    Q_INVOKABLE QString getSupportedImageFormatsFilter() const;

public Q_SLOTS:
    void importFragmentShader(const QUrl &shaderFile);
    void importVertexShader(const QUrl &shaderFile);
    bool save();
    bool saveMaterial(const QUrl &materialFile);
    bool loadMaterial(const QUrl &materialFile);
    bool exportQmlComponent(const QUrl &componentFile, const QString &vertName, const QString &fragName);
    void reset();

Q_SIGNALS:
    void materialChanged();
    void fragmentShaderChanged();
    void vertexShaderChanged();
    void vertexStatusChanged();
    void uniformModelChanged();
    void fragmentStatusChanged();
    void unsavedChangesChanged();
    void materialSaveFileChanged();

    void errorOccurred();
    void postMaterialSaved();
    void rootNodeChanged();
    void cullModeChanged();
    void depthDrawModeChanged();
    void shadingModeChanged();
    void srcBlendChanged();
    void dstBlendChanged();

private:
    static void updateShader(QQuick3DMaterial &target);
    static void bakerStatusCallback(const QByteArray &descKey, QtQuick3DEditorHelpers::ShaderBaker::Status status, const QString &err, QShader::Stage stage);
    void updateMaterialDescription(CustomMaterial::Shaders shaders);
    void updateMaterialDescription();

    QString importShader(const QUrl &shaderFile);
    QFile resolveFileFromUrl(const QUrl &fileUrl);
    QPointer<QQuick3DCustomMaterial> m_material;
    UniformModel *m_uniformModel = nullptr;
    QUrl m_vertexUrl;
    QUrl m_fragUrl;
    QString m_fragmentShader;
    QString m_vertexShader;
    ShaderBuildMessage m_vertexMsg;
    ShaderBuildMessage m_fragmentMsg;
    bool m_unsavedChanges = true;
    QUrl m_materialSaveFile;
    QPointer<QQuick3DNode> m_rootNode;
    CustomMaterial m_materialDescr;
    CustomMaterial::UniformTable uniformTable;
    CustomMaterial::Properties m_properties;
};

QT_END_NAMESPACE

#endif // MATERIALADAPTER_H
