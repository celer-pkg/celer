// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterobjectbufferhelper_p.h"
#include "objecthelper_p.h"

#include <QtGui/QVector2D>
#include <QtGui/QMatrix4x4>
#include <QtGui/qquaternion.h>

#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

const GLfloat ScatterObjectBufferHelper::itemScaler = 3.0f;

ScatterObjectBufferHelper::ScatterObjectBufferHelper()
    : m_scaleY(0.0f)
{
}

ScatterObjectBufferHelper::~ScatterObjectBufferHelper()
{
}

void ScatterObjectBufferHelper::fullLoad(ScatterSeriesRenderCache *cache, qreal dotScale)
{
    m_indexCount = 0;

    ObjectHelper *dotObj = cache->object();
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const uint renderArraySize = renderArray.size();

    if (renderArraySize == 0)
        return;  // No use to go forward

    uint itemCount = 0;
    QQuaternion seriesRotation(cache->meshRotation());

    if (m_meshDataLoaded) {
        // Delete old data
        glDeleteBuffers(1, &m_vertexbuffer);
        glDeleteBuffers(1, &m_uvbuffer);
        glDeleteBuffers(1, &m_normalbuffer);
        glDeleteBuffers(1, &m_elementbuffer);
        m_vertexbuffer = 0;
        m_uvbuffer = 0;
        m_normalbuffer = 0;
        m_elementbuffer = 0;
        m_meshDataLoaded = false;
    }

    // Index vertices
    const QList<GLuint> indices = dotObj->indices();
    const QList<QVector3D> indexed_vertices = dotObj->indexedvertices();
    const QList<QVector2D> indexed_uvs = dotObj->indexedUVs();
    const QList<QVector3D> indexed_normals = dotObj->indexedNormals();
    const int indicesCount = indices.size();
    const int verticeCount = indexed_vertices.size();
    const int uvsCount = indexed_uvs.size();
    const int normalsCount = indexed_normals.size();

    float itemSize = cache->itemSize() / itemScaler;
    if (itemSize == 0.0f)
        itemSize = dotScale;
    QVector3D modelScaler(itemSize, itemSize, itemSize);
    QMatrix4x4 modelMatrix;
    if (!seriesRotation.isIdentity()) {
        QMatrix4x4 matrix;
        matrix.rotate(seriesRotation);
        modelMatrix = matrix.transposed();
    }
    modelMatrix.scale(modelScaler);

    QList<QVector3D> scaled_vertices;
    scaled_vertices.resize(verticeCount);
    for (int i = 0; i < verticeCount; i++)
        scaled_vertices[i] = (QVector4D(indexed_vertices[i]) * modelMatrix).toVector3D();

    QList<GLuint> buffered_indices;
    QList<QVector3D> buffered_vertices;
    QList<QVector2D> buffered_uvs;
    QList<QVector3D> buffered_normals;

    buffered_indices.resize(indicesCount * renderArraySize);
    buffered_vertices.resize(verticeCount * renderArraySize);
    buffered_normals.resize(normalsCount * renderArraySize);
    buffered_uvs.resize(uvsCount * renderArraySize);

    if (cache->colorStyle() == Q3DTheme::ColorStyleRangeGradient)
        createRangeGradientUVs(cache, buffered_uvs);
    else if (cache->colorStyle() == Q3DTheme::ColorStyleObjectGradient)
        createObjectGradientUVs(cache, buffered_uvs, indexed_vertices);

    QVector2D dummyUV(0.0f, 0.0f);

    cache->bufferIndices().resize(renderArraySize);

    for (uint i = 0; i < renderArraySize; i++) {
        const ScatterRenderItem &item = renderArray.at(i);
        if (!item.isVisible())
            continue;
        else
            cache->bufferIndices()[i] = itemCount;

        int offset = itemCount * verticeCount;
        if (item.rotation().isIdentity()) {
            for (int j = 0; j < verticeCount; j++) {
                buffered_vertices[j + offset] = scaled_vertices[j] + item.translation();
                buffered_normals[j + offset] = indexed_normals[j];
            }
        } else {
            QMatrix4x4 matrix;
            QQuaternion totalRotation = seriesRotation * item.rotation();
            matrix.rotate(totalRotation);
            matrix.scale(modelScaler);
            QMatrix4x4 itModelMatrix = matrix.inverted();
            modelMatrix = matrix.transposed(); // Because of row-column major difference

            for (int j = 0; j < verticeCount; j++) {
                buffered_vertices[j + offset]
                        = (QVector4D(indexed_vertices[j]) * modelMatrix).toVector3D()
                        + item.translation();
                buffered_normals[j + offset]
                        = (QVector4D(indexed_normals[j]) * itModelMatrix).toVector3D();
            }
        }

        if (cache->colorStyle() == Q3DTheme::ColorStyleUniform) {
            offset = itemCount * uvsCount;
            for (int j = 0; j < uvsCount; j++)
                buffered_uvs[j + offset] = dummyUV;
        }

        int offsetVertice = itemCount * verticeCount;
        offset = itemCount * indicesCount;
        for (int j = 0; j < indicesCount; j++)
            buffered_indices[j + offset] = GLuint(indices[j] + offsetVertice);

        itemCount++;
    }

    m_indexCount = indicesCount * itemCount;

    if (itemCount > 0) {
        glGenBuffers(1, &m_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, verticeCount * itemCount * sizeof(QVector3D),
                     &buffered_vertices.at(0),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &m_normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, normalsCount * itemCount * sizeof(QVector3D),
                     &buffered_normals.at(0),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &m_uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, uvsCount * itemCount * sizeof(QVector2D),
                     &buffered_uvs.at(0), GL_STATIC_DRAW);

        glGenBuffers(1, &m_elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * itemCount * sizeof(GLint),
                     &buffered_indices.at(0), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_meshDataLoaded = true;
    }
}

void ScatterObjectBufferHelper::updateUVs(ScatterSeriesRenderCache *cache)
{
    ObjectHelper *dotObj = cache->object();
    const int uvsCount = dotObj->indexedUVs().size();
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const bool updateAll = (cache->updateIndices().size() == 0);
    const int updateSize = updateAll ? renderArray.size() : cache->updateIndices().size();

    if (!updateSize)
        return;

    QList<QVector2D> buffered_uvs;
    buffered_uvs.resize(uvsCount * updateSize);

    uint itemCount = 0;
    if (cache->colorStyle() == Q3DTheme::ColorStyleRangeGradient) {
        itemCount = createRangeGradientUVs(cache, buffered_uvs);
    } else if (cache->colorStyle() == Q3DTheme::ColorStyleObjectGradient) {
        const QList<QVector3D> indexed_vertices = dotObj->indexedvertices();
        itemCount = createObjectGradientUVs(cache, buffered_uvs, indexed_vertices);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
    int itemSize = uvsCount * sizeof(QVector2D);
    if (cache->updateIndices().size()) {
        int pos = 0;
        for (int i = 0; i < updateSize; i++) {
            int index = cache->updateIndices().at(i);
            if (renderArray.at(index).isVisible()) {
                int dataPos = cache->bufferIndices().at(index);
                glBufferSubData(GL_ARRAY_BUFFER, itemSize * dataPos, itemSize,
                                &buffered_uvs.at(uvsCount * pos++));
            }
        }
    } else {
        glBufferData(GL_ARRAY_BUFFER, itemSize * itemCount, &buffered_uvs.at(0), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

uint ScatterObjectBufferHelper::createRangeGradientUVs(ScatterSeriesRenderCache *cache,
                                                       QList<QVector2D> &buffered_uvs)
{
    ObjectHelper *dotObj = cache->object();
    const int uvsCount = dotObj->indexedUVs().size();
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const bool updateAll = (cache->updateIndices().size() == 0);
    const int updateSize = updateAll ? renderArray.size() : cache->updateIndices().size();
    const float yAdjustment = 0.1f;
    const float flippedYAdjustment = 0.9f;

    QVector2D uv;
    uv.setX(0.0f);
    uint pos = 0;
    for (int i = 0; i < updateSize; i++) {
        int index = updateAll ? i : cache->updateIndices().at(i);
        const ScatterRenderItem &item = renderArray.at(index);
        if (!item.isVisible())
            continue;

        float y = ((item.translation().y() + m_scaleY) * 0.5f) / m_scaleY;

        // Avoid values near gradient texel boundary, as this causes artifacts
        // with some graphics cards.
        const float floorY = float(qFloor(y * gradientTextureHeight));
        const float diff = (y * gradientTextureHeight) - floorY;
        if (diff < yAdjustment)
            y += yAdjustment / gradientTextureHeight;
        else if (diff > flippedYAdjustment)
            y -= yAdjustment / gradientTextureHeight;
        uv.setY(y);

        int offset = pos * uvsCount;
        for (int j = 0; j < uvsCount; j++)
            buffered_uvs[j + offset] = uv;

        pos++;
    }

    return pos;
}

uint ScatterObjectBufferHelper::createObjectGradientUVs(ScatterSeriesRenderCache *cache,
                                                        QList<QVector2D> &buffered_uvs,
                                                        const QList<QVector3D> &indexed_vertices)
{
    ObjectHelper *dotObj = cache->object();
    const int uvsCount = dotObj->indexedUVs().size();
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const uint renderArraySize = renderArray.size();

    QVector2D uv;
    uv.setX(0.0f);
    uint pos = 0;
    for (uint i = 0; i < renderArraySize; i++) {
        const ScatterRenderItem &item = renderArray.at(i);
        if (!item.isVisible())
            continue;

        int offset = pos * uvsCount;
        for (int j = 0; j < uvsCount; j++) {
            uv.setY((indexed_vertices.at(j).y() + 1.0f) / 2.0f);
            buffered_uvs[j + offset] = uv;
        }

        pos++;
    }

    return pos;
}

void ScatterObjectBufferHelper::update(ScatterSeriesRenderCache *cache, qreal dotScale)
{
    ObjectHelper *dotObj = cache->object();
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const bool updateAll = (cache->updateIndices().size() == 0);
    const int updateSize = updateAll ? renderArray.size() : cache->updateIndices().size();
    QQuaternion seriesRotation(cache->meshRotation());

    if (!updateSize)
        return;

    // Index vertices
    const QList<QVector3D> indexed_vertices = dotObj->indexedvertices();
    int verticeCount = indexed_vertices.size();

    float itemSize = cache->itemSize() / itemScaler;
    if (itemSize == 0.0f)
        itemSize = dotScale;
    QVector3D modelScaler(itemSize, itemSize, itemSize);
    QMatrix4x4 modelMatrix;
    if (!seriesRotation.isIdentity()) {
        QMatrix4x4 matrix;
        matrix.rotate(seriesRotation);
        modelMatrix = matrix.transposed();
    }
    modelMatrix.scale(modelScaler);

    QList<QVector3D> scaled_vertices;
    scaled_vertices.resize(verticeCount);
    for (int i = 0; i < verticeCount; i++)
        scaled_vertices[i] = (QVector4D(indexed_vertices[i]) * modelMatrix).toVector3D();

    QList<QVector3D> buffered_vertices;
    buffered_vertices.resize(verticeCount * updateSize);

    int itemCount = 0;
    for (int i = 0; i < updateSize; i++) {
        int index = updateAll ? i : cache->updateIndices().at(i);
        const ScatterRenderItem &item = renderArray.at(index);
        if (!item.isVisible())
            continue;

        const int offset = itemCount * verticeCount;
        if (item.rotation().isIdentity()) {
            for (int j = 0; j < verticeCount; j++)
                buffered_vertices[j + offset] = scaled_vertices[j] + item.translation();
        } else {
            QMatrix4x4 matrix;
            matrix.rotate(seriesRotation * item.rotation());
            modelMatrix = matrix.transposed();
            modelMatrix.scale(modelScaler);

            for (int j = 0; j < verticeCount; j++) {
                buffered_vertices[j + offset]
                        = (QVector4D(indexed_vertices[j]) * modelMatrix).toVector3D()
                        + item.translation();
            }
        }
        itemCount++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
    int sizeOfItem = verticeCount * sizeof(QVector3D);
    if (updateAll) {
        if (itemCount) {
            glBufferData(GL_ARRAY_BUFFER, itemCount * sizeOfItem,
                         &buffered_vertices.at(0), GL_STATIC_DRAW);
        }
    } else {
        itemCount = 0;
        for (int i = 0; i < updateSize; i++) {
            int index = updateAll ? i : cache->updateIndices().at(i);
            if (renderArray.at(index).isVisible()) {
                glBufferSubData(GL_ARRAY_BUFFER, cache->bufferIndices().at(index) * sizeOfItem,
                                sizeOfItem, &buffered_vertices.at(itemCount * verticeCount));
                itemCount++;
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_meshDataLoaded = true;
}

QT_END_NAMESPACE
