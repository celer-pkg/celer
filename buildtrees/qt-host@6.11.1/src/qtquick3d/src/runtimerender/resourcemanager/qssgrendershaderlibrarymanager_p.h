// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_DYNAMIC_OBJECT_SYSTEM_H
#define QSSG_RENDER_DYNAMIC_OBJECT_SYSTEM_H

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

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DUtils/private/qqsbcollection_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>

#include <QtGui/QVector2D>

#include <QtCore/QString>
#include <QtCore/QReadWriteLock>

QT_BEGIN_NAMESPACE

class QSSGRenderContextInterface;

struct QSSGCustomShaderMetaData
{
    enum Flag {
        UsesScreenTexture = 1 << 0,
        UsesDepthTexture = 1 << 1,
        UsesAoTexture = 1 << 2,
        OverridesPosition = 1 << 3,
        UsesProjectionMatrix = 1 << 4,
        UsesInverseProjectionMatrix = 1 << 5,
        UsesScreenMipTexture = 1 << 6,
        UsesVarColor = 1 << 7,
        UsesSharedVars = 1 << 8,
        UsesIblOrientation = 1 << 9,
        UsesLightmap = 1 << 10,
        UsesSkinning = 1 << 11,
        UsesMorphing = 1 << 12,
        UsesViewIndex = 1 << 13,
        UsesInputTexture = 1 << 14,
        UsesClearcoat = 1 << 15,
        UsesClearcoatFresnelScaleBias = 1 << 16,
        UsesFresnelScaleBias = 1 << 17,
        UsesTransmission = 1 << 18,
        UsesViewMatrix = 1 << 19,
        UsesNormalTexture = 1 << 20,
        UsesMotionVectorTexture = 1 << 21
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    Flags flags;
    QSet<QByteArray> customFunctions;
    QSSGShaderFeatures features;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGCustomShaderMetaData::Flags)

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGShaderLibraryManager
{
    Q_DISABLE_COPY(QSSGShaderLibraryManager)
public:
    typedef QHash<QByteArray, QByteArray> TPathDataMap;
    typedef QSet<QString> TPathSet;

    TPathDataMap m_expandedFiles;
    QHash<QByteArray, QSSGCustomShaderMetaData> m_metadata;
    QByteArray m_vertShader;
    QByteArray m_fragShader;

    QQsbCollection::EntryMap m_preGeneratedShaderEntries;

    QReadWriteLock m_lock;

    static QString getShaderCodeLibraryDirectory();

    explicit QSSGShaderLibraryManager();

    ~QSSGShaderLibraryManager();

    QQsbCollection::EntryMap getParticleShaderEntries() const;

    void setShaderSource(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type,
                         const QByteArray &inSource, const QSSGCustomShaderMetaData &meta);

    // Does not load any shaders, only information about the content of the pregenerated shaders
    void loadPregeneratedShaderInfo();

    void resolveIncludeFiles(QByteArray &theReadBuffer, const QByteArray &inMaterialInfoString);
    QByteArray getIncludeContents(const QByteArray &inShaderPathKey);

    QByteArray getShaderSource(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type);
    QSSGCustomShaderMetaData getShaderMetaData(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type);

    void setShaderCodeLibraryVersion(const QByteArray &version);

    static bool compare(const QSSGShaderDefaultMaterialKey &key1, const QSSGShaderDefaultMaterialKey &key2);
};

QT_END_NAMESPACE

#endif
