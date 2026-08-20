// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_CAMERA_H
#define QSSG_RENDER_CAMERA_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderray_p.h>

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

QT_BEGIN_NAMESPACE

struct QSSGCameraGlobalCalculationResult
{
    bool m_wasDirty;
    bool m_computeFrustumSucceeded /* = true */;
};

struct QSSGRenderCameraConfiguration
{
    float dpr = 1.0f;
    float ssaaMultiplier = 1.0f;
};

class QSSGRenderCameraFieldOfView
{
public:
    enum class Orientation
    {
        Vertical,
        Horizontal,
    };

    explicit constexpr QSSGRenderCameraFieldOfView(Orientation orientation = Orientation::Vertical) : fovOrientation(orientation) {}
    QSSGRenderCameraFieldOfView(const QSSGRenderCameraFieldOfView &) = default;
    QSSGRenderCameraFieldOfView(QSSGRenderCameraFieldOfView &&) = default;

    QSSGRenderCameraFieldOfView &operator=(const QSSGRenderCameraFieldOfView &other) = default;

    template <Orientation Type = Orientation::Vertical>
    static QSSGRenderCameraFieldOfView fromDegrees(float fov)
    {
        return QSSGRenderCameraFieldOfView(fov, FovValue::Degrees, Type);
    }
    template <Orientation Type = Orientation::Vertical>
    static QSSGRenderCameraFieldOfView fromRadians(float fov)
    {
        return QSSGRenderCameraFieldOfView(fov, FovValue::Radians, Type);
    }

    constexpr Orientation orientation() const { return fovOrientation; }

    float degrees() const { return fovDegrees; }
    float radians() const { return fovRadians; }

    void setDegrees(float fov)
    {
        fovDegrees = fov;
        fovRadians = qDegreesToRadians(fov);
    }

    QSSGRenderCameraFieldOfView asVerticalFov(float aspectRatio) const
    {
        if (fovOrientation == Orientation::Vertical)
            return *this;

        const float fovV = float(2.0 * qAtan(qTan(qreal(fovRadians) / 2.0) / qreal(aspectRatio)));
        return fromRadians<Orientation::Vertical>(fovV);
    }

    void setRadians(float fov)
    {
        fovRadians = fov;
        fovDegrees = qRadiansToDegrees(fov);
    }
private:
    enum class FovValue
    {
        Degrees,
        Radians,
    };
    constexpr explicit QSSGRenderCameraFieldOfView(float fov, FovValue value, Orientation angleOfView)
        : fovDegrees(value == FovValue::Degrees ? fov : qRadiansToDegrees(fov))
        , fovRadians(value == FovValue::Radians ? fov : qDegreesToRadians(fov))
        , fovOrientation(angleOfView)
    {
    }

    float fovDegrees = 60.0f;
    float fovRadians = qDegreesToRadians(fovDegrees);
    Orientation fovOrientation;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderCamera : public QSSGRenderNode
{
public:
    using Configuration = QSSGRenderCameraConfiguration;
    using FieldOfView = QSSGRenderCameraFieldOfView;

    explicit QSSGRenderCamera(QSSGRenderGraphObject::Type type);

    enum class DirtyFlag : quint8
    {
        CameraDirty = 0x1,
        LayerMaskDirty = 0x2,
    };
    using FlagT = std::underlying_type_t<DirtyFlag>;

    static constexpr DirtyFlag DirtyMask { std::numeric_limits<FlagT>::max() };

    static constexpr quint32 ReservedLayerMask = 0xFF000000; // The upper 8 bits are reserved for internal use.
    static constexpr quint32 LayerMaskUserAll = 0x00FFFFFF; // The lower 24 bits are available for user layers.
    // Default to all layers (user + reserved).
    static constexpr quint32 LayerMaskAll = LayerMaskUserAll | ReservedLayerMask;

    class ClipPlanes : public QVector2D
    {
    public:
        constexpr inline ClipPlanes() : QVector2D(0.0f, 10000.0f) {}
        constexpr inline ClipPlanes(float n, float f) : QVector2D(n, f) {}
        constexpr float clipNear() const noexcept { return x(); }
        constexpr float clipFar() const noexcept { return y(); }

        constexpr inline void setClipNear(float n) noexcept { setX(n); }
        constexpr inline void setClipFar(float f) noexcept { setY(f); }
    };

    class Magnification : public QVector2D
    {
    public:
        constexpr inline Magnification() : QVector2D(1.0f, 1.0f) {}
        constexpr inline Magnification(float h, float v) : QVector2D(h, v) {}
        constexpr float horizontal() const noexcept { return x(); }
        constexpr float vertical() const noexcept { return y(); }

        constexpr inline void setHorizontal(float h) noexcept { setX(h); }
        constexpr inline void setVertical(float v) noexcept { setY(v); }
    };

    // Used by the custom frustum camera
    struct Frustum
    {
        float top = 0.0f;
        float bottom = 0.0f;
        float left = 0.0f;
        float right = 0.0f;
    };

    // Set our position, rotation member variables based on the lookat target
    // Marks this object as dirty.
    // Need to test this when the camera's local transform is null.
    // Assumes parent's local transform is the identity, meaning our local transform is
    // our global transform.
    void lookAt(const QVector3D &inCameraPos, const QVector3D &inUpDir, const QVector3D &inTargetPos, const QVector3D &pivot);

    /*!
     * Convenience function intended for use with internal cameras that's not part of the scene graph!
     * This function will NOT ensure that the camera's global transform, projection matrix, or state
     * is updated!.
     */
    static bool calculateProjectionInternal(QSSGRenderCamera &camera,
                                            const QRectF &inViewport,
                                            Configuration config = {});

    /*!
     * \brief calculateProjection
     * \param inViewport
     * \param config
     * \return true if the projection was successfully calculated, false otherwise.
     *
     * This function will calculate the projection matrix for the camera.
     *
     * NOTE: This function will not update the camera's global transform. It is the caller's
     * responsibility to ensure that the camera's global transform has been calculated prior
     * to calling this function.
     */
    bool calculateProjection(const QRectF &inViewport, Configuration config = {});

    void setCustomProjection(const QMatrix4x4 &proj);

    static bool computeFrustumOrtho(QRectF inViewport,
                                    QSSGRenderCamera::ClipPlanes clipPlanes,
                                    QSSGRenderCamera::Magnification magnification,
                                    Configuration config,
                                    QMatrix4x4 &outProjection);
    static bool computeFrustumPerspective(QRectF inViewport,
                                          FieldOfView fov,
                                          QSSGRenderCamera::ClipPlanes clipPlanes,
                                          QMatrix4x4 &outProjection);
    static bool computeCustomFrustum(QSSGRenderCamera::Frustum frustum,
                                     QSSGRenderCamera::ClipPlanes clipPlanes,
                                     QMatrix4x4 &outProjection);

    static void calculateViewProjectionMatrix(const QMatrix4x4 &globalTransform,
                                              const QMatrix4x4 &projection,
                                              QMatrix4x4 &outMatrix);

    void calculateViewProjectionMatrix(const QMatrix4x4 &globalTransform, QMatrix4x4 &outMatrix) const;
    void calculateViewProjectionWithoutTranslation(const QMatrix4x4 &globalTransform, float near, float far, QMatrix4x4 &outMatrix) const;

    // Unproject a point (x,y) in viewport relative coordinates meaning
    // left, bottom is 0,0 and values are increasing right,up respectively.
    QSSGRenderRay unproject(const QMatrix4x4 &globalTransform, const QVector2D &inLayerRelativeMouseCoords, const QRectF &inViewport) const;

    [[nodiscard]] inline bool isDirty(DirtyFlag dirtyFlag = DirtyMask) const
    {
        return ((cameraDirtyFlags & FlagT(dirtyFlag)) != 0)
               || ((dirtyFlag == DirtyMask) && QSSGRenderNode::isDirty());
    }
    void markDirty(DirtyFlag dirtyFlag);
    void clearDirty(DirtyFlag dirtyFlag);

    float getLevelOfDetailMultiplier() const;

    float getDpr() const { return dpr; }

    // Setting these variables should be followed by marking the camera dirty!
    ClipPlanes clipPlanes;
    FieldOfView fov;
    Frustum customFrustum;
    Magnification magnification;
    QMatrix4x4 projection;
    float levelOfDetailPixelThreshold = 1.0;
    quint32 layerMask = LayerMaskAll;
    bool enableFrustumClipping = true;

private:
    // Record some values from creating the projection matrix
    // to use during mouse picking.
    QVector2D frustumScale;

    QRectF previousInViewport;
    float dpr = 1.0f;
    FlagT cameraDirtyFlags = 0;
};

[[nodiscard]] constexpr bool qFuzzyCompare(const QSSGRenderCamera::FieldOfView &fov1, const QSSGRenderCamera::FieldOfView &fov2) noexcept
{
    return fov1.orientation() == fov2.orientation() && qFuzzyCompare(fov1.degrees(), fov2.degrees());
}

[[nodiscard]] constexpr bool qFuzzyCompare(QSSGRenderCamera::Frustum f1, QSSGRenderCamera::Frustum f2) noexcept
{
    return qFuzzyCompare(f1.top, f2.top) && qFuzzyCompare(f1.bottom, f2.bottom) && qFuzzyCompare(f1.left, f2.left) && qFuzzyCompare(f1.right, f2.right);
}

QT_END_NAMESPACE

#endif
