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

#ifndef OBJECTHELPER_P_H
#define OBJECTHELPER_P_H

#include "datavisualizationglobal_p.h"
#include "abstractobjecthelper_p.h"

QT_BEGIN_NAMESPACE

class Abstract3DRenderer;

class ObjectHelper : public AbstractObjectHelper
{
private:
    ObjectHelper(const QString &objectFile);
public:
    virtual ~ObjectHelper();

    static void resetObjectHelper(const Abstract3DRenderer *cacheId, ObjectHelper *&obj,
                                  const QString &meshFile);
    static void releaseObjectHelper(const Abstract3DRenderer *cacheId, ObjectHelper *&obj);
    inline const QString &objectFile() { return m_objectFile; }

    inline const QList<GLuint> &indices() const { return m_indices; }
    inline const QList<QVector3D> &indexedvertices() const { return m_indexedVertices; }
    inline const QList<QVector2D> &indexedUVs() const { return m_indexedUVs; }
    inline const QList<QVector3D> &indexedNormals() const { return m_indexedNormals; }

private:
    static ObjectHelper *getObjectHelper(const Abstract3DRenderer *cacheId,
                                         const QString &objectFile);
    void load();

    QString m_objectFile;
    QList<GLuint> m_indices;
    QList<QVector3D> m_indexedVertices;
    QList<QVector2D> m_indexedUVs;
    QList<QVector3D> m_indexedNormals;
};

QT_END_NAMESPACE

#endif
