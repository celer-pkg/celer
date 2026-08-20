// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include "qssgrendercamera_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtGui/qquaternion.h>
#include <QtGui/QVector2D>

#include <qmath.h>

QT_BEGIN_NAMESPACE

static inline float getAspectRatio(const QRectF &inViewport)
{
    return inViewport.height() != 0 ? inViewport.width() / inViewport.height() : 0.0f;
}

QSSGRenderCamera::QSSGRenderCamera(QSSGRenderGraphObject::Type type)
    : QSSGRenderNode(type)
{
    Q_ASSERT(QSSGRenderCamera::isCamera(type));
    markDirty(DirtyFlag::CameraDirty);
}

bool QSSGRenderCamera::calculateProjectionInternal(QSSGRenderCamera &camera,
                                                   const QRectF &inViewport,
                                                   Configuration config)
{
    return camera.calculateProjection(inViewport, config);
}

bool QSSGRenderCamera::calculateProjection(const QRectF &inViewport, Configuration config)
{
    bool retval = false;

    // NOTE: We always update the dpr, under normal surcumstances it won't change, but we have at least
    //       one broken use-cases (front-end camera class) where we depend on the dpr being available from
    //       here...
    dpr = config.dpr;

    const bool argumentsChanged = inViewport != previousInViewport;
    if (!argumentsChanged && !isDirty(DirtyFlag::CameraDirty))
        return true;
    previousInViewport = inViewport;
    clearDirty(DirtyFlag::CameraDirty);

    switch (type) {
    case QSSGRenderGraphObject::Type::OrthographicCamera:
        retval = computeFrustumOrtho(inViewport, clipPlanes, magnification, config, projection);
        break;
    case QSSGRenderGraphObject::Type::PerspectiveCamera:
        retval = computeFrustumPerspective(inViewport, fov, clipPlanes, projection);
        break;
    case QSSGRenderGraphObject::Type::CustomCamera:
        retval = true; // Do nothing
        break;
    case QSSGRenderGraphObject::Type::CustomFrustumCamera:
        retval = computeCustomFrustum(customFrustum, clipPlanes, projection);
        break;
    default:
        Q_UNREACHABLE();
    }

    if (retval) {
        float *writePtr(projection.data());
        frustumScale.setX(writePtr[0]);
        frustumScale.setY(writePtr[5]);
    }
    return retval;
}

//==============================================================================
/**
 *    Compute the projection matrix for a perspective camera
 *    @return true if the computed projection matrix is valid
 */
bool QSSGRenderCamera::computeFrustumPerspective(QRectF inViewport,
                                                 QSSGRenderCamera::FieldOfView fov,
                                                 QSSGRenderCamera::ClipPlanes clipPlane,
                                                 QMatrix4x4 &outProjection)
{
    outProjection.setToIdentity();
    const float aspectRatio = getAspectRatio(inViewport);
    const auto vfov = fov.asVerticalFov(aspectRatio);
    outProjection.perspective(vfov.degrees(), aspectRatio, clipPlane.clipNear(), clipPlane.clipFar());
    return true;
}

bool QSSGRenderCamera::computeCustomFrustum(QSSGRenderCamera::Frustum frustum,
                                            QSSGRenderCamera::ClipPlanes clipPlanes,
                                            QMatrix4x4 &outProjection)
{
    outProjection.setToIdentity();
    outProjection.frustum(frustum.left, frustum.right, frustum.bottom, frustum.top, clipPlanes.clipNear(), clipPlanes.clipFar());
    return true;
}

void QSSGRenderCamera::calculateViewProjectionMatrix(const QMatrix4x4 &globalTransform,
                                                     const QMatrix4x4 &projection,
                                                     QMatrix4x4 &outMatrix)
{
    QMatrix4x4 nonScaledGlobal(Qt::Uninitialized);
    nonScaledGlobal.setColumn(0, globalTransform.column(0).normalized());
    nonScaledGlobal.setColumn(1, globalTransform.column(1).normalized());
    nonScaledGlobal.setColumn(2, globalTransform.column(2).normalized());
    nonScaledGlobal.setColumn(3, globalTransform.column(3));
    outMatrix = projection * nonScaledGlobal.inverted();
}

//==============================================================================
/**
 *    Compute the outProjection matrix for a orthographic camera
 *    @return true if the computed outProjection matrix is valid
 */
bool QSSGRenderCamera::computeFrustumOrtho(QRectF inViewport,
                                           QSSGRenderCamera::ClipPlanes clipPlanes,
                                           QSSGRenderCamera::Magnification magnification,
                                           Configuration config,
                                           QMatrix4x4 &outProjection)
{


    outProjection.setToIdentity();
    // When using ssaa we need to zoom with the ssaa multiplier since otherwise the
    // orthographic camera will be zoomed out due to the bigger viewport. We therefore
    // scale the magnification before calulating the projection.
    // Since the same camera can be used in several View3Ds with or without ssaa we
    // cannot store the magnification permanently.
    magnification *= config.ssaaMultiplier;
    const float halfWidth = inViewport.width() / 2.0f / magnification.horizontal() / config.dpr;
    const float halfHeight = inViewport.height() / 2.0f / magnification.vertical() / config.dpr;
    outProjection.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, clipPlanes.clipNear(), clipPlanes.clipFar());
    return true;
}

static QQuaternion rotationQuaternionForLookAt(const QVector3D &sourcePosition,
                                               const QVector3D &sourceDirection,
                                               const QVector3D &targetPosition,
                                               const QVector3D &upDirection)
{
    QVector3D targetDirection = sourcePosition - targetPosition;
    targetDirection.normalize();

    QVector3D rotationAxis = QVector3D::crossProduct(sourceDirection, targetDirection);

    const QVector3D normalizedAxis = rotationAxis.normalized();
    if (qFuzzyIsNull(normalizedAxis.lengthSquared()))
        rotationAxis = upDirection;

    float dot = QVector3D::dotProduct(sourceDirection, targetDirection);
    dot = qBound(-1.0f, dot, 1.0f);
    float rotationAngle = float(qRadiansToDegrees(qAcos(qreal(dot))));

    return QQuaternion::fromAxisAndAngle(rotationAxis, rotationAngle);
}

void QSSGRenderCamera::lookAt(const QVector3D &inCameraPos, const QVector3D &inUpDir, const QVector3D &inTargetPos, const QVector3D &pivot)
{
    QQuaternion rotation = rotationQuaternionForLookAt(inCameraPos, getScalingCorrectDirection(localTransform), inTargetPos, inUpDir.normalized());
    localTransform = QSSGRenderNode::calculateTransformMatrix(inCameraPos, QSSGRenderNode::initScale, pivot, rotation);
    QSSGRenderNode::markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
}

void QSSGRenderCamera::calculateViewProjectionMatrix(const QMatrix4x4 &globalTransform, QMatrix4x4 &outMatrix) const
{
    QMatrix4x4 nonScaledGlobal(Qt::Uninitialized);
    nonScaledGlobal.setColumn(0, globalTransform.column(0).normalized());
    nonScaledGlobal.setColumn(1, globalTransform.column(1).normalized());
    nonScaledGlobal.setColumn(2, globalTransform.column(2).normalized());
    nonScaledGlobal.setColumn(3, globalTransform.column(3));
    outMatrix = projection * nonScaledGlobal.inverted();
}

void QSSGRenderCamera::calculateViewProjectionWithoutTranslation(const QMatrix4x4 &globalTransform, float clipNear, float clipFar, QMatrix4x4 &outMatrix) const
{
    if (qFuzzyIsNull(clipFar - clipNear)) {
        qWarning() << "QSSGRenderCamera::calculateViewProjection: far == near";
        return;
    }

    QMatrix4x4 proj = projection;
    proj(2, 2) = -(clipFar + clipNear) / (clipFar - clipNear);
    proj(2, 3) = -2 * clipFar * clipNear / (clipFar - clipNear);
    QMatrix4x4 nonScaledGlobal(Qt::Uninitialized);
    nonScaledGlobal.setColumn(0, globalTransform.column(0).normalized());
    nonScaledGlobal.setColumn(1, globalTransform.column(1).normalized());
    nonScaledGlobal.setColumn(2, globalTransform.column(2).normalized());
    nonScaledGlobal.setColumn(3, QVector4D(0, 0, 0, 1));
    outMatrix = proj * nonScaledGlobal.inverted();
}

QSSGRenderRay QSSGRenderCamera::unproject(const QMatrix4x4 &globalTransform,
                                          const QVector2D &inViewportRelativeCoords,
                                          const QRectF &inViewport) const
{
    QSSGRenderRay theRay;
    QVector2D normalizedCoords = QSSGUtils::rect::relativeToNormalizedCoordinates(inViewport, inViewportRelativeCoords);
    QVector3D &outOrigin(theRay.origin);
    QVector3D &outDir(theRay.direction);
    QVector2D inverseFrustumScale(1.0f / frustumScale.x(), 1.0f / frustumScale.y());
    QVector2D scaledCoords(inverseFrustumScale.x() * normalizedCoords.x(), inverseFrustumScale.y() * normalizedCoords.y());

    if (type == QSSGRenderCamera::Type::OrthographicCamera) {
        outOrigin.setX(scaledCoords.x());
        outOrigin.setY(scaledCoords.y());
        outOrigin.setZ(0.0f);

        outDir.setX(0.0f);
        outDir.setY(0.0f);
        outDir.setZ(-1.0f);
    } else {
        outOrigin.setX(0.0f);
        outOrigin.setY(0.0f);
        outOrigin.setZ(0.0f);

        outDir.setX(scaledCoords.x());
        outDir.setY(scaledCoords.y());
        outDir.setZ(-1.0f);
    }

    outOrigin = QSSGUtils::mat44::transform(globalTransform, outOrigin);
    QMatrix3x3 theNormalMatrix = globalTransform.normalMatrix();

    outDir = QSSGUtils::mat33::transform(theNormalMatrix, outDir);
    outDir.normalize();

    return theRay;
}

void QSSGRenderCamera::markDirty(DirtyFlag dirtyFlag)
{
    cameraDirtyFlags |= FlagT(dirtyFlag);
    QSSGRenderNode::markDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

void QSSGRenderCamera::clearDirty(DirtyFlag dirtyFlag)
{
    cameraDirtyFlags &= ~FlagT(dirtyFlag);
    QSSGRenderNode::clearDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

static float getZNear(const QMatrix4x4 &projection)
{
    const float *data = projection.constData();
    QSSGPlane plane(QVector3D(data[3] + data[2], data[7] + data[6], data[11] + data[10]), -data[15] - data[14]);
    plane.normalize();
    return plane.d;
}

static QVector2D getViewportHalfExtents(const QMatrix4x4 &projection) {
    const float *data = projection.constData();

    QSSGPlane nearPlane(QVector3D(data[3] + data[2], data[7] + data[6], data[11] + data[10]), -data[15] - data[14]);
    nearPlane.normalize();
    QSSGPlane rightPlane(QVector3D(data[3] - data[0], data[7] - data[4], data[11] - data[8]), -data[15] + data[12]);
    rightPlane.normalize();
    QSSGPlane topPlane(QVector3D(data[3] - data[1], data[7] - data[5], data[11] - data[9]), -data[15] + data[13]);
    topPlane.normalize();

    // Get intersection the 3 planes
    float denom = QVector3D::dotProduct(QVector3D::crossProduct(nearPlane.n, rightPlane.n), topPlane.n);
    if (qFuzzyIsNull(denom))
        return QVector2D();

    QVector3D intersection = (QVector3D::crossProduct(rightPlane.n, topPlane.n) * nearPlane.d +
                             (QVector3D::crossProduct(topPlane.n, nearPlane.n) * rightPlane.d) +
                             (QVector3D::crossProduct(nearPlane.n, rightPlane.n) * topPlane.d)) / denom;

    return QVector2D(intersection.x(), intersection.y());
}

float QSSGRenderCamera::getLevelOfDetailMultiplier() const
{
    if (type == QSSGRenderGraphObject::Type::OrthographicCamera)
        return getViewportHalfExtents(projection).x();

    float zn = getZNear(projection);
    float width = getViewportHalfExtents(projection).x() * 2.0;
    return 1.0 / (zn / width);

}

QT_END_NAMESPACE
