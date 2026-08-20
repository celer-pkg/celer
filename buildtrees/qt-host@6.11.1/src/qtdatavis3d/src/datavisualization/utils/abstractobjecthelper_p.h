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

#ifndef ABSTRACTOBJECTHELPER_H
#define ABSTRACTOBJECTHELPER_H

#include "datavisualizationglobal_p.h"

QT_BEGIN_NAMESPACE

class AbstractObjectHelper: protected QOpenGLFunctions
{
protected:
    AbstractObjectHelper();
public:
    virtual ~AbstractObjectHelper();

    GLuint vertexBuf();
    GLuint normalBuf();
    virtual GLuint uvBuf();
    GLuint elementBuf();
    GLuint indexCount();

public:
    GLuint m_vertexbuffer;
    GLuint m_normalbuffer;
    GLuint m_uvbuffer;
    GLuint m_elementbuffer;

    GLuint m_indexCount;
    GLboolean m_meshDataLoaded;
};

QT_END_NAMESPACE

#endif
