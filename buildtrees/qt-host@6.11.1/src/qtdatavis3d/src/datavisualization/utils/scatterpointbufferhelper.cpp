// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterpointbufferhelper_p.h"
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

const QVector3D hiddenPos(-1000.0f, -1000.0f, -1000.0f);

ScatterPointBufferHelper::ScatterPointBufferHelper()
    : m_pointbuffer(0),
      m_oldRemoveIndex(-1)
{
}

ScatterPointBufferHelper::~ScatterPointBufferHelper()
{
    if (QOpenGLContext::currentContext())
        glDeleteBuffers(1, &m_pointbuffer);
}

GLuint ScatterPointBufferHelper::pointBuf()
{
    if (!m_meshDataLoaded)
        qFatal("No loaded object");
    return m_pointbuffer;
}

void ScatterPointBufferHelper::pushPoint(uint pointIndex)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_pointbuffer);

    // Pop the previous point if it is still pushed
    if (m_oldRemoveIndex >= 0) {
        glBufferSubData(GL_ARRAY_BUFFER, m_oldRemoveIndex * sizeof(QVector3D),
                        sizeof(QVector3D), &m_bufferedPoints.at(m_oldRemoveIndex));
    }

    glBufferSubData(GL_ARRAY_BUFFER, pointIndex * sizeof(QVector3D),
                    sizeof(QVector3D),
                    &hiddenPos);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_oldRemoveIndex = pointIndex;
}

void ScatterPointBufferHelper::popPoint()
{
    if (m_oldRemoveIndex >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pointbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, m_oldRemoveIndex * sizeof(QVector3D),
                        sizeof(QVector3D), &m_bufferedPoints.at(m_oldRemoveIndex));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    m_oldRemoveIndex = -1;
}

void ScatterPointBufferHelper::load(ScatterSeriesRenderCache *cache)
{
    ScatterRenderItemArray &renderArray = cache->renderArray();
    const int renderArraySize = renderArray.size();
    m_indexCount = 0;

    if (m_meshDataLoaded) {
        // Delete old data
        glDeleteBuffers(1, &m_pointbuffer);
        glDeleteBuffers(1, &m_uvbuffer);
        m_bufferedPoints.clear();
        m_pointbuffer = 0;
        m_uvbuffer = 0;
        m_meshDataLoaded = false;
    }

    bool itemsVisible = false;
    m_bufferedPoints.resize(renderArraySize);
    for (int i = 0; i < renderArraySize; i++) {
        const ScatterRenderItem &item = renderArray.at(i);
        if (!item.isVisible()) {
            m_bufferedPoints[i] = hiddenPos;
        } else {
            itemsVisible = true;
            m_bufferedPoints[i] = item.translation();
        }
    }

    QList<QVector2D> buffered_uvs;
    if (itemsVisible)
        m_indexCount = renderArraySize;

    if (m_indexCount > 0) {
        if (cache->colorStyle() == Q3DTheme::ColorStyleRangeGradient)
            createRangeGradientUVs(cache, buffered_uvs);

        glGenBuffers(1, &m_pointbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_pointbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_bufferedPoints.size() * sizeof(QVector3D),
                     &m_bufferedPoints.at(0),
                     GL_DYNAMIC_DRAW);

        if (buffered_uvs.size()) {
            glGenBuffers(1, &m_uvbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, buffered_uvs.size() * sizeof(QVector2D),
                         &buffered_uvs.at(0), GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_meshDataLoaded = true;
    }
}

void ScatterPointBufferHelper::update(ScatterSeriesRenderCache *cache)
{
    // It may be that the buffer hasn't yet been initialized, in case the entire series was
    // hidden items. No need to update in that case.
    if (m_indexCount > 0) {
        const ScatterRenderItemArray &renderArray = cache->renderArray();
        const int updateSize = cache->updateIndices().size();

        glBindBuffer(GL_ARRAY_BUFFER, m_pointbuffer);
        for (int i = 0; i < updateSize; i++) {
            int index = cache->updateIndices().at(i);
            const ScatterRenderItem &item = renderArray.at(index);
            if (!item.isVisible())
                m_bufferedPoints[index] = hiddenPos;
            else
                m_bufferedPoints[index] = item.translation();

            if (index != m_oldRemoveIndex) {
                glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(QVector3D),
                                sizeof(QVector3D), &m_bufferedPoints.at(index));
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ScatterPointBufferHelper::updateUVs(ScatterSeriesRenderCache *cache)
{
    // It may be that the buffer hasn't yet been initialized, in case the entire series was
    // hidden items. No need to update in that case.
    if (m_indexCount > 0) {
        QList<QVector2D> buffered_uvs;
        createRangeGradientUVs(cache, buffered_uvs);

        if (buffered_uvs.size()) {
            if (!m_uvbuffer)
                glGenBuffers(1, &m_uvbuffer);

            int updateSize = cache->updateIndices().size();
            glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
            if (updateSize) {
                for (int i = 0; i < updateSize; i++) {
                    int index = cache->updateIndices().at(i);
                    glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(QVector2D),
                                    sizeof(QVector2D), &buffered_uvs.at(i));

                }
            } else {
                glBufferData(GL_ARRAY_BUFFER, buffered_uvs.size() * sizeof(QVector2D),
                             &buffered_uvs.at(0), GL_STATIC_DRAW);
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}

void ScatterPointBufferHelper::createRangeGradientUVs(ScatterSeriesRenderCache *cache,
                                                      QList<QVector2D> &buffered_uvs)
{
    const ScatterRenderItemArray &renderArray = cache->renderArray();
    const bool updateAll = (cache->updateIndices().size() == 0);
    const int updateSize = updateAll ? renderArray.size() : cache->updateIndices().size();
    buffered_uvs.resize(updateSize);

    QVector2D uv;
    uv.setX(0.0f);
    for (int i = 0; i < updateSize; i++) {
        int index = updateAll ? i : cache->updateIndices().at(i);
        const ScatterRenderItem &item = renderArray.at(index);

        float y = ((item.translation().y() + m_scaleY) * 0.5f) / m_scaleY;
        uv.setY(y);
        buffered_uvs[i] = uv;
    }
}

QT_END_NAMESPACE
