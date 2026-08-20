// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Q3DSCATTERRENDERER_P_H
#define Q3DSCATTERRENDERER_P_H

#include "datavisualizationglobal_p.h"
#include "scatter3dcontroller_p.h"
#include "abstract3drenderer_p.h"
#include "scatterrenderitem_p.h"

QT_FORWARD_DECLARE_CLASS(QSizeF)

QT_BEGIN_NAMESPACE

class ShaderHelper;
class Q3DScene;
class ScatterSeriesRenderCache;
class QScatterDataItem;

class Q_DATAVISUALIZATION_EXPORT Scatter3DRenderer : public Abstract3DRenderer
{
    Q_OBJECT

private:
    // Internal state
    ScatterRenderItem *m_selectedItem; // points to renderitem array
    bool m_updateLabels;
    ShaderHelper *m_dotShader;
    ShaderHelper *m_dotGradientShader;
    ShaderHelper *m_staticSelectedItemGradientShader;
    ShaderHelper *m_staticSelectedItemShader;
    ShaderHelper *m_pointShader;
    ShaderHelper *m_depthShader;
    ShaderHelper *m_selectionShader;
    ShaderHelper *m_backgroundShader;
    ShaderHelper *m_staticGradientPointShader;
    GLuint m_bgrTexture;
    GLuint m_selectionTexture;
    GLuint m_depthFrameBuffer;
    GLuint m_selectionFrameBuffer;
    GLuint m_selectionDepthBuffer;
    GLfloat m_shadowQualityToShader;
    GLint m_shadowQualityMultiplier;
    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;
    int m_selectedItemIndex;
    ScatterSeriesRenderCache *m_selectedSeriesCache;
    ScatterSeriesRenderCache *m_oldSelectedSeriesCache;
    GLfloat m_dotSizeScale;
    ScatterRenderItem m_dummyRenderItem;
    GLfloat m_maxItemSize;
    int m_clickedIndex;
    bool m_havePointSeries;
    bool m_haveMeshSeries;
    bool m_haveUniformColorMeshSeries;
    bool m_haveGradientMeshSeries;

public:
    explicit Scatter3DRenderer(Scatter3DController *controller);
    ~Scatter3DRenderer();

    void updateData() override;
    void updateSeries(const QList<QAbstract3DSeries *> &seriesList) override;
    SeriesRenderCache *createNewCache(QAbstract3DSeries *series) override;
    void updateItems(const QList<Scatter3DController::ChangeItem> &items);
    void updateScene(Q3DScene *scene) override;
    void updateAxisLabels(QAbstract3DAxis::AxisOrientation orientation,
                          const QStringList &labels) override;
    void updateAxisTitleVisibility(QAbstract3DAxis::AxisOrientation orientation,
                                   bool visible) override;
    void updateOptimizationHint(QAbstract3DGraph::OptimizationHints hint) override;
    void updateMargin(float margin) override;

    QVector3D convertPositionToTranslation(const QVector3D &position, bool isAbsolute) override;

    inline int clickedIndex() const { return m_clickedIndex; }
    void resetClickedStatus();

    void render(GLuint defaultFboHandle) override;

public Q_SLOTS:
    void updateSelectedItem(int index, QScatter3DSeries *series);

protected:
    void contextCleanup() override;
    void initializeOpenGL() override;
    void fixCameraTarget(QVector3D &target) override;
    void getVisibleItemBounds(QVector3D &minBounds, QVector3D &maxBounds) override;

private:
    void initShaders(const QString &vertexShader, const QString &fragmentShader) override;
    void initGradientShaders(const QString &vertexShader, const QString &fragmentShader) override;
    void initStaticSelectedItemShaders(const QString &vertexShader,
                                       const QString &fragmentShader,
                                       const QString &gradientVertexShader,
                                       const QString &gradientFragmentShader) override;
    void updateShadowQuality(QAbstract3DGraph::ShadowQuality quality) override;
    void updateTextures() override;
    void fixMeshFileName(QString &fileName, QAbstract3DSeries::Mesh mesh) override;

    void drawScene(GLuint defaultFboHandle);
    void drawLabels(bool drawSelection, const Q3DCamera *activeCamera,
                    const QMatrix4x4 &viewMatrix, const QMatrix4x4 &projectionMatrix);

    void loadBackgroundMesh();
    void initSelectionShader();
    void initBackgroundShaders(const QString &vertexShader, const QString &fragmentShader) override;
    void initStaticPointShaders(const QString &vertexShader, const QString &fragmentShader);
    void initSelectionBuffer() override;
    void initDepthShader();
    void updateDepthBuffer() override;
    void initPointShader();
    void calculateTranslation(ScatterRenderItem &item);
    void calculateSceneScalingFactors();

    void selectionColorToSeriesAndIndex(const QVector4D &color, int &index,
                                        QAbstract3DSeries *&series);
    inline void updateRenderItem(const QScatterDataItem &dataItem, ScatterRenderItem &renderItem);

    Q_DISABLE_COPY(Scatter3DRenderer)
};

QT_END_NAMESPACE

#endif
