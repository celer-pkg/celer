// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "meshloader_p.h"
#include "vertexindexer_p.h"
#include "objecthelper_p.h"

QT_BEGIN_NAMESPACE

ObjectHelper::ObjectHelper(const QString &objectFile)
    : m_objectFile(objectFile)
{
    load();
}

struct ObjectHelperRef {
    int refCount;
    ObjectHelper *obj;
};

// The "Abstract3DRenderer *" key identifies the renderer
static QHash<const Abstract3DRenderer *, QHash<QString, ObjectHelperRef *> *> cacheTable;

ObjectHelper::~ObjectHelper()
{
}

void ObjectHelper::resetObjectHelper(const Abstract3DRenderer *cacheId, ObjectHelper *&obj,
                                     const QString &meshFile)
{
    Q_ASSERT(cacheId);

    if (obj) {
        const QString &oldFile = obj->objectFile();
        if (meshFile == oldFile)
            return; // same file, do nothing
        releaseObjectHelper(cacheId, obj);
    }
    obj = getObjectHelper(cacheId, meshFile);
}

void ObjectHelper::releaseObjectHelper(const Abstract3DRenderer *cacheId, ObjectHelper *&obj)
{
    Q_ASSERT(cacheId);

    if (obj) {
        QHash<QString, ObjectHelperRef *> *objectTable = cacheTable.value(cacheId, 0);
        if (objectTable) {
            // Delete object if last reference is released
            ObjectHelperRef *objRef = objectTable->value(obj->m_objectFile, 0);
            if (objRef) {
                objRef->refCount--;
                if (objRef->refCount <= 0) {
                    objectTable->remove(obj->m_objectFile);
                    delete objRef->obj;
                    delete objRef;
                }
            }
            if (objectTable->isEmpty()) {
                // Remove the entire cache if last object was removed
                cacheTable.remove(cacheId);
                delete objectTable;
            }
        } else {
            // Just delete the object if unknown cache
            delete obj;
        }
        obj = 0;
    }
}

ObjectHelper *ObjectHelper::getObjectHelper(const Abstract3DRenderer *cacheId,
                                            const QString &objectFile)
{
    if (objectFile.isEmpty())
        return 0;

    QHash<QString, ObjectHelperRef *> *objectTable = cacheTable.value(cacheId, 0);
    if (!objectTable) {
        objectTable = new QHash<QString, ObjectHelperRef *>;
        cacheTable.insert(cacheId, objectTable);
    }

    // Check if object helper for this mesh already exists
    ObjectHelperRef *objRef = objectTable->value(objectFile, 0);
    if (!objRef) {
        objRef = new ObjectHelperRef;
        objRef->refCount = 0;
        objRef->obj = new ObjectHelper(objectFile);
        if (objRef->obj->m_meshDataLoaded) {
            objectTable->insert(objectFile, objRef);
        } else {
            delete objRef->obj;
            delete objRef;
            objRef = nullptr;
        }
    }
    if (objRef) {
        objRef->refCount++;
        return objRef->obj;
    }
    return nullptr;
}

void ObjectHelper::load()
{
    if (m_meshDataLoaded) {
        // Delete old data
        glDeleteBuffers(1, &m_vertexbuffer);
        glDeleteBuffers(1, &m_uvbuffer);
        glDeleteBuffers(1, &m_normalbuffer);
        glDeleteBuffers(1, &m_elementbuffer);
        m_indices.clear();
        m_indexedVertices.clear();
        m_indexedUVs.clear();
        m_indexedNormals.clear();
        m_vertexbuffer = 0;
        m_uvbuffer = 0;
        m_normalbuffer = 0;
        m_elementbuffer = 0;
    }
    QList<QVector3D> vertices;
    QList<QVector2D> uvs;
    QList<QVector3D> normals;
    bool loadOk = MeshLoader::loadOBJ(m_objectFile, vertices, uvs, normals);

    if (!loadOk) {
        qCritical() << "Loading" << m_objectFile << "failed";
        m_meshDataLoaded = false;
    } else {
        // Index vertices
        VertexIndexer::indexVBO(vertices, uvs, normals, m_indices, m_indexedVertices, m_indexedUVs,
                                m_indexedNormals);

        m_indexCount = m_indices.size();

        glGenBuffers(1, &m_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_indexedVertices.size() * sizeof(QVector3D),
                     &m_indexedVertices.at(0),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &m_normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_indexedNormals.size() * sizeof(QVector3D),
                     &m_indexedNormals.at(0),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &m_uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_indexedUVs.size() * sizeof(QVector2D),
                     &m_indexedUVs.at(0), GL_STATIC_DRAW);

        glGenBuffers(1, &m_elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
                     &m_indices.at(0), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_meshDataLoaded = true;
    }
}

QT_END_NAMESPACE
