// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlecustomshape_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticleutils_p.h"
#include "qquick3dparticleshapedatautils_p.h"
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlfile.h>
#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleCustomShape3D
    \inherits ParticleAbstractShape3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Loads custom particle shapes for emitters and affectors.
    \since 6.3

    The ParticleCustomShape3D element can be used to load custom particle shapes.

    For example, to emit particles from positions defined in heart.cbor:

    \qml
    ParticleEmitter3D {
        shape: ParticleCustomShape3D {
            source: "heart.cbor"
        }
        ...
    }
    \endqml

    The format of CBOR shape files is following:
    \badcode
    [
      "QQ3D_SHAPE", // string
      version, // integer
      [
        posX, // float
        posY, // float
        posZ, // float
        posX, // float
        ...
      ]
    ]
    \endcode

    To assist in generating these shape files you can use the shapegen tool.
*/

QQuick3DParticleCustomShape::QQuick3DParticleCustomShape(QObject *parent)
    : QQuick3DParticleAbstractShape(parent)
{
}

/*!
    \qmlproperty url ParticleCustomShape3D::source

    This property holds the location of the shape file.

     \warning \a source is expected to contain trusted content. Application
     developers are advised to carefully consider the potential implications
     before passing in user-provided source files that are not part of the
     application.
*/

QUrl QQuick3DParticleCustomShape::source() const
{
    return m_source;
}

/*!
    \qmlproperty bool ParticleCustomShape3D::randomizeData

    This property holds whether the particles are used in random order instead
    of in the order they are specified in the source.

    The default value is \c false.
*/
bool QQuick3DParticleCustomShape::randomizeData() const
{
    return m_random;
}

void QQuick3DParticleCustomShape::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;

    loadFromSource();
    Q_EMIT sourceChanged();
}

void QQuick3DParticleCustomShape::setRandomizeData(bool random)
{
    if (m_random == random)
        return;

    m_random = random;
    if (m_random)
        m_randomizeDirty = true;
    Q_EMIT randomizeDataChanged();
}

void QQuick3DParticleCustomShape::loadFromSource()
{
    m_positions.clear();

    // Get path to file
    const QQmlContext *context = qmlContext(this);
    QString dataFilePath = QQmlFile::urlToLocalFileOrQrc(context ? context->resolvedUrl(m_source) : m_source);

    QFile dataFile(dataFilePath);
    if (!dataFile.open(QIODevice::ReadOnly)) {
        // Invalid file
        qWarning() << "Unable to open file:" << dataFilePath;
        return;
    }
    QCborStreamReader reader(&dataFile);

    // Check that file is proper CBOR and get the version
    int version = QQuick3DParticleShapeDataUtils::readShapeHeader(reader);

    if (version == -1) {
        // Invalid file
        qWarning() << "Invalid shape data version:" << version;
        return;
    }

    // Start positions array
    reader.enterContainer();

    m_center = QVector3D();
    while (reader.lastError() == QCborError::NoError && reader.hasNext()) {
        QVector3D pos = QQuick3DParticleShapeDataUtils::readValue(reader, QMetaType::QVector3D).value<QVector3D>();
        m_positions.append(pos);
        m_center += pos;
    }
    if (!m_positions.isEmpty())
        m_center *= 1.0f / float(m_positions.size());

    // Leave positions array
    reader.leaveContainer();

    // Leave root array
    reader.leaveContainer();

    if (m_random)
        m_randomizeDirty = true;
}

void QQuick3DParticleCustomShape::doRandomizeData()
{
    if (!m_system || m_positions.isEmpty())
        return;

    auto rand = m_system->rand();
    int seed = rand->get(0, QPRand::Shape1) * float(INT_MAX);
    std::shuffle(m_positions.begin(), m_positions.end(), std::default_random_engine(seed));

    m_randomizeDirty = false;
}

QVector3D QQuick3DParticleCustomShape::getPosition(int particleIndex)
{
    auto *parent = parentNode();
    if (!parent || m_positions.isEmpty())
        return QVector3D();

    if (m_randomizeDirty)
        doRandomizeData();

    int index = particleIndex % m_positions.size();
    return m_positions.at(index) * parent->scale();
}

QVector3D QQuick3DParticleCustomShape::getSurfaceNormal(int particleIndex)
{
    auto *parent = parentNode();
    if (!parent || m_positions.isEmpty())
        return QVector3D();

    if (m_randomizeDirty)
        doRandomizeData();

    int index = particleIndex % m_positions.size();
    return (m_positions.at(index) - m_center).normalized();
}

QT_END_NAMESPACE
