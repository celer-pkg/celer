// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Based on:
// https://behreajj.medium.com/making-a-capsule-mesh-via-script-in-five-3d-environments-c2214abf02db

#include "qcapsulegeometry_p.h"

#include <QVector3D>

QT_BEGIN_NAMESPACE

/*!
    \qmltype CapsuleGeometry
    \inqmlmodule QtQuick3D.Physics.Helpers
    \inherits Geometry
    \since 6.4
    \brief A geometry for generating a capsule model.
    \deprecated [6.10]

    Deprecated, use \l{QtQuick3D.Helpers.CapsuleGeometry}.

    A geometry for generating a capsule model.
*/

/*! \qmlproperty bool CapsuleGeometry::enableNormals
    \default true

    Generate mesh face normals.
*/

/*! \qmlproperty bool CapsuleGeometry::enableUV
    \default false

    Generate mesh uv coordinates.
*/

/*! \qmlproperty int CapsuleGeometry::longitudes
    \default 32

    Number of longitudes, or meridians, distributed by azimuth.
*/

/*! \qmlproperty int CapsuleGeometry::latitudes
    \default 16

    Number of latitudes, distributed by inclination. Must be even.
*/

/*! \qmlproperty int CapsuleGeometry::rings
    \default 1

    Number of sections in cylinder between hemispheres.
*/

/*! \qmlproperty real CapsuleGeometry::height
    \default 100

    Height of the middle cylinder on the y axis, excluding the hemispheres.
*/

/*! \qmlproperty real CapsuleGeometry::diameter
    \default 100

    Diameter on the xz plane.
*/

CapsuleGeometryPhysics::CapsuleGeometryPhysics()
{
    updateData();
}

void CapsuleGeometryPhysics::setEnableNormals(bool enable)
{
    if (m_enableNormals == enable)
        return;

    m_enableNormals = enable;
    emit enableNormalsChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setEnableUV(bool enable)
{
    if (m_enableUV == enable)
        return;

    m_enableUV = enable;
    emit enableUVChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setLongitudes(int longitudes)
{
    if (m_longitudes == longitudes)
        return;

    m_longitudes = longitudes;
    emit longitudesChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setLatitudes(int latitudes)
{
    if (m_latitudes == latitudes)
        return;

    m_latitudes = latitudes;
    emit latitudesChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setRings(int rings)
{
    if (m_rings == rings)
        return;

    m_rings = rings;
    emit ringsChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setHeight(float height)
{
    if (m_height == height)
        return;

    m_height = height;
    emit heightChanged();
    updateData();
    update();
}

void CapsuleGeometryPhysics::setDiameter(float diameter)
{
    if (m_diameter == diameter)
        return;

    m_diameter = diameter;
    emit diameterChanged();
    updateData();
    update();
}

struct Face
{
    // Coordinate index.
    uint32_t vertexIdx = 0;
    // Texture coordinate index.
    uint32_t textureIdx = 0;
    // Normal index.
    uint32_t normalIdx = 0;
};

void CapsuleGeometryPhysics::updateData()
{
    clear();

    constexpr float EPSILON = 0.001f;
    const float radius = m_diameter * 0.5f;

    // m_latitudes must be even for symmetry.
    int verifLats = qMax(2, m_latitudes);
    if (verifLats % 2 != 0) {
        verifLats += 1;
    }

    // Validate input arguments.
    uint32_t verifLons = qMax(3, m_longitudes);
    uint32_t verifRings = qMax(0, m_rings);
    float verifDepth = qMax(EPSILON, m_height);
    float verifRad = qMax(EPSILON, radius);

    // Intermediary calculations.
    bool calcMiddle = verifRings > 0;
    uint32_t halfLats = verifLats / 2;
    uint32_t halfLatsn1 = halfLats - 1;
    uint32_t halfLatsn2 = halfLats - 2;
    uint32_t verifRingsp1 = verifRings + 1;
    uint32_t verifLonsp1 = verifLons + 1;
    uint32_t lonsHalfLatn1 = halfLatsn1 * verifLons;
    uint32_t lonsRingsp1 = verifRingsp1 * verifLons;
    float halfDepth = verifDepth * 0.5f;
    float summit = halfDepth + verifRad;

    // Index offsets for coordinates.
    uint32_t idxVNEquator = verifLonsp1 + verifLons * halfLatsn2;
    uint32_t idxVCyl = idxVNEquator + verifLons;
    uint32_t idxVSEquator = idxVCyl;
    if (calcMiddle) {
        idxVSEquator += verifLons * verifRings;
    }
    uint32_t idxVSouth = idxVSEquator + verifLons;
    uint32_t idxVSouthCap = idxVSouth + verifLons * halfLatsn2;
    uint32_t idxVSouthPole = idxVSouthCap + verifLons;

    // Index offsets for texture coordinates.
    uint32_t idxVtNEquator = verifLons + verifLonsp1 * halfLatsn1;
    uint32_t idxVtCyl = idxVtNEquator + verifLonsp1;
    uint32_t idxVtSEquator = idxVtCyl;
    if (calcMiddle) {
        idxVtSEquator += verifLonsp1 * verifRings;
    }
    uint32_t idxVtSHemi = idxVtSEquator + verifLonsp1;
    uint32_t idxVtSPolar = idxVtSHemi + verifLonsp1 * halfLatsn2;
    uint32_t idxVtSCap = idxVtSPolar + verifLonsp1;

    // Index offsets for normals.
    uint32_t idxVnSouth = idxVNEquator + verifLons;
    uint32_t idxVnSouthCap = idxVnSouth + verifLons * halfLatsn2;
    uint32_t idxVnSouthPole = idxVnSouthCap + verifLons;

    // Find index offsets for face indices.
    uint32_t idxFsCyl = verifLons + lonsHalfLatn1 * 2;
    uint32_t idxFsSouthEquat = idxFsCyl + lonsRingsp1 * 2;
    uint32_t idxFsSouthHemi = idxFsSouthEquat + lonsHalfLatn1 * 2;

    // Array lengths.
    uint32_t verticesLen = idxVSouthPole + 1;
    uint32_t texturesLen = idxVtSCap + verifLons;
    uint32_t normalsLen = idxVnSouthPole + 1;
    uint32_t facesLen = idxFsSouthHemi + verifLons;

    // Initialize arrays.
    auto vertices = QList<QVector3D>(verticesLen);
    auto vertexTextures = QList<QVector2D>(texturesLen);
    auto vertexNormals = QList<QVector3D>(normalsLen);

    // If we plan to use only triangles, we can initialize
    // the inner array to 3.
    auto faces = QList<std::array<Face, 3>>(facesLen);

    // North pole.
    vertices[0] = QVector3D(-summit, 0.f, 0.f);
    vertexNormals[0] = QVector3D(-1.f, 0.f, 0.f);

    // South pole.
    vertices[idxVSouthPole] = QVector3D(summit, 0.f, 0.f);
    vertexNormals[idxVnSouthPole] = QVector3D(1.f, 0.f, 0.f);

    // Calculate polar texture coordinates, equatorial coordinates.
    QList<float> sinThetaCache = QList<float>(verifLons);
    QList<float> cosThetaCache = QList<float>(verifLons);
    float toTheta = 2 * M_PI / verifLons;
    float toPhi = M_PI / verifLats;
    float toTexHorizontal = 1.f / verifLons;
    float toTexVertical = 1.f / halfLats;

    for (uint32_t j = 0; j < verifLons; ++j) {

        // Coordinates.
        float theta = j * toTheta;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        sinThetaCache[j] = sinTheta;
        cosThetaCache[j] = cosTheta;

        // Texture coordinates at North and South pole.
        float sTex = (j + 0.5f) * toTexHorizontal;
        vertexTextures[j] = QVector2D(sTex, 1.f);
        vertexTextures[idxVtSCap + j] = QVector2D(sTex, 0.f);

        // Multiply by radius to get equatorial x and y.
        float x = verifRad * cosTheta;
        float z = verifRad * sinTheta;

        // Set equatorial coordinates. Offset by cylinder depth.
        vertices[idxVNEquator + j] = QVector3D(-halfDepth, x, -z);
        vertices[idxVSEquator + j] = QVector3D(halfDepth, x, -z);

        // Set equatorial normals.
        vertexNormals[idxVNEquator + j] = QVector3D(0.f, cosTheta, -sinTheta);

        // Set polar indices.
        uint32_t jNextVt = j + 1;
        uint32_t jNextV = jNextVt % verifLons;

        // North triangle.
        faces[j] = { Face { 0, j, 0 }, Face { jNextVt, verifLons + j, jNextVt },
                     Face { 1 + jNextV, verifLons + jNextVt, 1 + jNextV } };

        // South triangle.
        faces[idxFsSouthHemi + j] = {
            Face { idxVSouthPole, idxVtSCap + j, idxVnSouthPole },
            Face { idxVSouthCap + jNextV, idxVtSPolar + jNextVt, idxVnSouthCap + jNextV },
            Face { idxVSouthCap + j, idxVtSPolar + j, idxVnSouthCap + j }
        };
    }

    // Determine UV aspect ratio from the profile.
    float vtAspectRatio = 0.f;
    switch (m_uvProfile) {
    case CapsuleGeometryPhysics::UvProfile::Fixed:
        vtAspectRatio = 0.33333333f;
        break;
    case CapsuleGeometryPhysics::UvProfile::Aspect:
        vtAspectRatio = verifRad / (verifDepth + verifRad + verifRad);
        break;
    case CapsuleGeometryPhysics::UvProfile::Uniform:
        vtAspectRatio = (float)halfLats / (verifRingsp1 + verifLats);
        break;
    }
    float vtAspectSouth = vtAspectRatio;
    float vtAspectNorth = 1.f - vtAspectRatio;

    // Cache horizontal measure.
    QList<float> sTexCache = QList<float>(verifLonsp1);

    // Calculate equatorial texture coordinates.
    for (uint32_t j = 0; j < verifLonsp1; ++j) {
        float sTex = j * toTexHorizontal;
        sTexCache[j] = sTex;
        vertexTextures[idxVtNEquator + j] = QVector2D(sTex, vtAspectNorth);
        vertexTextures[idxVtSEquator + j] = QVector2D(sTex, vtAspectSouth);
    }

    // Divide m_latitudes into hemispheres. Start at i = 1 due to the poles.
    uint32_t vHemiOffsetNorth = 1;
    uint32_t vHemiOffsetSouth = idxVSouth;
    uint32_t vtHemiOffsetNorth = verifLons;
    uint32_t vtHemiOffsetSouth = idxVtSHemi;
    uint32_t vnHemiOffsetSouth = idxVnSouth;
    uint32_t fHemiOffsetNorth = verifLons;
    uint32_t fHemiOffsetSouth = idxFsSouthEquat;

    for (uint32_t i = 0; i < halfLatsn1; ++i) {
        uint32_t iLonsCurr = i * verifLons;
        float ip1f = i + 1.f;
        float phi = ip1f * toPhi;
        float sinPhiSouth = sin(phi);
        float cosPhiSouth = cos(phi);

        // Use trigonometric symmetries to avoid calculating another
        // sine and cosine for phi North.
        float cosPhiNorth = sinPhiSouth;
        float sinPhiNorth = -cosPhiSouth;

        // For North coordinates, multiply by radius and offset.
        float rhoCosPhiNorth = verifRad * cosPhiNorth;
        float rhoSinPhiNorth = verifRad * sinPhiNorth;
        float yOffsetNorth = halfDepth - rhoSinPhiNorth;

        // For South coordinates, multiply by radius and offset.
        float rhoCosPhiSouth = verifRad * cosPhiSouth;
        float rhoSinPhiSouth = verifRad * sinPhiSouth;
        float yOffsetSouth = -halfDepth - rhoSinPhiSouth;

        // North coordinate index offset.
        uint32_t vCurrLatN = 1 + iLonsCurr;
        uint32_t vNextLatN = vCurrLatN + verifLons;

        // South coordinate index offset.
        uint32_t vCurrLatS = idxVSEquator + iLonsCurr;
        uint32_t vNextLatS = vCurrLatS + verifLons;

        // North texture coordinate index offset.
        uint32_t vtCurrLatN = verifLons + i * verifLonsp1;
        uint32_t vtNextLatN = vtCurrLatN + verifLonsp1;

        // South texture coordinate index offset.
        uint32_t vtCurrLatS = idxVtSEquator + i * verifLonsp1;
        uint32_t vtNextLatS = vtCurrLatS + verifLonsp1;

        // North normal index offset.
        uint32_t vnCurrLatN = 1 + iLonsCurr;
        uint32_t vnNextLatN = vnCurrLatN + verifLons;

        // South normal index offset.
        uint32_t vnCurrLatS = idxVNEquator + iLonsCurr;
        uint32_t vnNextLatS = vnCurrLatS + verifLons;

        // Coordinates, normals and face indices.
        for (uint32_t j = 0; j < verifLons; ++j) {
            float sinTheta = sinThetaCache[j];
            float cosTheta = cosThetaCache[j];

            // North coordinate.
            vertices[vHemiOffsetNorth] =
                    QVector3D(-yOffsetNorth, rhoCosPhiNorth * cosTheta, -rhoCosPhiNorth * sinTheta);

            // North normal.
            vertexNormals[vHemiOffsetNorth] =
                    QVector3D(sinPhiNorth, cosPhiNorth * cosTheta, -cosPhiNorth * sinTheta);

            // South coordinate.
            vertices[vHemiOffsetSouth] =
                    QVector3D(-yOffsetSouth, rhoCosPhiSouth * cosTheta, -rhoCosPhiSouth * sinTheta);

            // South normal.
            vertexNormals[vnHemiOffsetSouth] =
                    QVector3D(sinPhiSouth, cosPhiSouth * cosTheta, -cosPhiSouth * sinTheta);

            ++vHemiOffsetNorth;
            ++vHemiOffsetSouth;
            ++vnHemiOffsetSouth;

            uint32_t jNextVt = j + 1;
            uint32_t jNextV = jNextVt % verifLons;

            // North coordinate indices.
            uint32_t vn00 = vCurrLatN + j;
            uint32_t vn01 = vNextLatN + j;
            uint32_t vn11 = vNextLatN + jNextV;
            uint32_t vn10 = vCurrLatN + jNextV;

            // South coordinate indices.
            uint32_t vs00 = vCurrLatS + j;
            uint32_t vs01 = vNextLatS + j;
            uint32_t vs11 = vNextLatS + jNextV;
            uint32_t vs10 = vCurrLatS + jNextV;

            // North texture coordinate indices.
            uint32_t vtn00 = vtCurrLatN + j;
            uint32_t vtn01 = vtNextLatN + j;
            uint32_t vtn11 = vtNextLatN + jNextVt;
            uint32_t vtn10 = vtCurrLatN + jNextVt;

            // South texture coordinate indices.
            uint32_t vts00 = vtCurrLatS + j;
            uint32_t vts01 = vtNextLatS + j;
            uint32_t vts11 = vtNextLatS + jNextVt;
            uint32_t vts10 = vtCurrLatS + jNextVt;

            // North normal indices.
            uint32_t vnn00 = vnCurrLatN + j;
            uint32_t vnn01 = vnNextLatN + j;
            uint32_t vnn11 = vnNextLatN + jNextV;
            uint32_t vnn10 = vnCurrLatN + jNextV;

            // South normal indices.
            uint32_t vns00 = vnCurrLatS + j;
            uint32_t vns01 = vnNextLatS + j;
            uint32_t vns11 = vnNextLatS + jNextV;
            uint32_t vns10 = vnCurrLatS + jNextV;

            // North triangles.
            faces[fHemiOffsetNorth] = { Face { vn00, vtn00, vnn00 }, Face { vn11, vtn11, vnn11 },
                                        Face { vn10, vtn10, vnn10 } };

            faces[fHemiOffsetNorth + 1] = { Face { vn00, vtn00, vnn00 },
                                            Face { vn01, vtn01, vnn01 },
                                            Face { vn11, vtn11, vnn11 } };

            // South triangles.
            faces[fHemiOffsetSouth] = { Face { vs00, vts00, vns00 }, Face { vs11, vts11, vns11 },
                                        Face { vs10, vts10, vns10 } };

            faces[fHemiOffsetSouth + 1] = { Face { vs00, vts00, vns00 },
                                            Face { vs01, vts01, vns01 },
                                            Face { vs11, vts11, vns11 } };

            fHemiOffsetNorth += 2;
            fHemiOffsetSouth += 2;
        }

        // For UVs, linear interpolation from North pole to
        // North aspect ratio; and from South pole to South
        // aspect ratio.
        float tTexFac = ip1f * toTexVertical;
        float tTexNorth = 1.f - tTexFac + tTexFac * vtAspectNorth;
        float tTexSouth = vtAspectSouth * (1.f - tTexFac);

        // Texture coordinates.
        for (uint32_t j = 0; j < verifLonsp1; ++j) {
            float sTex = sTexCache[j];

            vertexTextures[vtHemiOffsetNorth] = QVector2D(sTex, tTexNorth);
            vertexTextures[vtHemiOffsetSouth] = QVector2D(sTex, tTexSouth);

            ++vtHemiOffsetNorth;
            ++vtHemiOffsetSouth;
        }
    }

    // Calculate sections of cylinder in middle.
    if (calcMiddle) {

        // Linear interpolation must exclude the origin (North equator)
        // and the destination (South equator), so step must never equal
        // 0.0 or 1.0 .
        float toFac = 1.f / verifRingsp1;
        uint32_t vCylOffset = idxVCyl;
        uint32_t vtCylOffset = idxVtCyl;
        for (uint32_t m = 1; m < verifRingsp1; ++m) {
            float fac = m * toFac;
            float cmplFac = 1.f - fac;

            // Coordinates.
            for (uint32_t j = 0; j < verifLons; ++j) {
                QVector3D vEquatorNorth = vertices[idxVNEquator + j];
                QVector3D vEquatorSouth = vertices[idxVSEquator + j];

                // xy should be the same for both North and South.
                // North z should equal half_depth while South z
                // should equal -half_depth. However this is kept as
                // a linear interpolation for clarity.
                vertices[vCylOffset] =
                        QVector3D(cmplFac * vEquatorNorth.x() + fac * vEquatorSouth.x(),
                                  cmplFac * vEquatorNorth.y() + fac * vEquatorSouth.y(),
                                  cmplFac * vEquatorNorth.z() + fac * vEquatorSouth.z());

                ++vCylOffset;
            }

            // Texture coordinates.
            float tTex = cmplFac * vtAspectNorth + fac * vtAspectSouth;
            for (uint32_t j = 0; j < verifLonsp1; ++j) {
                float sTex = sTexCache[j];
                vertexTextures[vtCylOffset] = QVector2D(sTex, tTex);
                ++vtCylOffset;
            }
        }
    }

    // Cylinder face indices.
    uint32_t fCylOffset = idxFsCyl;
    for (uint32_t m = 0; m < verifRingsp1; ++m) {
        uint32_t vCurrRing = idxVNEquator + m * verifLons;
        uint32_t vNextRing = vCurrRing + verifLons;

        uint32_t vtCurrRing = idxVtNEquator + m * verifLonsp1;
        uint32_t vtNextRing = vtCurrRing + verifLonsp1;

        for (uint32_t j = 0; j < verifLons; ++j) {
            uint32_t jNextVt = j + 1;
            uint32_t jNextV = jNextVt % verifLons;

            // Coordinate corners.
            uint32_t v00 = vCurrRing + j;
            uint32_t v01 = vNextRing + j;
            uint32_t v11 = vNextRing + jNextV;
            uint32_t v10 = vCurrRing + jNextV;

            // Texture coordinate corners.
            uint32_t vt00 = vtCurrRing + j;
            uint32_t vt01 = vtNextRing + j;
            uint32_t vt11 = vtNextRing + jNextVt;
            uint32_t vt10 = vtCurrRing + jNextVt;

            // Normal corners.
            uint32_t vn0 = idxVNEquator + j;
            uint32_t vn1 = idxVNEquator + jNextV;

            faces[fCylOffset] = { Face { v00, vt00, vn0 }, Face { v11, vt11, vn1 },
                                  Face { v10, vt10, vn1 } };

            faces[fCylOffset + 1] = { Face { v00, vt00, vn0 }, Face { v01, vt01, vn0 },
                                      Face { v11, vt11, vn1 } };

            fCylOffset += 2;
        }
    }

    uint32_t stride = 3 * sizeof(float);
    uint32_t strideNormal = 0;
    uint32_t strideUV = 0;

    if (m_enableNormals) {
        strideNormal = stride;
        stride += 3 * sizeof(float);
    }
    if (m_enableUV) {
        strideUV = stride;
        stride += 2 * sizeof(float);
    }

    QByteArray vertexData(vertices.length() * stride, Qt::Initialization::Uninitialized);
    QByteArray indexData(faces.length() * 3 * sizeof(quint32), Qt::Initialization::Uninitialized);

    const auto getVertexPtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector3D *>(vertexData.data() + stride * vertexIdx);
    };
    const auto getNormalPtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector3D *>(vertexData.data() + stride * vertexIdx + strideNormal);
    };
    const auto getTexturePtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector2D *>(vertexData.data() + stride * vertexIdx + strideUV);
    };

    uint32_t *indexPtr = reinterpret_cast<uint32_t *>(indexData.data());

    for (qsizetype i = 0; i < vertices.length(); i++) {
        *getVertexPtr(i) = vertices[i];
    }

    for (qsizetype i = 0; i < faces.length(); i++) {
        const auto vertexIndices =
                std::array<uint32_t, 3> { faces[i][0].vertexIdx, faces[i][1].vertexIdx,
                                          faces[i][2].vertexIdx };
        *indexPtr = vertexIndices[0];
        indexPtr++;
        *indexPtr = vertexIndices[1];
        indexPtr++;
        *indexPtr = vertexIndices[2];
        indexPtr++;

        if (m_enableNormals) {
            const auto normalIndices =
                    std::array<uint32_t, 3> { faces[i][0].normalIdx, faces[i][1].normalIdx,
                                              faces[i][2].normalIdx };
            *getNormalPtr(vertexIndices[0]) = vertexNormals[normalIndices[0]];
            *getNormalPtr(vertexIndices[1]) = vertexNormals[normalIndices[1]];
            *getNormalPtr(vertexIndices[2]) = vertexNormals[normalIndices[2]];
        }

        if (m_enableUV) {
            const auto textureIndices =
                    std::array<uint32_t, 3> { faces[i][0].textureIdx, faces[i][1].textureIdx,
                                              faces[i][2].textureIdx };
            *getTexturePtr(vertexIndices[0]) = vertexTextures[textureIndices[0]];
            *getTexturePtr(vertexIndices[1]) = vertexTextures[textureIndices[1]];
            *getTexturePtr(vertexIndices[2]) = vertexTextures[textureIndices[2]];
        }
    }

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    if (m_enableNormals) {
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, strideNormal,
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
    }
    if (m_enableUV) {
        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, strideUV,
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
    }
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::U32Type);

    setStride(stride);
    setVertexData(vertexData);
    setIndexData(indexData);

    setBounds(QVector3D(-radius - 0.5f * m_height, -radius, -radius),
              QVector3D(radius + 0.5f * m_height, radius, radius));
}

QT_END_NAMESPACE
