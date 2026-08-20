// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


// Based on:
// https://behreajj.medium.com/making-a-capsule-mesh-via-script-in-five-3d-environments-c2214abf02db

#include "capsulegeometry_p.h"

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif
#include <QQuaternion>

#include <cmath>

QT_BEGIN_NAMESPACE

/*!
    \qmltype CapsuleGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.10
    \brief Provides geometry for a capsule.

    A geometry for generating a capsule model. The capsule is centered
    at \c{(0, 0, 0)} with the height of the capsule extending in the x
    direction and the diameter on the yz plane.
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

    Number of latitudes, distributed by inclination. Will always be snapped to an even number.
*/

/*! \qmlproperty int CapsuleGeometry::rings
    \default 1

    Number of sections in cylinder between hemispheres.
*/

/*! \qmlproperty real CapsuleGeometry::height
    \default 100

    Height of the middle cylinder on the x axis, excluding the hemispheres.
*/

/*! \qmlproperty real CapsuleGeometry::diameter
    \default 100

    Diameter on the yz plane.
*/

/*!
    \qmlproperty UVProfile CapsuleGeometry::uvProfile
    \default CapsuleGeometry.Fixed

    Manner in which UV coordinates are distributed along the length of the capsule.

    \value CapsuleGeometry.Fixed The upper third of the UV texture is the North
    hemisphere, the middle third is the cylinder and the last third is the
    South hemisphere.
    \value CapsuleGeometry.Aspect UVs match the height to radius ratio.
    \value CapsuleGeometry.Uniform Uniform proportion for all UV cells,
    according to the ratio of latitudes to rings.
*/

/*!
    \qmlproperty bool CapsuleGeometry::asynchronous
    \default true

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool CapsuleGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value CapsuleGeometry.Null The geometry generation has not started
    \value CapsuleGeometry.Ready The geometry generation is complete.
    \value CapsuleGeometry.Loading The geometry generation is in progress.
    \value CapsuleGeometry.Error The geometry generation failed.
*/

CapsuleGeometry::CapsuleGeometry(QQuick3DObject *parent) : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &CapsuleGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

CapsuleGeometry::~CapsuleGeometry() = default;

void CapsuleGeometry::setEnableNormals(bool enable)
{
    if (m_enableNormals == enable)
        return;

    m_enableNormals = enable;
    emit enableNormalsChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setEnableUV(bool enable)
{
    if (m_enableUV == enable)
        return;

    m_enableUV = enable;
    emit enableUVChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setLongitudes(int longitudes)
{
    if (m_longitudes == longitudes)
        return;

    m_longitudes = longitudes;
    emit longitudesChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setLatitudes(int latitudes)
{
    if (m_latitudes == latitudes)
        return;

    m_latitudes = latitudes;
    emit latitudesChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setRings(int rings)
{
    if (m_rings == rings)
        return;

    m_rings = rings;
    emit ringsChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setHeight(float height)
{
    if (m_height == height)
        return;

    m_height = height;
    emit heightChanged();
    scheduleGeometryUpdate();
}

void CapsuleGeometry::setDiameter(float diameter)
{
    if (m_diameter == diameter)
        return;

    m_diameter = diameter;
    emit diameterChanged();
    scheduleGeometryUpdate();
}

CapsuleGeometry::UVProfile CapsuleGeometry::uvProfile() const
{
    return m_uvProfile;
}

void CapsuleGeometry::setUVProfile(UVProfile newUVProfile)
{
    if (m_uvProfile == newUVProfile)
        return;
    m_uvProfile = newUVProfile;
    emit uvProfileChanged();
    scheduleGeometryUpdate();
}

bool CapsuleGeometry::asynchronous() const
{
    return m_asynchronous;
}

void CapsuleGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

CapsuleGeometry::Status CapsuleGeometry::status() const
{
    return m_status;
}

void CapsuleGeometry::doUpdateGeometry()
{
    // reset the flag since we are processing the update
    m_geometryUpdateRequested = false;

#if QT_CONFIG(concurrent)
    if (m_geometryDataFuture.isRunning()) {
        m_pendingAsyncUpdate = true;
        return;
    }
#endif

#if QT_CONFIG(concurrent)
    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generateCapsuleGeometryAsync,
                                                 m_enableNormals,
                                                 m_enableUV,
                                                 m_longitudes,
                                                 m_latitudes,
                                                 m_rings,
                                                 m_height,
                                                 m_diameter,
                                                 m_uvProfile);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {
#endif // QT_CONFIG(concurrent)
        updateGeometry(generateCapsuleGeometry(m_enableNormals, m_enableUV, m_longitudes, m_latitudes, m_rings, m_height, m_diameter, m_uvProfile));
    }
}

void CapsuleGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void CapsuleGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void CapsuleGeometry::updateGeometry(const GeometryData &geometryData)
{
    clear();
    setStride(geometryData.stride);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::ComponentType::F32Type);
    if (geometryData.enableNormals) {
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                     geometryData.strideNormal,
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
    }
    if (geometryData.enableUV) {
        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, geometryData.strideUV, QQuick3DGeometry::Attribute::ComponentType::F32Type);
    }
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, QQuick3DGeometry::Attribute::ComponentType::U32Type);

    setBounds(geometryData.boundsMin, geometryData.boundsMax);
    setVertexData(geometryData.vertexData);
    setIndexData(geometryData.indexData);

    // If the geometry update was requested while the geometry was being generated asynchronously,
    // we need to schedule another geometry update now that the geometry is ready.
    if (m_pendingAsyncUpdate) {
        m_pendingAsyncUpdate = false;
        scheduleGeometryUpdate();
    } else {
        m_status = Status::Ready;
        Q_EMIT statusChanged();
    }
    update();
    emit geometryChanged();
}

CapsuleGeometry::GeometryData CapsuleGeometry::generateCapsuleGeometry(bool enableNormals,
                                                                       bool enableUV,
                                                                       int longitudes,
                                                                       int latitudes,
                                                                       int rings,
                                                                       float height,
                                                                       float diameter,
                                                                       UVProfile uvProfile)
{
    longitudes = qMax(3, longitudes);
    latitudes = qMax(2, latitudes + (latitudes % 2)); // make even
    rings = qMax(1, rings);
    height = qMax(0.00001f, height);
    diameter = qMax(0.00001f, diameter);

    const UVProfile profile = uvProfile;
    const float radius = diameter / 2;
    const float depth = height;

    bool calcMiddle = rings > 0;
    int halfLats = latitudes / 2;
    int halfLatsn1 = halfLats - 1;
    int halfLatsn2 = halfLats - 2;
    int ringsp1 = rings + 1;
    int lonsp1 = longitudes + 1;
    float halfDepth = depth * 0.5f;
    float summit = halfDepth + radius;

    // Vertex index offsets.
    int vertOffsetNorthHemi = longitudes;
    int vertOffsetNorthEquator = vertOffsetNorthHemi + lonsp1 * halfLatsn1;
    int vertOffsetCylinder = vertOffsetNorthEquator + lonsp1;
    int vertOffsetSouthEquator = calcMiddle ?
            vertOffsetCylinder + lonsp1 * rings :
            vertOffsetCylinder;
    int vertOffsetSouthHemi = vertOffsetSouthEquator + lonsp1;
    int vertOffsetSouthPolar = vertOffsetSouthHemi + lonsp1 * halfLatsn2;
    int vertOffsetSouthCap = vertOffsetSouthPolar + lonsp1;

    // Initialize arrays.
    int vertLen = vertOffsetSouthCap + longitudes;
    QList<QVector3D> vs = QList<QVector3D>(vertLen);
    QList<QVector2D> vts = QList<QVector2D>(vertLen);
    QList<QVector3D> vns = QList<QVector3D>(vertLen);

    float toTheta = 2.0f * M_PI / longitudes;
    float toPhi = M_PI / latitudes;
    float toTexHorizontal = 1.0f / longitudes;
    float toTexVertical = 1.0f / halfLats;

    // Calculate positions for texture coordinates vertical.
    float vtAspectRatio = 1.0f;
    switch (profile)
    {
    case UVProfile::Aspect:
        vtAspectRatio = radius / (depth + radius + radius);
        break;

    case UVProfile::Uniform:
        vtAspectRatio = (float) halfLats / (ringsp1 + latitudes);
        break;

    case UVProfile::Fixed:
    default:
        vtAspectRatio = 1.0f / 3.0f;
        break;
    }

    float vtAspectNorth = 1.0f - vtAspectRatio;
    float vtAspectSouth = vtAspectRatio;

    QList<QVector2D> thetaCartesian = QList<QVector2D>(longitudes);
    QList<QVector2D> rhoThetaCartesian = QList<QVector2D>(longitudes);
    QList<float> sTextureCache = QList<float>(lonsp1);

    // Polar vertices.
    for (int j = 0; j < longitudes; ++j)
    {
        float jf = j;
        float sTexturePolar = 1.0f - ((jf + 0.5f) * toTexHorizontal);
        float theta = jf * toTheta;

        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        thetaCartesian[j] = QVector2D(cosTheta, sinTheta);
        rhoThetaCartesian[j] = QVector2D(
                radius * cosTheta,
                radius * sinTheta);

        // North.
        vs[j] = QVector3D(0.0f, summit, 0.0f);
        vts[j] = QVector2D(sTexturePolar, 1.0f);
        vns[j] = QVector3D(0.0f, 1.0f, 0.0f);

        // South.
        int idx = vertOffsetSouthCap + j;
        vs[idx] = QVector3D(0.0f, -summit, 0.0f);
        vts[idx] = QVector2D(sTexturePolar, 0.0f);
        vns[idx] = QVector3D(0.0f, -1.0f, 0.0f);
    }

    // Equatorial vertices.
    for (int j = 0; j < lonsp1; ++j)
    {
        float sTexture = 1.0f - j * toTexHorizontal;
        sTextureCache[j] = sTexture;

        // Wrap to first element upon reaching last.
        int jMod = j % longitudes;
        QVector2D tc = thetaCartesian[jMod];
        QVector2D rtc = rhoThetaCartesian[jMod];

        // North equator.
        int idxn = vertOffsetNorthEquator + j;
        vs[idxn] = QVector3D(rtc.x(), halfDepth, -rtc.y());
        vts[idxn] = QVector2D(sTexture, vtAspectNorth);
        vns[idxn] = QVector3D(tc.x(), 0.0f, -tc.y());

        // South equator.
        int idxs = vertOffsetSouthEquator + j;
        vs[idxs] = QVector3D(rtc.x(), -halfDepth, -rtc.y());
        vts[idxs] = QVector2D(sTexture, vtAspectSouth);
        vns[idxs] = QVector3D(tc.x(), 0.0f, -tc.y());
    }

    // Hemisphere vertices.
    for (int i = 0; i < halfLatsn1; ++i)
    {
        float ip1f = i + 1.0f;
        float phi = ip1f * toPhi;

        // For coordinates.
        float cosPhiSouth = std::cos(phi);
        float sinPhiSouth = std::sin(phi);

        // Symmetrical hemispheres mean cosine and sine only needs
        // to be calculated once.
        float cosPhiNorth = sinPhiSouth;
        float sinPhiNorth = -cosPhiSouth;

        float rhoCosPhiNorth = radius * cosPhiNorth;
        float rhoSinPhiNorth = radius * sinPhiNorth;
        float zOffsetNorth = halfDepth - rhoSinPhiNorth;

        float rhoCosPhiSouth = radius * cosPhiSouth;
        float rhoSinPhiSouth = radius * sinPhiSouth;
        float zOffsetSouth = -halfDepth - rhoSinPhiSouth;

        // For texture coordinates.
        float tTexFac = ip1f * toTexVertical;
        float cmplTexFac = 1.0f - tTexFac;
        float tTexNorth = cmplTexFac + vtAspectNorth * tTexFac;
        float tTexSouth = cmplTexFac * vtAspectSouth;

        int iLonsp1 = i * lonsp1;
        int vertCurrLatNorth = vertOffsetNorthHemi + iLonsp1;
        int vertCurrLatSouth = vertOffsetSouthHemi + iLonsp1;

        for (int j = 0; j < lonsp1; ++j)
        {
            int jMod = j % longitudes;

            float sTexture = sTextureCache[j];
            QVector2D tc = thetaCartesian[jMod];

            // North hemisphere.
            int idxn = vertCurrLatNorth + j;
            vs[idxn] = QVector3D(
                    rhoCosPhiNorth * tc.x(),
                    zOffsetNorth,
                    -rhoCosPhiNorth * tc.y());
            vts[idxn] = QVector2D(sTexture, tTexNorth);
            vns[idxn] = QVector3D(
                    cosPhiNorth * tc.x(),
                    -sinPhiNorth,
                    -cosPhiNorth * tc.y());

            // South hemisphere.
            int idxs = vertCurrLatSouth + j;
            vs[idxs] = QVector3D(
                    rhoCosPhiSouth * tc.x(),
                    zOffsetSouth,
                    -rhoCosPhiSouth * tc.y());
            vts[idxs] = QVector2D(sTexture, tTexSouth);
            vns[idxs] = QVector3D(
                    cosPhiSouth * tc.x(),
                    -sinPhiSouth,
                    -cosPhiSouth * tc.y());
        }
    }

    // Cylinder vertices.
    if (calcMiddle)
    {
        // Exclude both origin and destination edges
        // (North and South equators) from the interpolation.
        float toFac = 1.0f / ringsp1;
        int idxCylLat = vertOffsetCylinder;

        for (int h = 1; h < ringsp1; ++h)
        {
            float fac = h * toFac;
            float cmplFac = 1.0f - fac;
            float tTexture = cmplFac * vtAspectNorth + fac * vtAspectSouth;
            float z = halfDepth - depth * fac;

            for (int j = 0; j < lonsp1; ++j)
            {
                int jMod = j % longitudes;
                QVector2D tc = thetaCartesian[jMod];
                QVector2D rtc = rhoThetaCartesian[jMod];
                float sTexture = sTextureCache[j];

                vs[idxCylLat] = QVector3D(rtc.x(), z, -rtc.y());
                vts[idxCylLat] = QVector2D(sTexture, tTexture);
                vns[idxCylLat] = QVector3D(tc.x(), 0.0f, -tc.y());

                ++idxCylLat;
            }
        }
    }

    // Triangle indices.
    // Stride is 3 for polar triangles;
    // stride is 6 for two triangles forming a quad.
    int lons3 = longitudes * 3;
    int lons6 = longitudes * 6;
    int hemiLons = halfLatsn1 * lons6;

    int triOffsetNorthHemi = lons3;
    int triOffsetCylinder = triOffsetNorthHemi + hemiLons;
    int triOffsetSouthHemi = triOffsetCylinder + ringsp1 * lons6;
    int triOffsetSouthCap = triOffsetSouthHemi + hemiLons;

    int fsLen = triOffsetSouthCap + lons3;
    QList<int> tris = QList<int>(fsLen);

    // Polar caps.
    for (int i = 0, k = 0, m = triOffsetSouthCap; i < longitudes; ++i, k += 3, m += 3)
    {
        // North.
        tris[k] = i;
        tris[k + 1] = vertOffsetNorthHemi + i;
        tris[k + 2] = vertOffsetNorthHemi + i + 1;

        // South.
        tris[m] = vertOffsetSouthCap + i;
        tris[m + 1] = vertOffsetSouthPolar + i + 1;
        tris[m + 2] = vertOffsetSouthPolar + i;
    }

    // Hemispheres.
    for (int i = 0, k = triOffsetNorthHemi, m = triOffsetSouthHemi; i < halfLatsn1; ++i)
    {
        int iLonsp1 = i * lonsp1;

        int vertCurrLatNorth = vertOffsetNorthHemi + iLonsp1;
        int vertNextLatNorth = vertCurrLatNorth + lonsp1;

        int vertCurrLatSouth = vertOffsetSouthEquator + iLonsp1;
        int vertNextLatSouth = vertCurrLatSouth + lonsp1;

        for (int j = 0; j < longitudes; ++j, k += 6, m += 6)
        {
            // North.
            int north00 = vertCurrLatNorth + j;
            int north01 = vertNextLatNorth + j;
            int north11 = vertNextLatNorth + j + 1;
            int north10 = vertCurrLatNorth + j + 1;

            tris[k] = north00;
            tris[k + 1] = north11;
            tris[k + 2] = north10;

            tris[k + 3] = north00;
            tris[k + 4] = north01;
            tris[k + 5] = north11;

            // South.
            int south00 = vertCurrLatSouth + j;
            int south01 = vertNextLatSouth + j;
            int south11 = vertNextLatSouth + j + 1;
            int south10 = vertCurrLatSouth + j + 1;

            tris[m] = south00;
            tris[m + 1] = south11;
            tris[m + 2] = south10;

            tris[m + 3] = south00;
            tris[m + 4] = south01;
            tris[m + 5] = south11;
        }
    }

    // Cylinder.
    for (int i = 0, k = triOffsetCylinder; i < ringsp1; ++i)
    {
        int vertCurrLat = vertOffsetNorthEquator + i * lonsp1;
        int vertNextLat = vertCurrLat + lonsp1;

        for (int j = 0; j < longitudes; ++j, k += 6)
        {
            int cy00 = vertCurrLat + j;
            int cy01 = vertNextLat + j;
            int cy11 = vertNextLat + j + 1;
            int cy10 = vertCurrLat + j + 1;

            tris[k] = cy00;
            tris[k + 1] = cy11;
            tris[k + 2] = cy10;

            tris[k + 3] = cy00;
            tris[k + 4] = cy01;
            tris[k + 5] = cy11;
        }
    }

    auto vertices = vs;
    auto vertexTextures = vts;
    auto vertexNormals = vns;

    uint32_t stride = 3 * sizeof(float);
    uint32_t strideNormal = 0;
    uint32_t strideUV = 0;

    if (enableNormals) {
        strideNormal = stride;
        stride += 3 * sizeof(float);
    }
    if (enableUV) {
        strideUV = stride;
        stride += 2 * sizeof(float);
    }

    QByteArray vertexData(vertices.length() * stride, Qt::Initialization::Uninitialized);
    QByteArray indexData(tris.length() * sizeof(int), Qt::Initialization::Uninitialized);

    const auto getVertexPtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector3D *>(vertexData.data() + stride * vertexIdx);
    };
    const auto getNormalPtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector3D *>(vertexData.data() + stride * vertexIdx + strideNormal);
    };
    const auto getTexturePtr = [&](const int vertexIdx) {
        return reinterpret_cast<QVector2D *>(vertexData.data() + stride * vertexIdx + strideUV);
    };

    const QQuaternion rotateZ = QQuaternion::fromEulerAngles(0, 0, 90);
    uint32_t *indexPtr = reinterpret_cast<uint32_t *>(indexData.data());

    for (qsizetype i = 0; i < vertices.length(); i++) {
        *getVertexPtr(i) = rotateZ * vertices[i];
    }

    for (qsizetype i = 0; i < tris.length() / 3; i++) {
        std::array<int, 3> triIndices = { tris[i * 3], tris[i * 3 + 1], tris[i * 3 + 2] };
        *indexPtr = triIndices[0];
        indexPtr++;
        *indexPtr = triIndices[1];
        indexPtr++;
        *indexPtr = triIndices[2];
        indexPtr++;

        if (enableNormals) {
            *getNormalPtr(triIndices[0]) = rotateZ * vertexNormals[triIndices[0]];
            *getNormalPtr(triIndices[1]) = rotateZ * vertexNormals[triIndices[1]];
            *getNormalPtr(triIndices[2]) = rotateZ * vertexNormals[triIndices[2]];
        }

        if (enableUV) {
            *getTexturePtr(triIndices[0]) = vertexTextures[triIndices[0]];
            *getTexturePtr(triIndices[1]) = vertexTextures[triIndices[1]];
            *getTexturePtr(triIndices[2]) = vertexTextures[triIndices[2]];
        }
    }

    GeometryData geometryData;
    geometryData.indexData = indexData;
    geometryData.vertexData = vertexData;
    geometryData.boundsMin = QVector3D(-radius - halfDepth, -radius, -radius);
    geometryData.boundsMax = QVector3D(radius + halfDepth, radius, radius);
    geometryData.stride = stride;
    geometryData.strideNormal = strideNormal;
    geometryData.strideUV = strideUV;
    geometryData.enableNormals = enableNormals;
    geometryData.enableUV = enableUV;
    return geometryData;
}

#if QT_CONFIG(concurrent)
void CapsuleGeometry::generateCapsuleGeometryAsync(QPromise<CapsuleGeometry::GeometryData> &promise,
                                                   bool enableNormals,
                                                   bool enableUV,
                                                   int longitudes,
                                                   int latitudes,
                                                   int rings,
                                                   float height,
                                                   float diameter,
                                                   CapsuleGeometry::UVProfile uvProfile)
{
    auto output = generateCapsuleGeometry(enableNormals, enableUV, longitudes, latitudes, rings, height, diameter, uvProfile);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
