// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendershaderlibrarymanager_p.h"

#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderloadedtexture_p.h>

#include <QXmlStreamReader>
#include <QFileInfo>
#include <QCryptographicHash>

#include <QtQuick3DRuntimeRender/private/qssgruntimerenderlogging_p.h>

QT_BEGIN_NAMESPACE

QString QSSGShaderLibraryManager::getShaderCodeLibraryDirectory()
{
    return QStringLiteral("res/effectlib");
}
static QByteArray includeSearch() { return QByteArrayLiteral("#include \""); };
static QByteArray copyrightHeaderStart() { return QByteArrayLiteral("/****************************************************************************"); }
static QByteArray copyrightHeaderEnd() { return QByteArrayLiteral("****************************************************************************/"); }

QSSGShaderLibraryManager::QSSGShaderLibraryManager() {}

QSSGShaderLibraryManager::~QSSGShaderLibraryManager() {}

static inline char stageKey(QSSGShaderCache::ShaderType type)
{
    switch (type) {
    case QSSGShaderCache::ShaderType::Vertex:
        return 'V';
    case QSSGShaderCache::ShaderType::Fragment:
        return 'F';
    default:
        break;
    }
    return '?';
}

void QSSGShaderLibraryManager::setShaderSource(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type,
                                               const QByteArray &inSource, const QSSGCustomShaderMetaData &meta)
{
    QWriteLocker locker(&m_lock);

    const QByteArray perStageKey = stageKey(type) + inShaderPathKey;
    {
        auto it = m_expandedFiles.find(perStageKey);
        if (it != m_expandedFiles.end())
            it.value() = inSource;
        else
            m_expandedFiles.insert(perStageKey, inSource);
    }

    {
        auto it = m_metadata.find(perStageKey);
        if (it != m_metadata.end())
            it.value() = meta;
        else
            m_metadata.insert(perStageKey, meta);
    }
}

void QSSGShaderLibraryManager::resolveIncludeFiles(QByteArray &theReadBuffer, const QByteArray &inMaterialInfoString)
{
    // Now do search and replace for the headers
    for (int thePos = theReadBuffer.indexOf(includeSearch()); thePos != -1;
         thePos = theReadBuffer.indexOf(includeSearch(), thePos + 1)) {
        int theEndQuote = theReadBuffer.indexOf('\"', thePos + includeSearch().size() + 1);
        // Indicates an unterminated include file.
        if (theEndQuote == -1) {
            qCCritical(INVALID_OPERATION, "Unterminated include in file: %s", inMaterialInfoString.constData());
            theReadBuffer.clear();
            break;
        }
        const int theActualBegin = thePos + includeSearch().size();
        const auto &theInclude = theReadBuffer.mid(theActualBegin, theEndQuote - theActualBegin);
        // If we haven't included the file yet this round
        auto contents = getIncludeContents(theInclude);
        // Strip copywrite headers from include if present
        if (contents.startsWith(copyrightHeaderStart())) {
            int clipPos = contents.indexOf(copyrightHeaderEnd()) ;
            if (clipPos >= 0)
                contents.remove(0, clipPos + copyrightHeaderEnd().size());
        }
        // Write insert comment for begin source
        contents.prepend(QByteArrayLiteral("\n// begin \"") + theInclude + QByteArrayLiteral("\"\n"));
        // Write insert comment for end source
        contents.append(QByteArrayLiteral("\n// end \"" ) + theInclude + QByteArrayLiteral("\"\n"));

        theReadBuffer = theReadBuffer.replace(thePos, (theEndQuote + 1) - thePos, contents);
    }
}

QByteArray QSSGShaderLibraryManager::getIncludeContents(const QByteArray &inShaderPathKey)
{
    QWriteLocker locker(&m_lock);

    auto theInsert = m_expandedFiles.constFind(inShaderPathKey);
    const bool found = (theInsert != m_expandedFiles.cend());

    QByteArray theReadBuffer;
    if (!found) {
        const QString defaultDir = getShaderCodeLibraryDirectory();
        const auto ver = QByteArrayLiteral("rhi");

        QString fullPath;
        QSharedPointer<QIODevice> theStream;
        QTextStream stream(&fullPath);
        stream << defaultDir << QLatin1Char('/') << ver << QLatin1Char('/') << QString::fromLocal8Bit(inShaderPathKey);
        theStream = QSSGInputUtil::getStreamForFile(fullPath, true);
        if (theStream.isNull()) {
            fullPath.clear();
            QTextStream stream(&fullPath);
            stream << defaultDir << QLatin1Char('/') << QString::fromLocal8Bit(inShaderPathKey);
            theStream = QSSGInputUtil::getStreamForFile(fullPath, false);
        }
        if (!theStream.isNull()) {
            char readBuf[1024];
            qint64 amountRead = 0;
            do {
                amountRead = theStream->read(readBuf, 1024);
                if (amountRead)
                    theReadBuffer.append(readBuf, int(amountRead));
            } while (amountRead);
        } else {
            qCCritical(INVALID_OPERATION, "Failed to find include file %s", qPrintable(QString::fromLocal8Bit(inShaderPathKey)));
            Q_ASSERT(false);
        }
        theInsert = m_expandedFiles.insert(inShaderPathKey, theReadBuffer);
    } else {
        theReadBuffer = theInsert.value();
    }

    locker.unlock();
    resolveIncludeFiles(theReadBuffer, inShaderPathKey);

    return theReadBuffer;
}

QByteArray QSSGShaderLibraryManager::getShaderSource(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type)
{
    QReadLocker locker(&m_lock);

    const QByteArray perStageKey = stageKey(type) + inShaderPathKey;
    auto it = m_expandedFiles.constFind(perStageKey);
    if (it != m_expandedFiles.cend())
        return it.value();

    qWarning("No shader source stored for key %s", perStageKey.constData());
    return QByteArray();
}

QSSGCustomShaderMetaData QSSGShaderLibraryManager::getShaderMetaData(const QByteArray &inShaderPathKey, QSSGShaderCache::ShaderType type)
{
    QReadLocker locker(&m_lock);

    const QByteArray perStageKey = stageKey(type) + inShaderPathKey;
    auto it = m_metadata.constFind(perStageKey);
    if (it != m_metadata.cend())
        return it.value();

    qWarning("No shader metadata stored for key %s", perStageKey.constData());
    return {};
}

void QSSGShaderLibraryManager::loadPregeneratedShaderInfo()
{
    const auto collectionFilePath = QString::fromLatin1(QSSGShaderCache::resourceFolder() + QSSGShaderCache::shaderCollectionFile());
    QFile file(collectionFilePath);
    if (file.exists()) {
        QQsbIODeviceCollection qsbc(file);
        if (qsbc.map(QQsbIODeviceCollection::Read))
            m_preGeneratedShaderEntries = qsbc.availableEntries();
        qsbc.unmap();
    }
}

QQsbCollection::EntryMap QSSGShaderLibraryManager::getParticleShaderEntries() const
{
    QQsbCollection::EntryMap map;
    const auto collectionFilePath = QString::fromLatin1(QSSGShaderCache::resourceFolder() + QSSGShaderCache::particleShaderCollectionFile());
    QFile file(collectionFilePath);
    if (file.exists()) {
        QQsbIODeviceCollection qsbc(file);
        if (qsbc.map(QQsbIODeviceCollection::Read))
            map = qsbc.availableEntries();
        qsbc.unmap();
    }
    return map;
}

bool QSSGShaderLibraryManager::compare(const QSSGShaderDefaultMaterialKey &key1, const QSSGShaderDefaultMaterialKey &key2)
{
    QSSGShaderDefaultMaterialKeyProperties props;
#define COMPARE_PROP(x) \
    if (props.x.getValue(key1) < props.x.getValue(key2)) return true;

    COMPARE_PROP(m_hasLighting)
    COMPARE_PROP(m_hasIbl)
    COMPARE_PROP(m_fresnelEnabled)
    COMPARE_PROP(m_fresnelScaleBiasEnabled)
    COMPARE_PROP(m_clearcoatFresnelScaleBiasEnabled)
    COMPARE_PROP(m_baseColorSingleChannelEnabled)
    COMPARE_PROP(m_specularSingleChannelEnabled)
    COMPARE_PROP(m_emissiveSingleChannelEnabled)
    COMPARE_PROP(m_invertOpacityMapValue)
    COMPARE_PROP(m_vertexColorsEnabled)
    COMPARE_PROP(m_vertexColorsMaskEnabled)
    COMPARE_PROP(m_vertexColorRedMask)
    COMPARE_PROP(m_vertexColorGreenMask)
    COMPARE_PROP(m_vertexColorBlueMask)
    COMPARE_PROP(m_vertexColorAlphaMask)
    COMPARE_PROP(m_vertexAttributes)
    COMPARE_PROP(m_alphaMode)

    for (int i = 0; i < QSSGShaderDefaultMaterialKeyProperties::ImageMapCount; i++) {
        COMPARE_PROP(m_imageMaps[i])
    }
    for (int i = 0; i < QSSGShaderDefaultMaterialKeyProperties::SingleChannelImageCount; i++) {
        COMPARE_PROP(m_textureChannels[i])
    }
#undef COMPARE_PROP
    return false;
}

QT_END_NAMESPACE
