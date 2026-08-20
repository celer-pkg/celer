// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "abstractobjecthelper_p.h"

QT_BEGIN_NAMESPACE

AbstractObjectHelper::AbstractObjectHelper()
    : m_vertexbuffer(0),
      m_normalbuffer(0),
      m_uvbuffer(0),
      m_elementbuffer(0),
      m_indexCount(0),
      m_meshDataLoaded(false)
{
    initializeOpenGLFunctions();
}

AbstractObjectHelper::~AbstractObjectHelper()
{
    if (QOpenGLContext::currentContext()) {
        glDeleteBuffers(1, &m_vertexbuffer);
        glDeleteBuffers(1, &m_uvbuffer);
        glDeleteBuffers(1, &m_normalbuffer);
        glDeleteBuffers(1, &m_elementbuffer);
    }
}

GLuint AbstractObjectHelper::vertexBuf()
{
    if (!m_meshDataLoaded)
        qFatal("No loaded object");
    return m_vertexbuffer;
}

GLuint AbstractObjectHelper::normalBuf()
{
    if (!m_meshDataLoaded)
        qFatal("No loaded object");
    return m_normalbuffer;
}

GLuint AbstractObjectHelper::uvBuf()
{
    if (!m_meshDataLoaded)
        qFatal("No loaded object");
    return m_uvbuffer;
}

GLuint AbstractObjectHelper::elementBuf()
{
    if (!m_meshDataLoaded)
        qFatal("No loaded object");
    return m_elementbuffer;
}

GLuint AbstractObjectHelper::indexCount()
{
    return m_indexCount;
}

QT_END_NAMESPACE
