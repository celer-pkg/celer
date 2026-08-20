// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleutils_p.h"
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>

#include <QtQuick3D/QQuick3DGeometry>
#include <QtGui/qquaternion.h>
#include <QtCore/qdir.h>

#include <iterator>

QT_BEGIN_NAMESPACE

Q_CONSTINIT const float qt_quick3d_sine_table[] = {
    float(0.0),
    float(0.024541228522912288),
    float(0.049067674327418015),
    float(0.073564563599667426),
    float(0.098017140329560604),
    float(0.1224106751992162),
    float(0.14673047445536175),
    float(0.17096188876030122),
    float(0.19509032201612825),
    float(0.2191012401568698),
    float(0.24298017990326387),
    float(0.26671275747489837),
    float(0.29028467725446233),
    float(0.31368174039889152),
    float(0.33688985339222005),
    float(0.35989503653498811),
    float(0.38268343236508978),
    float(0.40524131400498986),
    float(0.42755509343028208),
    float(0.44961132965460654),
    float(0.47139673682599764),
    float(0.49289819222978404),
    float(0.51410274419322166),
    float(0.53499761988709715),
    float(0.55557023301960218),
    float(0.57580819141784534),
    float(0.59569930449243336),
    float(0.61523159058062682),
    float(0.63439328416364549),
    float(0.65317284295377676),
    float(0.67155895484701833),
    float(0.68954054473706683),
    float(0.70710678118654746),
    float(0.72424708295146689),
    float(0.74095112535495911),
    float(0.75720884650648446),
    float(0.77301045336273699),
    float(0.78834642762660623),
    float(0.80320753148064483),
    float(0.81758481315158371),
    float(0.83146961230254524),
    float(0.84485356524970701),
    float(0.85772861000027212),
    float(0.87008699110871135),
    float(0.88192126434835494),
    float(0.89322430119551532),
    float(0.90398929312344334),
    float(0.91420975570353069),
    float(0.92387953251128674),
    float(0.93299279883473885),
    float(0.94154406518302081),
    float(0.94952818059303667),
    float(0.95694033573220894),
    float(0.96377606579543984),
    float(0.97003125319454397),
    float(0.97570213003852857),
    float(0.98078528040323043),
    float(0.98527764238894122),
    float(0.98917650996478101),
    float(0.99247953459870997),
    float(0.99518472667219682),
    float(0.99729045667869021),
    float(0.99879545620517241),
    float(0.99969881869620425),
    float(1.0),
    float(0.99969881869620425),
    float(0.99879545620517241),
    float(0.99729045667869021),
    float(0.99518472667219693),
    float(0.99247953459870997),
    float(0.98917650996478101),
    float(0.98527764238894122),
    float(0.98078528040323043),
    float(0.97570213003852857),
    float(0.97003125319454397),
    float(0.96377606579543984),
    float(0.95694033573220894),
    float(0.94952818059303667),
    float(0.94154406518302081),
    float(0.93299279883473885),
    float(0.92387953251128674),
    float(0.91420975570353069),
    float(0.90398929312344345),
    float(0.89322430119551521),
    float(0.88192126434835505),
    float(0.87008699110871146),
    float(0.85772861000027212),
    float(0.84485356524970723),
    float(0.83146961230254546),
    float(0.81758481315158371),
    float(0.80320753148064494),
    float(0.78834642762660634),
    float(0.7730104533627371),
    float(0.75720884650648468),
    float(0.74095112535495899),
    float(0.72424708295146689),
    float(0.70710678118654757),
    float(0.68954054473706705),
    float(0.67155895484701855),
    float(0.65317284295377664),
    float(0.63439328416364549),
    float(0.61523159058062693),
    float(0.59569930449243347),
    float(0.57580819141784545),
    float(0.55557023301960218),
    float(0.53499761988709715),
    float(0.51410274419322177),
    float(0.49289819222978415),
    float(0.47139673682599786),
    float(0.44961132965460687),
    float(0.42755509343028203),
    float(0.40524131400498992),
    float(0.38268343236508989),
    float(0.35989503653498833),
    float(0.33688985339222033),
    float(0.31368174039889141),
    float(0.29028467725446239),
    float(0.26671275747489848),
    float(0.24298017990326407),
    float(0.21910124015687005),
    float(0.19509032201612861),
    float(0.17096188876030122),
    float(0.1467304744553618),
    float(0.12241067519921635),
    float(0.098017140329560826),
    float(0.073564563599667732),
    float(0.049067674327417966),
    float(0.024541228522912326),
    float(0.0),
    float(-0.02454122852291208),
    float(-0.049067674327417724),
    float(-0.073564563599667496),
    float(-0.09801714032956059),
    float(-0.1224106751992161),
    float(-0.14673047445536158),
    float(-0.17096188876030097),
    float(-0.19509032201612836),
    float(-0.2191012401568698),
    float(-0.24298017990326382),
    float(-0.26671275747489825),
    float(-0.29028467725446211),
    float(-0.31368174039889118),
    float(-0.33688985339222011),
    float(-0.35989503653498811),
    float(-0.38268343236508967),
    float(-0.40524131400498969),
    float(-0.42755509343028181),
    float(-0.44961132965460665),
    float(-0.47139673682599764),
    float(-0.49289819222978393),
    float(-0.51410274419322155),
    float(-0.53499761988709693),
    float(-0.55557023301960196),
    float(-0.57580819141784534),
    float(-0.59569930449243325),
    float(-0.61523159058062671),
    float(-0.63439328416364527),
    float(-0.65317284295377653),
    float(-0.67155895484701844),
    float(-0.68954054473706683),
    float(-0.70710678118654746),
    float(-0.72424708295146678),
    float(-0.74095112535495888),
    float(-0.75720884650648423),
    float(-0.77301045336273666),
    float(-0.78834642762660589),
    float(-0.80320753148064505),
    float(-0.81758481315158382),
    float(-0.83146961230254524),
    float(-0.84485356524970701),
    float(-0.85772861000027201),
    float(-0.87008699110871135),
    float(-0.88192126434835494),
    float(-0.89322430119551521),
    float(-0.90398929312344312),
    float(-0.91420975570353047),
    float(-0.92387953251128652),
    float(-0.93299279883473896),
    float(-0.94154406518302081),
    float(-0.94952818059303667),
    float(-0.95694033573220882),
    float(-0.96377606579543984),
    float(-0.97003125319454397),
    float(-0.97570213003852846),
    float(-0.98078528040323032),
    float(-0.98527764238894111),
    float(-0.9891765099647809),
    float(-0.99247953459871008),
    float(-0.99518472667219693),
    float(-0.99729045667869021),
    float(-0.99879545620517241),
    float(-0.99969881869620425),
    float(-1.0),
    float(-0.99969881869620425),
    float(-0.99879545620517241),
    float(-0.99729045667869021),
    float(-0.99518472667219693),
    float(-0.99247953459871008),
    float(-0.9891765099647809),
    float(-0.98527764238894122),
    float(-0.98078528040323043),
    float(-0.97570213003852857),
    float(-0.97003125319454397),
    float(-0.96377606579543995),
    float(-0.95694033573220894),
    float(-0.94952818059303679),
    float(-0.94154406518302092),
    float(-0.93299279883473907),
    float(-0.92387953251128663),
    float(-0.91420975570353058),
    float(-0.90398929312344334),
    float(-0.89322430119551532),
    float(-0.88192126434835505),
    float(-0.87008699110871146),
    float(-0.85772861000027223),
    float(-0.84485356524970723),
    float(-0.83146961230254546),
    float(-0.81758481315158404),
    float(-0.80320753148064528),
    float(-0.78834642762660612),
    float(-0.77301045336273688),
    float(-0.75720884650648457),
    float(-0.74095112535495911),
    float(-0.724247082951467),
    float(-0.70710678118654768),
    float(-0.68954054473706716),
    float(-0.67155895484701866),
    float(-0.65317284295377709),
    float(-0.63439328416364593),
    float(-0.61523159058062737),
    float(-0.59569930449243325),
    float(-0.57580819141784523),
    float(-0.55557023301960218),
    float(-0.53499761988709726),
    float(-0.51410274419322188),
    float(-0.49289819222978426),
    float(-0.47139673682599792),
    float(-0.44961132965460698),
    float(-0.42755509343028253),
    float(-0.40524131400499042),
    float(-0.38268343236509039),
    float(-0.359895036534988),
    float(-0.33688985339222),
    float(-0.31368174039889152),
    float(-0.2902846772544625),
    float(-0.26671275747489859),
    float(-0.24298017990326418),
    float(-0.21910124015687016),
    float(-0.19509032201612872),
    float(-0.17096188876030177),
    float(-0.14673047445536239),
    float(-0.12241067519921603),
    float(-0.098017140329560506),
    float(-0.073564563599667412),
    float(-0.049067674327418091),
    float(-0.024541228522912448)
};
static_assert(std::size(qt_quick3d_sine_table) == QT_QUICK3D_SINE_TABLE_SIZE);

QQuick3DNode *getSharedParentNode(QQuick3DNode *node, QQuick3DNode *system) {
    QQuick3DNode *systemSharedParent = nullptr;
    if (node && system) {
        QVector<QQuick3DNode *> parents;
        QQuick3DNode *parent = node->parentNode();
        while (parent) {
            parents.append(parent);
            parent = parent->parentNode();
        }

        parent = system;
        while (parent) {
            if (parents.contains(parent)) {
                systemSharedParent = parent;
                break;
            }
            parent = parent->parentNode();
        }
    }
    return systemSharedParent;
}

QMatrix4x4 calculateParticleTransform(const QQuick3DNode *parent, const QQuick3DNode *systemSharedParent)
{
    QMatrix4x4 transform = parent->sceneTransform();
    if (systemSharedParent)
        transform = systemSharedParent->sceneTransform().inverted() * transform;
    return transform;
}

QQuaternion calculateParticleRotation(const QQuick3DNode *parent, const QQuick3DNode *systemSharedParent)
{
    QQuaternion rotation = parent->sceneRotation();
    if (systemSharedParent)
        rotation = systemSharedParent->sceneRotation().inverted() * rotation;
    return rotation;
}

static QSSGMesh::Mesh loadModelShapeMesh(const QString &source)
{
    QString src = source;
    if (source.startsWith(QLatin1Char('#'))) {
        src = QSSGBufferManager::primitivePath(source);
        src.prepend(QLatin1String(":/"));
    }
    src = QDir::cleanPath(src);
    if (src.startsWith(QLatin1String("qrc:/")))
        src = src.mid(3);
    QSSGMesh::Mesh mesh;
    QFileInfo fileInfo = QFileInfo(src);
    if (fileInfo.exists()) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QFile::ReadOnly))
            return {};
        mesh = QSSGMesh::Mesh::loadMesh(&file);
    }
    return mesh;
}

QList<QVector3D> positionsFromModel(QQuick3DModel *model, const QMatrix4x4 *matrix, QQmlContext *context)
{
    QVector<QVector3D> indicedPositions;
    QVector<QVector3D> positions;
    if (QQuick3DGeometry *geometry = model->geometry()) {
        bool hasIndexBuffer = false;
        QQuick3DGeometry::Attribute::ComponentType indexBufferFormat;
        int posOffset = 0;
        QQuick3DGeometry::Attribute::ComponentType posType = QQuick3DGeometry::Attribute::U16Type;
        for (int i = 0; i < geometry->attributeCount(); ++i) {
            auto attribute = geometry->attribute(i);
            if (attribute.semantic == QQuick3DGeometry::Attribute::PositionSemantic) {
                posOffset = attribute.offset;
                posType = attribute.componentType;
            } else if (attribute.semantic == QQuick3DGeometry::Attribute::IndexSemantic) {
                hasIndexBuffer = true;
                indexBufferFormat = attribute.componentType;
            }
        }
        if (posType == QQuick3DGeometry::Attribute::F32Type) {
            const auto &data = geometry->vertexData();
            int stride = geometry->stride();
            for (int i = 0; i < data.size(); i += stride) {
                float v[3];
                memcpy(v, data + posOffset + i, sizeof(v));
                if (matrix)
                    positions.append(matrix->mapVector(QVector3D(v[0], v[1], v[2])));
                else
                    positions.append(QVector3D(v[0], v[1], v[2]));
            }
            if (hasIndexBuffer) {
                const auto &data = geometry->indexData();
                int indexSize = 4;
                if (indexBufferFormat == QQuick3DGeometry::Attribute::U16Type)
                    indexSize = 2;
                for (int i = 0; i < data.size(); i += indexSize) {
                    qsizetype index = 0;
                    memcpy(&index, data + i, indexSize);
                    if (positions.size() > index)
                        indicedPositions.append(positions[index]);
                }
            }
        }
    } else {
        QString src = model->source().toString();
        if (context && !src.startsWith(QLatin1Char('#'))) {
            if (src.contains(QStringLiteral("#")))
                src = src.right(src.length() - src.lastIndexOf(QStringLiteral("#")));
            else
                src = QQmlFile::urlToLocalFileOrQrc(context->resolvedUrl(model->source()));
        }
        QSSGMesh::Mesh mesh = loadModelShapeMesh(src);
        if (mesh.isValid() && mesh.drawMode() == QSSGMesh::Mesh::DrawMode::Triangles) {
            auto entries = mesh.vertexBuffer().entries;
            int posOffset = 0;
            int posCount = 0;
            // Just set 'posType' to something to avoid invalid 'maybe-uninitialized' warning
            QSSGMesh::Mesh::ComponentType posType = QSSGMesh::Mesh::ComponentType::UnsignedInt8;
            for (int i = 0; i < entries.size(); ++i) {
                const char *nameStr = entries[i].name.constData();
                if (!strcmp(nameStr, QSSGMesh::MeshInternal::getPositionAttrName())) {
                    posOffset = entries[i].offset;
                    posCount = entries[i].componentCount;
                    posType = entries[i].componentType;
                    break;
                }
            }
            if (posCount == 3 && posType == QSSGMesh::Mesh::ComponentType::Float32) {
                const auto &data = mesh.vertexBuffer().data;
                int stride = mesh.vertexBuffer().stride;
                for (int i = 0; i < data.size(); i += stride) {
                    float v[3];
                    memcpy(v, data + posOffset + i, sizeof(v));
                    if (matrix)
                        positions.append(matrix->map(QVector3D(v[0], v[1], v[2])));
                    else
                        positions.append(QVector3D(v[0], v[1], v[2]));
                }
                const auto &indexData = mesh.indexBuffer().data;
                int indexSize = QSSGMesh::MeshInternal::byteSizeForComponentType(mesh.indexBuffer().componentType);
                for (int i = 0; i < indexData.size(); i += indexSize) {
                    qsizetype index = 0;
                    memcpy(&index, indexData + i, indexSize);
                    if (positions.size() > index)
                        indicedPositions.append(positions[index]);
                }
            }
        }
    }
    if (!indicedPositions.empty())
        return indicedPositions;
    return positions;
}

QT_END_NAMESPACE
