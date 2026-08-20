// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dsceneenvironment_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dtexture_p.h"
#include "qquick3dcubemaptexture_p.h"
#include "qquick3ddebugsettings_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype SceneEnvironment
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Lets you configure how a scene is rendered.

    SceneEnvironment defines a set of global properties for how a scene should be rendered.

    \note The QtQuick3D.Helpers module offers an \l ExtendedSceneEnvironment
    type which inherits from SceneEnvironment and adds a number of built-in
    effects on top.

    To use SceneEnvironment or \l ExtendedSceneEnvironment, associate the
    \l{View3D::environment}{environment property} of a View3D with an instance
    of these types. The object can be declared inline, for example like this:

    \qml
        View3D {
            environment: SceneEnvironment {
                antialiasingMode: SceneEnvironment.MSAA
                tonemapMode: SceneEnvironment.TonemapModeFilmic
                backgroundMode: SceneEnvironment.SkyBox
                lightProbe: Texture {
                    source: "panoramic_hdri_background.hdr"
                }
            }
        }
    \endqml

    Alternatively, the environment object can be defined separately. It can
    then be referenced by one or more View3D objects. An example code snippet,
    using \l ExtendedSceneEnvironment this time:

    \qml
        ExtendedSceneEnvironment {
            id: myEnv
            vignetteEnabled: true
        }

        View3D {
            width: parent.width / 2
            environment: myEnv
        }

        View3D {
            width: parent.width / 2
            x: parent.width / 2
            environment: myEnv
        }
    \endqml

    \section1 Feature Overview

    \list

    \li Anti-aliasing settings. See \l{Anti-Aliasing Best Practices} for an
    overview of this topic. The relevant properties are \l antialiasingMode, \l
    antialiasingQuality, \l specularAAEnabled, \l temporalAAEnabled, \l
    temporalAAStrength. In addition, if \l ExtendedSceneEnvironment is used,
    another method is available via
    \l{ExtendedSceneEnvironment::fxaaEnabled}{fxaaEnabled}.

    \li Screen space ambient occlusion. The relevant properties are \l
    aoEnabled, \l aoStrength, \l aoBias, \l aoDistance, \l aoDither, \l
    aoSampleRate, \l aoSoftness. Note that
    \l{ExtendedSceneEnvironment::ssgiEnabled}{SSGI} also provides ambient
    occlusion, and it will often give better quality results, albeit possibly at
    a higher performance cost. Avoid enabling SceneEnvironment's ambient
    occlusion and ExtendedSceneEnvironment's SSGI together.

    \li Clear color, skybox, image-based lighting. For more information on IBL,
    see \l{Using Image-Based Lighting}. The relevant properties are \l
    backgroundMode, \l clearColor, \l lightProbe, \l probeExposure, \l
    probeHorizon, \l probeOrientation, \l skyboxBlurAmount, \l skyBoxCubeMap.

    \li Tonemapping. \l tonemapMode configures the tonemapping method that is
    used to convert the high dynamic range color values to the 0-1 range at the
    end of the graphics pipeline. \l ExtendedSceneEnvironment offers a few
    additional properties, such as
    \l{ExtendedSceneEnvironment::whitePoint}{whitePoint} and
    \l{ExtendedSceneEnvironment::sharpnessAmount}{sharpnessAmount} that can be
    used to tune the tonemapping calculations.

    \li Depth buffer settings. The relevant properties are \l
    depthPrePassEnabled, \l depthTestEnabled.

    \li Post-processing effects. In addition to the built-in post-processing
    effects provided by \l ExtendedSceneEnvironment, applications can provide
    their own custom effects via the \l Effect type. The \l effects property is
    a list of \l Effect instances.

    \li Debug visualization settings, such as wireframe mode or rendering only
    certain color contributions for the materials. This is controlled by the \l
    DebugSettings object referenced from the \l debugSettings property. Most of
    these settings can also be controlled interactively when a \l DebugView
    item is added to the scene.

    \li Fog settings. To enable fog, set an appropriately configured
    \l [QML] Fog object in the \l [QML] fog property.

    \li Lightmap baking settings. When pre-baked lightmaps are used for some
    models in the scene, the \l Lightmapper object set in the \l lightmapper
    property defines the settings used during the baking process.

    \li Scissor settings. To apply a scissor different than the viewport, set
    the \l scissorRect property.

    \endlist

    \sa ExtendedSceneEnvironment
*/

QQuick3DSceneEnvironment::QQuick3DSceneEnvironment(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::SceneEnvironment)), parent)
{
    m_debugSettings = new QQuick3DDebugSettings(this);
    m_debugSettingsSignalConnection = QObject::connect(m_debugSettings, &QQuick3DDebugSettings::changed, this,
                                                       [this] { update(); });
    QObject::connect(m_debugSettings, &QObject::destroyed, this,
                     [this](QObject *obj) {
        if (m_debugSettings == obj) {
            m_debugSettings = nullptr;
            update();
        }
    });
}

QQuick3DSceneEnvironment::~QQuick3DSceneEnvironment()
{
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::antialiasingMode
    \since 5.15

    This property controls the antialiasing mode that is applied when rendering
    the scene.

    Possible values are:
    \value SceneEnvironment.NoAA No antialiasing is applied.
    \value SceneEnvironment.SSAA Supersample antialiasing is applied.
    \value SceneEnvironment.MSAA Multisample antialiasing is applied.
    \value SceneEnvironment.ProgressiveAA Progressive antialiasing is applied.

    The default value is \c SceneEnvironment.NoAA.

    \b Supersampling

    The scene is rendered in a higher resolution, and then scaled down to
    actual resolution.

    \b Pros: High quality. Antialiases all scene content and not just geometry
    silhouettes.

    \b Cons: Usually more expensive than MSAA. Increases video memory usage.
    Supported with View3D items with all renderMode except Inline, but since
    the technique implies rendering to a texture first, enabling SSAA with a
    renderMode of Underlay or Overlay will result in using an intermediate
    texture and render pass that would normally not be needed, meaning the
    performance costs may be more noticeable. It is recommended to use SSAA
    only when the renderMode is the default Offscreen.

    \b Multisampling

    The edges of geometry are super-sampled, resulting in smoother silhouettes.
    This technique has no effect on the materials inside geometry, however.

    \b Pros: Works with any View3D item regardless of the renderMode. Good
    results on geometry silhouettes, where aliasing is often most noticeable;
    works with fast animation without issues. Performance depends purely on the
    system's (GPU) capabilities.

    \b Cons: Does not help with texture or reflection issues. Increases video
    memory usage. Can be expensive to use on less powerful graphics hardware.
    Can be controlled on a per-window basis or for individual View3D items
    depending on the renderMode. When using Underlay/Overlay with an effect
    applied or Offscreen, MSAA can be controlled for each View3D item. On the
    other hand, using Underlay/Overlay without any effect or Inline will make
    MSAA controlled per-window.

    \note For View3D items with a \l{QtQuick3D::View3D::renderMode}{renderMode}
    other than Underlay/Overlay with effects or Offscreen, multisampling can only
    be enabled via the \l{QSurfaceFormat::setSamples()}{QSurfaceFormat} of the
    QQuickWindow or QQuickView. This will then affect all content,
    both 2D and 3D, in that window.

    \b {Progressive antialiasing}

    This property enables and sets the level of progressive antialiasing
    applied to the scene.

    When all content of the scene has stopped moving, the camera is jiggled
    very slightly between frames, and the result of each new frame is blended
    with the previous frames. The more frames you accumulate, the better
    looking the result.

    \b Pros: Provides great results when all content in the scene is standing still.

    \b Cons: Does not take effect if any visual changes are occurring.
    Expensive due to having to accumulate and blend. Increases video memory
    usage.

    \note Progressing antialiasing is not currently supported with multiview
    rendering, and should not be used in VR/AR applications.

    See \l{Anti-Aliasing Best Practices} for further discussion on
    anti-aliasing methods.
*/
QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues QQuick3DSceneEnvironment::antialiasingMode() const
{
    return m_antialiasingMode;
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::antialiasingQuality
    \since 5.15

    This property sets the level of antialiasing applied to the scene.
    Behavior depends on used antialiasingMode. With antialiasingMode
    property set to \c NoAA this property doesn't have an effect.

    Possible values are:
    \value SceneEnvironment.Medium
    SSAA: Antialiasing uses 1.2x supersampling resolution.\br
    MSAA: Antialiasing uses 2 samples per pixel.\br
    ProgressiveAA: Antialiasing uses 2 frames for final image.
    \value SceneEnvironment.High
    SSAA: Antialiasing uses 1.5x supersampling resolution.\br
    MSAA: Antialiasing uses 4 samples per pixel.\br
    ProgressiveAA: Antialiasing uses 4 frames for final image.
    \value SceneEnvironment.VeryHigh
    SSAA: Antialiasing uses 2.0x supersampling resolution.\br
    MSAA: Antialiasing uses 8 samples per pixel.\br
    ProgressiveAA: Antialiasing uses 8 frames for final image.

    The default value is \c SceneEnvironment.High
*/

QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues QQuick3DSceneEnvironment::antialiasingQuality() const
{
    return m_antialiasingQuality;
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::backgroundMode

    This property controls if and how the background of the scene should be
    cleared.

    \note The clearing of the color buffer backing the View 3D does not always
    happen: depending on the \l{QtQuick3D::View3D::renderMode}{renderMode}
    property the View3D may not perform any clearing on its own, in which case
    \c{SceneEnvironment.Transparent} and \c{SceneEnvironment.Color} have no
    effect. Only the default \c Offscreen \l{View3D::renderMode}{render mode}
    (rendering into a texture) supports all clearing modes. With the \c
    Underlay mode, use \l{QQuickWindow::setColor()} or
    \l[QtQuick]{Window::color}{Window.color} to control the clear color for the
    Qt Quick scene. SkyBox is handled differently, as it implies drawing actual
    geometry, so that works identically across all render modes.

    \value SceneEnvironment.Transparent
        The scene is cleared to be transparent. This is useful to render 3D content on top of another item.
        This mode has no effect when the View3D is using a renderMode of Underlay or Overlay without any
        post processing enabled.
    \value SceneEnvironment.Color
        The scene is cleared with the color specified by the clearColor property.
        This mode has no effect when the View3D is using a renderMode of Underlay or Overlay without any
        post processing enabled.
    \value SceneEnvironment.SkyBox
        The scene will not be cleared, but instead a SkyBox or Skydome will be rendered. The SkyBox
        is defined using the HDRI map defined in the lightProbe property.
    \value SceneEnvironment.SkyBoxCubeMap
        The scene will not be cleared, but instead a SkyBox or Skydome will be rendered. The SkyBox
        is defined using the cubemap defined in the skyBoxCubeMap property.

    The default value is \c SceneEnvironment.Transparent

    Take the following example. The Suzanne model is expected to be
    pre-processed with the \c balsam tool and is sourced from the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models}{glTF Sample Models}
    repository.

    \qml
        import QtQuick
        import QtQuick3D
        import QtQuick3D.Helpers

        Item {
            width: 1280
            height: 720

            View3D {
                id: v3d
                anchors.fill: parent

                environment: ExtendedSceneEnvironment {
                    backgroundMode: SceneEnvironment.SkyBox
                    lightProbe: Texture { source: "00455_OpenfootageNET_field_low.hdr" }

                    glowEnabled: true
                    glowStrength: 1.25
                    glowBloom: 0.25
                    glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
                }

                DirectionalLight {
                }

                Suzanne {
                    scale: Qt.vector3d(50, 50, 50)
                    z: -500
                }

                PerspectiveCamera {
                    id: camera
                }

                WasdController {
                    controlledObject: camera
                }
            }
        }
    \endqml

    Using image-based lighting in additional to the DirectionalLight and also
    using the light probe texture as the skybox gives us the following:

    \image sceneenvironment_background_ibl.jpg
           {3D object with environment background}

    What happens if there is no light probe?

    \qml
        backgroundMode: SceneEnvironment.Transparent
    \endqml

    Here the background is provided not by the View3D but by the QQuickWindow
    or QQuickView hosting the 2D and 3D scene. Lighting is based on the
    DirectionalLight only.

    \image sceneenvironment_background_transparent.jpg
           {3D object with transparent background}

    Using a fixed clear color:

    \qml
        backgroundMode: SceneEnvironment.Color
        clearColor: "green"
    \endqml

    \image sceneenvironment_background_color.jpg
           {3D object with green background}

    \sa lightProbe, QQuickWindow::setColor(), Window::color, View3D
*/

QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes QQuick3DSceneEnvironment::backgroundMode() const
{
    return m_backgroundMode;
}

/*!
    \qmlproperty color QtQuick3D::SceneEnvironment::clearColor

    This property defines which color will be used to clear the viewport when
    using \c SceneEnvironment.Color for the backgroundMode property.

    The default value is \c Qt::black

    \sa backgroundMode
*/

QColor QQuick3DSceneEnvironment::clearColor() const
{
    return m_clearColor;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::aoStrength

    This property defines the amount of ambient occulusion applied. Ambient
    occulusion is a form of approximated global illumination which causes
    non-directional self-shadowing where objects are close together.
    A value of 100 causes full darkness shadows; lower values cause the
    shadowing to appear lighter. A value of 0 disables ambient occlusion
    entirely, improving performance at a cost to the visual realism of 3D
    objects rendered in the scene.

    All values other than 0 have the same impact to the performance.

    The default value is 0.0. The maximum value is 100.0.

    A value of 0 is equivalent to setting \l aoEnabled to false.

    Pictured here with the default aoSoftness and aoDistance:

    \table
    \header
    \li aoStrength of 0 (AO disabled)
    \li aoStrength of 100
    \li aoStrength of 50
    \row
    \li \image sceneenvironment_ao_off.jpg
               {Scene with ambient occlusion strength of 0}
    \li \image sceneenvironment_ao_full_strength.jpg
               {Scene with ambient occlusion strength of 100}
    \li \image sceneenvironment_ao_half_strength.jpg
               {Scene with ambient occlusion strength of 50}
    \endtable

    \note Getting visually good-looking screen space ambient occlusion is
    dependent on carefully tuning a number of related parameters, such as \l
    aoStrength, \l aoSoftness, \l aoDistance, \l aoDither, \l aoBias, and \l
    aoSampleRate.

    \sa aoEnabled, aoDistance, aoSoftness
*/
float QQuick3DSceneEnvironment::aoStrength() const
{
    return m_aoStrength;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::aoDistance

    This property defines roughly how far ambient occlusion shadows spread away
    from objects. Greater distances cause increasing impact to performance.

    The default value is 5.0.

    Pictured here with the default aoSoftness and the maximum aoStrength:

    \table
    \header
    \li aoDistance of 5
    \li aoDistance of 1
    \row
    \li \image sceneenvironment_ao_distance_5.jpg
               {Scene with ambient occlusion distance of 5}
    \li \image sceneenvironment_ao_distance_1.jpg
               {Scene with ambient occlusion distance of 1}
    \endtable

    \note Getting visually good-looking screen space ambient occlusion is
    dependent on carefully tuning a number of related parameters, such as \l
    aoStrength, \l aoSoftness, \l aoDistance, \l aoDither, \l aoBias, and \l
    aoSampleRate.

    \sa aoStrength, aoSoftness
*/
float QQuick3DSceneEnvironment::aoDistance() const
{
    return m_aoDistance;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::aoSoftness

    This property defines how smooth the edges of the ambient occlusion shading
    are.

    The value must be between 0.0 and 50.0. The default value is 50.0.

    Pictured here with the default aoDistance and the maximum aoStrength:

    \table
    \header
    \li aoSoftness of 50
    \li aoSoftness of 25
    \row
    \li \image sceneenvironment_ao_softness_default.jpg
               {Scene with ambient occlusion softness of 50}
    \li \image sceneenvironment_ao_softness_half.jpg
               {Scene with ambient occlusion softness of 25}
    \endtable

    \note Getting visually good-looking screen space ambient occlusion is
    dependent on carefully tuning a number of related parameters, such as \l
    aoStrength, \l aoSoftness, \l aoDistance, \l aoDither, \l aoBias, and \l
    aoSampleRate.

    \sa aoStrength, aoDistance
*/
float QQuick3DSceneEnvironment::aoSoftness() const
{
    return m_aoSoftness;
}

/*!
    \qmlproperty bool QtQuick3D::SceneEnvironment::aoDither

    When this property is enabled it scatters the edges of the ambient
    occlusion shadow bands to improve smoothness (at the risk of sometimes
    producing obvious patterned artifacts).

    \note Very large distances between the clipping planes of your camera may
    cause problems with ambient occlusion. If you are seeing odd banding in
    your ambient occlusion, try adjusting the \l {PerspectiveCamera::}{clipFar}
    property of your camera to be closer to your content.

    The default value is \c false.

    \sa {QtQuick3D::PerspectiveCamera::clipFar}{PerspectiveCamera.clipFar},
        {QtQuick3D::OrthographicCamera::clipFar}{OrthographicCamera.clipFar}
*/
bool QQuick3DSceneEnvironment::aoDither() const
{
    return m_aoDither;
}

/*!
    \qmlproperty int QtQuick3D::SceneEnvironment::aoSampleRate

    This property defines ambient occlusion quality (more shades of gray) at
    the expense of performance.

    The value must be 2, 3, or 4. The default value is 2.
*/
int QQuick3DSceneEnvironment::aoSampleRate() const
{
    return m_aoSampleRate;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::aoBias

    This property defines a cutoff distance preventing objects from exhibiting
    ambient occlusion at close distances. Higher values increase the distance
    required between objects before ambient occlusion is seen.

    \note If you see ambient occlusion shadowing on objects where there should
    be no shadowing, increase the value slightly to clip away close results.

    The default value is 0.0.
*/
float QQuick3DSceneEnvironment::aoBias() const
{
    return m_aoBias;
}

/*!
    \qmlproperty QtQuick3D::Texture QtQuick3D::SceneEnvironment::lightProbe

    This property defines an image used to light the scene, either instead of,
    or in addition to standard lights.

    The image is preferably a high-dynamic range image or a \l{Pre-generating
    IBL cubemap}{pre-generated cubemap}. Pre-baking provides significant
    performance improvements at run time, because no time is spent on filtering
    and mipmap generation. If the source is a .hdr or other image, the GPU-based
    pre-processing happens at run time after loading the image file, and that
    can be potentially time consuming, in particular on embedded and mobile
    hardware. Therefore, it is strongly recommended that applications
    pre-process .hdr images at latest at build time, as described
    \l{Pre-generating IBL cubemap}{here}.

    \note Using a Texture with \l{Texture::sourceItem}{sourceItem} is not
    supported in combination with this property. Pre-filtering of all mip
    levels for dynamic Qt Quick content is typically not reasonable in practice
    due to performance implications.

    For more information on image-based lighting, see \l{Using Image-Based Lighting}.

    \note The light probe texture, when the property is set to a valid Texture,
    is used for lighting regardless of the \l backgroundMode. However, when \l
    backgroundMode is set to \c{SceneEnvironment.SkyBox}, the texture is also
    used to render the scene background as a skybox.

    The examples below were generated with varying the \l backgroundMode in the
    environment of the following scene. The scene has no DirectionLight,
    PointLight, or SpotLight. All lighting is based on the panoramic HDRI
    image.

    \qml
        import QtQuick
        import QtQuick3D
        import QtQuick3D.Helpers

        Item {
            width: 1280
            height: 720

            View3D {
                id: v3d
                anchors.fill: parent

                environment: ExtendedSceneEnvironment {
                    backgroundMode: SceneEnvironment.SkyBox
                    lightProbe: Texture { source: "00455_OpenfootageNET_field_low.hdr" }

                    tonemapMode: SceneEnvironment.TonemapModeFilmic
                    sharpnessAmount: 0.4

                    glowEnabled: true
                    glowStrength: 1.25
                    glowBloom: 0.25
                    glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
                }

                Node {
                    scale: Qt.vector3d(100, 100, 100)

                    Sponza {
                    }

                    Suzanne {
                        y: 1
                        scale: Qt.vector3d(0.5, 0.5, 0.5)
                        eulerRotation.y: -90
                    }
                }

                PerspectiveCamera {
                    id: camera
                    y: 100
                }

                WasdController {
                    controlledObject: camera
                }
            }
        }
    \endqml

    Results with the above environment:

    \image sceneenvironment_lightprobe.jpg
           {Scene with light probe}
    \image sceneenvironment_lightprobe_2.jpg
           {Scene with light probe from different perspective}

    Switching the backgroundMode to \c{SceneEnvironment.Transparent} would give us:

    \image sceneenvironment_lightprobe_transparent.jpg
           {Scene with backgroundMode set to transparent}
    \image sceneenvironment_lightprobe_transparent_2.jpg
           {Scene with backgroundMode set to transparent from different
           perspective}

    Here the lighting of the 3D scene is the same as before, meaning the
    materials use the light probe in the lighting calculations the same way as
    before, but there is no skybox rendered. The background is white since that
    is the default clear color of the QQuickWindow hosting the 2D and 3D scene.

    It is valid to set the lightProbe property value back to the default null.
    This unassigns the previously associated texture. For example, let's use
    the Delete key to dynamically toggle between image-based lighting with a
    skybox, and no image-based lighting with a fixed clear color for the
    background:

    \qml
        environment: ExtendedSceneEnvironment {
            id: env

            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: iblTex

            tonemapMode: SceneEnvironment.TonemapModeFilmic
            sharpnessAmount: 0.4

            glowEnabled: true
            glowStrength: 1.25
            glowBloom: 0.25
            glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
        }

        Texture {
            id: iblTex
            source: "00455_OpenfootageNET_field_low.hdr"
        }

        focus: true
        Keys.onDeletePressed: {
            if (env.backgroundMode == SceneEnvironment.SkyBox) {
                env.backgroundMode = SceneEnvironment.Color;
                env.clearColor = "green";
                env.lightProbe = null;
            } else {
                env.backgroundMode = SceneEnvironment.SkyBox;
                env.lightProbe = iblTex;
            }
        }
    \endqml

    Pressing Delete gives the following result. Remember that the scene used
    here has no lights so all 3D models appear completely black.

    \image sceneenvironment_lightprobe_null.jpg
           {3D objects without light probe on green background}
    \image sceneenvironment_lightprobe_null_2.jpg
           {Ground plane without light probe on green background}

    While lightProbe is commonly used in combination with Texture instances
    that source their data from an image file (typically .hdr or .ktx), it can
    also makes sense to associate with a Texture that uses in-memory,
    \l{Texture::textureData}{procedurally generated image data}. A prime
    example of this is a Texture where the image data is generated by \l
    ProceduralSkyTextureData from the QtQuick3D.Helpers module:

    \qml
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {
                }
            }
    \endqml

    This gives us a procedurally generated HDR skybox texture that is now used
    both as the skybox and for image-based lighting:

    \image sceneenvironment_lightprobe_proceduralsky.jpg {Architectural scene lit by procedural sky visible in background}

    \sa backgroundMode, {Using Image-Based Lighting}, {Pre-generating IBL
    cubemap}, probeExposure, probeHorizon, probeOrientation, ProceduralSkyTextureData
*/
QQuick3DTexture *QQuick3DSceneEnvironment::lightProbe() const
{
    return m_lightProbe;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::probeExposure

    This property modifies the amount of light emitted by the light probe. Part
    of the tonemapping is exposure mapping, and this property adjusts how
    the light values in the light probes get tonemaped.

    By default exposure is set to is 1.0

    \note This property does not have an effect when \l tonemapMode is set to
    \c SceneEnvironment.TonemapModeNone.

    \sa lightProbe, probeHorizon, probeOrientation
*/
float QQuick3DSceneEnvironment::probeExposure() const
{
    return m_probeExposure;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::probeHorizon

    This property when defined with increasing values adds darkness (black)
    to the bottom half of the environment, forcing the lighting to come
    predominantly from the top of the image (and removing specific reflections
    from the lower half). This property is useful for accounting for a ground
    plane that would have the effect of obscuring the reflection of the light
    probe from the ground. This is necessary because light probe contributions
    come directily from the image without consideration for the content of the
    scene.

    The expected value range for the probeHorizon property is between 0.0
    and 1.0. Any value outside of this range will be clamped to the
    expected range.

    By default probeHorizon is set to 0.0 which means the whole light probe
    is used without adjustment.

    \note The probeHorizon property only affects materials lighting, and has
    no effect on the rendering of the sky box.

    \sa lightProbe, probeExposure, probeOrientation
*/
float QQuick3DSceneEnvironment::probeHorizon() const
{
    return m_probeHorizon;
}

/*!
    \qmlproperty vector3d QtQuick3D::SceneEnvironment::probeOrientation

    This property when defines the orientation of the light probe. Orientation
    is defined in terms of euler angles in degrees over the x, y, and z axes.

    \note This value augments how the lightProbe Texture is sampled in combination
    with any texture rotations and offsets set on the lightProbe texture.

    \sa lightProbe, probeHorizon, probeExposure
*/
QVector3D QQuick3DSceneEnvironment::probeOrientation() const
{
    return m_probeOrientation;
}

/*!
    \qmlproperty bool QtQuick3D::SceneEnvironment::temporalAAEnabled

    When this property is enabled temporal antialiasing will be used.

    The camera is jiggled very slightly between frames, and the result of each
    new frame is blended with the previous frame.

    \note Temporal antialiasing doesn't have an effect when antialiasingMode is MSAA.
    \note When using ProgressiveAA, temporal AA is applied during scene animation.
    Once the scene becomes static, ProgressiveAA takes over — but only when
    temporalAAMode is SceneEnvironment.TAADefault.
    In other words: ProgressiveAA has no effect when temporalAAMode is set to
    SceneEnvironment.TAAMotionVector.

    \b Pros: Due to the jiggling camera it finds real details that were otherwise
    lost; low impact on performance.

    \b Cons: Fast-moving objects cause one-frame ghosting.

    \default false

    \note Temporal antialiasing is not currently supported with multiview
    rendering, and should not be used in VR/AR applications.
*/
bool QQuick3DSceneEnvironment::temporalAAEnabled() const
{
    return m_temporalAAEnabled;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::temporalAAStrength
    \since 5.15

    This property modifies the amount of temporal movement (antialiasing).
    This has an effect only when temporalAAEnabled property is true.

    \default 0.3

    \sa temporalAAEnabled
*/
float QQuick3DSceneEnvironment::temporalAAStrength() const
{
    return m_temporalAAStrength;
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::temporalAAMode
    \since 6.11

    Controls the temporal anti-aliasing mode. Temporal AA reduces flickering and
    aliasing by blending information across multiple frames.

    \value SceneEnvironment.TAADefault
    Blends current and previous frames for basic temporal filtering.

    \value SceneEnvironment.TAAMotionVector
    Uses motion vectors to track pixel movement between frames, providing
    superior quality for animated scenes.

    The default is \c SceneEnvironment.TAADefault.

    \note Temporal AA may cause ghosting artifacts with fast-moving objects or
       rapid camera movements.

    \note Works best with constant frame rates. Frame rate variations reduce
    effectiveness.
*/
QQuick3DSceneEnvironment::QQuick3DEnvironmentTemporalAAMode QQuick3DSceneEnvironment::temporalAAMode() const
{
    return m_temporalAAMode;
}

/*!
    \qmlproperty bool QtQuick3D::SceneEnvironment::specularAAEnabled
    \since 6.4

    When this property is enabled, specular aliasing will be mitigated.
    Specular aliasing is often visible in form of bright dots, possibly
    flickering when moving the camera around.

    The default value is false.

    \table
    \header
    \li Specular AA disabled
    \li Specular AA enabled
    \row
    \li \image specular_aa_off.jpg {Architectural scene showing
               specular highlights without antialiasing}
    \li \image specular_aa_on.jpg {Architectural scene showing
               smoothed specular highlights with antialiasing}
    \endtable
*/
bool QQuick3DSceneEnvironment::specularAAEnabled() const
{
    return m_specularAAEnabled;
}

/*!
    \qmlproperty bool QtQuick3D::SceneEnvironment::depthTestEnabled

    The default value is \c true. By default the renderer classifies the objects
    in the scene either as \c opaque or as \c semi-transparent. The objects
    (sub-meshes with the associated material) in the \c opaque list are rendered
    first, with depth testing and depth write enabled, providing optimal
    Z-culling for typical 3D objects that have no semi-transparent regions. The
    objects in the \c semi-transparent list are rendered with depth write
    disabled, although still with depth testing enabled (to test against the
    opaque objects), in back to front order (sorted based on their center point's
    distance from the camera). This allows correct blending ("see through") for
    3D objects that involve semi-transparent regions on their surface, either
    due to the \l{Node::opacity}{node opacity} or due to some color or texture
    map in the material.

    When this property is set to \c {false}, the Z-buffer is not written and
    tested against, the depth test is skipped, and all objects, including fully
    opaque ones, are rendered in one go, sorted back to front.

    Setting this property to \c false should be rarely needed. It can be useful
    in scenes where it is known that there is little benefit in the two-round
    approach because either there are very few opaque objects, or they are
    transformed in a way that a single back to front sorted pass performs
    better.

    \note Setting this property to \c false may cause rendering errors in
    certain scenes. In addition, some features, such as shadows, ambient
    occlusion, \c SCREEN_TEXTURE and \c DEPTH_TEXTURE in custom materials and
    effects, will not behave correctly without enabling depth buffer usage.

    \note This flag has no control over the presence of a depth or
    depth-stencil buffer. Such buffers may still be allocated even when this is
    set to \c false.
*/
bool QQuick3DSceneEnvironment::depthTestEnabled() const
{
    return m_depthTestEnabled;
}
/*!
    \qmlproperty bool QtQuick3D::SceneEnvironment::depthPrePassEnabled

    When enabled, the renderer performs a Z-prepass for opaque objects, meaning
    it renders them with a simple shader and color write disabled in order to
    get the depth buffer pre-filled before issuing draw calls for the main
    rendering passes.

    This can improve performance depending on the scene contents. It is
    typically scenes with lots of overlapping objects and expensive fragment
    shading that benefit from this. At the same time, it is worth noting that
    the renderer performs front to back sorting for opaque objects, which in
    itself helps reducing unnecessary fragment shading, and therefore the
    Z-prepass does not always bring significant improvements.

    On GPUs that use a tiled rendering architecture, which is common in mobile
    and embedded systems, it is recommended to set this to \c false.

    The default value is \c false.

    \note This property has no effect when depth testing is disabled.
*/
bool QQuick3DSceneEnvironment::depthPrePassEnabled() const
{
    return m_depthPrePassEnabled;
}

/*!
    \qmlproperty List<QtQuick3D::Effect> QtQuick3D::SceneEnvironment::effects
    \since 5.15

    This property contains a list of post-processing effects that will be
    applied to the entire viewport. The result of each effect is fed to the
    next so the order is significant.

    \note For technical reasons, adding the same \l{QtQuick3D::Effect}{Effect}
    node several times to the list is unsupported and will give unexpected results.
*/
QQmlListProperty<QQuick3DEffect> QQuick3DSceneEnvironment::effects()
{
    return QQmlListProperty<QQuick3DEffect>(this,
                                            nullptr,
                                            QQuick3DSceneEnvironment::qmlAppendEffect,
                                            QQuick3DSceneEnvironment::qmlEffectsCount,
                                            QQuick3DSceneEnvironment::qmlEffectAt,
                                            QQuick3DSceneEnvironment::qmlClearEffects);
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::tonemapMode
    \since 6.0

    This property defines how colors are tonemapped before rendering. All
    rendering in Qt Quick 3D is performed in linear color space and can in
    many cases lead to generating color values that are not displayable. The
    tonemapMode determines the technique that is used to remap colors into a
    displayable range.

    The default value is \c SceneEnvironment.TonemapModeLinear

    \value SceneEnvironment.TonemapModeNone
        All Tonemapping is bypassed. This mode is useful when performing post
        processing effects.
    \value SceneEnvironment.TonemapModeLinear
        Linear tonemapping is applied. Colors are gamma corrected and returned
        in sRGB color space.
    \value SceneEnvironment.TonemapModeAces
        Academy Color Encoding System tonemapping is applied.
    \value SceneEnvironment.TonemapModeHejlDawson
        Hejl-Dawson tonemapping is applied.
    \value SceneEnvironment.TonemapModeFilmic
        Filmic tonemapping is applied.

    See \l{ExtendedSceneEnvironment::tonemapMode}{ExtendedSceneEnvironment} for
    an example of these different modes.

    \note When using post-processing effects, most effects expect untonemapped
    linear color data. With application-provided, custom effects implemented
    via the \l Effect type, it is important to know that starting with Qt 6.5
    effects can safely assume that they work with linear color data, and
    tonemapping is performed automatically on the output of the last effect in
    the chain. If there is a need to customize tonemapping completely, consider
    setting the \c SceneEnvironment.TonemapModeNone value to disable the
    built-in tonemapper, and perform the appropriate adjustments on the color
    value in the last effect in the chain instead. This does not apply to the
    built-in effects of \l ExtendedSceneEnvironment, because those
    automatically take care of proper tonemapping regardless of what
    combination of built-in effects are enabled in the environment.
*/
QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes QQuick3DSceneEnvironment::tonemapMode() const
{
    return m_tonemapMode;
}

/*!
    \qmlproperty real QtQuick3D::SceneEnvironment::skyboxBlurAmount
    \since 6.4

    This property determines how much much the skybox should be blurred when
    using \c SceneEnvironment.SkyBox for the \l backgroundMode property. The
    default value is \c 0.0 which means there is no blurring.

    Acceptable values range between 0.0 and 1.0, all other values will be clamped
    to this range.

*/

float QQuick3DSceneEnvironment::skyboxBlurAmount() const
{
    return m_skyboxBlurAmount;
}

/*!
    \qmlproperty QtQuick3D::DebugSettings QtQuick3D::SceneEnvironment::debugSettings
    \since 6.5

    This property specifies a \c DebugSettings object which is used to
    configure the debugging tools of the renderer. During construction
    the SceneEnvironment automatically creates a DebugSettings object
    associated with itself, and therefore setting a custom DebugSettings
    is usually not required.

    An example of rendering the scene with wireframe mode enabled:
    \image debugsettings_wireframe.jpg {Sponza scene in wireframe mode}

    Visualizing the normal vectors of the meshes:
    \image debugsettings_normals.jpg {Sponza scene showing surface normals}

    Visualizing the specular lighting contribution:
    \image debugsettings_specular.jpg {Sponza scene showing specular highlights}

    \sa DebugSettings
*/

QQuick3DDebugSettings *QQuick3DSceneEnvironment::debugSettings() const
{
    return m_debugSettings;
}

/*!
    \qmlproperty rect QtQuick3D::SceneEnvironment::scissorRect
    \since 6.5

    This property defines a scissor rectangle in view coordinates, with the
    top-left corner at [0, 0]
*/

QRect QQuick3DSceneEnvironment::scissorRect() const
{
    return m_scissorRect;
}

void QQuick3DSceneEnvironment::setAntialiasingMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues antialiasingMode)
{
    if (m_antialiasingMode == antialiasingMode)
        return;

    m_antialiasingMode = antialiasingMode;
    emit antialiasingModeChanged();
    update();
}

void QQuick3DSceneEnvironment::setAntialiasingQuality(QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues antialiasingQuality)
{
    if (m_antialiasingQuality == antialiasingQuality)
        return;

    m_antialiasingQuality = antialiasingQuality;
    emit antialiasingQualityChanged();
    update();
}

void QQuick3DSceneEnvironment::setBackgroundMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged();
    update();
}

void QQuick3DSceneEnvironment::setClearColor(const QColor &clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoStrength(float aoStrength)
{
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;

    const bool aoEnabled = !(qFuzzyIsNull(m_aoStrength) || qFuzzyIsNull(m_aoDistance));
    setAoEnabled(aoEnabled);

    emit aoStrengthChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoDistance(float aoDistance)
{
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;

    const bool aoEnabled = !(qFuzzyIsNull(m_aoStrength) || qFuzzyIsNull(m_aoDistance));
    setAoEnabled(aoEnabled);

    emit aoDistanceChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoSoftness(float aoSoftness)
{
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoDither(bool aoDither)
{
    if (m_aoDither == aoDither)
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged();
    update();
}

void QQuick3DSceneEnvironment::setAoBias(float aoBias)
{
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged();
    update();
}

void QQuick3DSceneEnvironment::setLightProbe(QQuick3DTexture *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSceneEnvironment::setLightProbe, lightProbe, m_lightProbe);

    m_lightProbe = lightProbe;
    emit lightProbeChanged();
    update();
}

void QQuick3DSceneEnvironment::setProbeExposure(float probeExposure)
{
    if (qFuzzyCompare(m_probeExposure, probeExposure))
        return;

    m_probeExposure = probeExposure;
    emit probeExposureChanged();
    update();
}

void QQuick3DSceneEnvironment::setProbeHorizon(float probeHorizon)
{
    // clamp value to expected range
    probeHorizon = qBound(0.0f, probeHorizon, 1.0f);

    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged();
    update();
}

void QQuick3DSceneEnvironment::setProbeOrientation(const QVector3D &orientation)
{
    if (qFuzzyCompare(m_probeOrientation, orientation))
        return;

    m_probeOrientation = orientation;
    emit probeOrientationChanged();
    update();
}

void QQuick3DSceneEnvironment::setDepthTestEnabled(bool depthTestEnabled)
{
    if (m_depthTestEnabled == depthTestEnabled)
        return;

    m_depthTestEnabled = depthTestEnabled;
    emit depthTestEnabledChanged();
    update();
}

void QQuick3DSceneEnvironment::setDepthPrePassEnabled(bool depthPrePassEnabled)
{
    if (m_depthPrePassEnabled == depthPrePassEnabled)
        return;

    m_depthPrePassEnabled = depthPrePassEnabled;
    emit depthPrePassEnabledChanged();
    update();
}

void QQuick3DSceneEnvironment::setTonemapMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes tonemapMode)
{
    if (m_tonemapMode == tonemapMode)
        return;

    m_tonemapMode = tonemapMode;
    emit tonemapModeChanged();
    update();
}

bool QQuick3DSceneEnvironment::useBuiltinTonemapper() const
{
    return true;
}

QSSGRenderGraphObject *QQuick3DSceneEnvironment::updateSpatialNode(QSSGRenderGraphObject *node)
{
    // Don't do anything, these properties get set by the scene renderer
    return node;
}

void QQuick3DSceneEnvironment::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

const QVector<QQuick3DEffect *> &QQuick3DSceneEnvironment::effectList() const
{
    return m_effects;
}

void QQuick3DSceneEnvironment::updateSceneManager(QQuick3DSceneManager *manager)
{
    if (manager) {
        QQuick3DObjectPrivate::refSceneManager(m_lightProbe, *manager);
        QQuick3DObjectPrivate::refSceneManager(m_skyBoxCubeMap, *manager);
    } else {
        QQuick3DObjectPrivate::derefSceneManager(m_lightProbe);
        QQuick3DObjectPrivate::derefSceneManager(m_skyBoxCubeMap);
    }
}

void QQuick3DSceneEnvironment::setTemporalAAMode(const QQuick3DEnvironmentTemporalAAMode &newTemporalAAMode)
{
    if (m_temporalAAMode == newTemporalAAMode)
        return;
    m_temporalAAMode = newTemporalAAMode;
    emit temporalAAModeChanged();
    update();
}

void QQuick3DSceneEnvironment::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged();
    update();
}

void QQuick3DSceneEnvironment::setTemporalAAStrength(float strength)
{
    if (qFuzzyCompare(m_temporalAAStrength, strength))
        return;

    m_temporalAAStrength = strength;
    emit temporalAAStrengthChanged();
    update();
}

void QQuick3DSceneEnvironment::setSpecularAAEnabled(bool enabled)
{
    if (m_specularAAEnabled == enabled)
        return;

    m_specularAAEnabled = enabled;
    emit specularAAEnabledChanged();
    update();
}

void QQuick3DSceneEnvironment::qmlAppendEffect(QQmlListProperty<QQuick3DEffect> *list, QQuick3DEffect *effect)
{
    if (effect == nullptr)
        return;
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    self->m_effects.push_back(effect);

    if (effect->parentItem() == nullptr)
        effect->setParentItem(self);

    for (QQuick3DEffect *e : self->m_effects)
        e->effectChainDirty();

    self->update();
}

QQuick3DEffect *QQuick3DSceneEnvironment::qmlEffectAt(QQmlListProperty<QQuick3DEffect> *list, qsizetype index)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    return self->m_effects.at(index);
}

qsizetype QQuick3DSceneEnvironment::qmlEffectsCount(QQmlListProperty<QQuick3DEffect> *list)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    return self->m_effects.size();
}

void QQuick3DSceneEnvironment::qmlClearEffects(QQmlListProperty<QQuick3DEffect> *list)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    self->m_effects.clear();
    self->update();
}

void QQuick3DSceneEnvironment::setSkyboxBlurAmount(float newSkyboxBlurAmount)
{
    newSkyboxBlurAmount = qBound(0.0f, newSkyboxBlurAmount, 1.0f);

    if (qFuzzyCompare(m_skyboxBlurAmount, newSkyboxBlurAmount))
        return;

    m_skyboxBlurAmount = newSkyboxBlurAmount;
    emit skyboxBlurAmountChanged();
    update();
}

/*!
    \qmlproperty Lightmapper QtQuick3D::SceneEnvironment::lightmapper

    When this property is set to a valid Lightmapper object, the settings
    specified by the object will be taken into account when baking lightmaps.

    The default value is null, which means using default values for all the
    baking-related settings.

    For more information on how to bake lightmaps, see the \l Lightmapper
    documentation.

    When lightmaps are not relevant to an application and baked lighting is
    never generated, the property and the associated object serve no purpose in
    practice.

    \sa Model::usedInBakedLighting, Model::bakedLightmap, Light::bakeMode, Lightmapper
 */

QQuick3DLightmapper *QQuick3DSceneEnvironment::lightmapper() const
{
    return m_lightmapper;
}

void QQuick3DSceneEnvironment::setLightmapper(QQuick3DLightmapper *lightmapper)
{
    if (m_lightmapper == lightmapper)
        return;

    if (m_lightmapper)
        m_lightmapper->disconnect(m_lightmapperSignalConnection);

    m_lightmapper = lightmapper;

    m_lightmapperSignalConnection = QObject::connect(m_lightmapper, &QQuick3DLightmapper::changed, this,
                                                     [this] { update(); });

    QObject::connect(m_lightmapper, &QObject::destroyed, this,
                     [this](QObject *obj)
    {
        if (m_lightmapper == obj) {
            m_lightmapper = nullptr;
            update();
        }
    });

    emit lightmapperChanged();
    update();
}

/*!
    \qmlproperty QtQuick3D::CubeMapTexture QtQuick3D::SceneEnvironment::skyBoxCubeMap

    This property defines a cubemap to be used as a skybox when the background mode is \c SkyBoxCubeMap.

    \since 6.4
*/
QQuick3DCubeMapTexture *QQuick3DSceneEnvironment::skyBoxCubeMap() const
{
    return m_skyBoxCubeMap;
}

void QQuick3DSceneEnvironment::setSkyBoxCubeMap(QQuick3DCubeMapTexture *newSkyBoxCubeMap)
{
    if (m_skyBoxCubeMap == newSkyBoxCubeMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSceneEnvironment::setSkyBoxCubeMap, newSkyBoxCubeMap, m_skyBoxCubeMap);

    m_skyBoxCubeMap = newSkyBoxCubeMap;
    emit skyBoxCubeMapChanged();
}

void QQuick3DSceneEnvironment::setDebugSettings(QQuick3DDebugSettings *newDebugSettings)
{
    if (m_debugSettings == newDebugSettings)
        return;

    if (m_debugSettings)
        m_debugSettings->disconnect(m_debugSettingsSignalConnection);

    m_debugSettings = newDebugSettings;

    m_debugSettingsSignalConnection = QObject::connect(m_debugSettings, &QQuick3DDebugSettings::changed, this,
                                                       [this] { update(); });
    QObject::connect(m_debugSettings, &QObject::destroyed, this,
                     [this](QObject *obj) {
        if (m_debugSettings == obj) {
            m_debugSettings = nullptr;
            update();
        }
    });

    emit debugSettingsChanged();
    update();
}

void QQuick3DSceneEnvironment::setScissorRect(QRect rect)
{
    if (m_scissorRect == rect)
        return;

    m_scissorRect = rect;
    emit scissorRectChanged();
    update();
}

bool QQuick3DSceneEnvironment::gridEnabled() const
{
    return m_gridEnabled;
}

void QQuick3DSceneEnvironment::setGridEnabled(bool newGridEnabled)
{
    if (m_gridEnabled == newGridEnabled)
        return;
    m_gridEnabled = newGridEnabled;
    update();
}

float QQuick3DSceneEnvironment::gridScale() const
{
    return m_gridScale;
}

void QQuick3DSceneEnvironment::setGridScale(float newGridScale)
{
    if (qFuzzyCompare(m_gridScale, newGridScale))
        return;
    m_gridScale = newGridScale;
    update();
}

uint QQuick3DSceneEnvironment::gridFlags() const
{
    return m_gridFlags;
}

void QQuick3DSceneEnvironment::setGridFlags(uint newGridFlags)
{
    if (m_gridFlags == newGridFlags)
        return;
    m_gridFlags = newGridFlags;
    update();
}

/*!
    \qmlproperty bool SceneEnvironment::aoEnabled
    \since 6.5

    Enable or disable ambient occlusion.

    The default value is \c false, which means ambient occlusion is disabled.

    \note If \l aoStrength or \ aoDistance is 0, then setting this property to
    \c true will also set those values appropriately to make the ambient
    occlusion effective.

    \note Getting visually good-looking screen space ambient occlusion is
    dependent on carefully tuning a number of related parameters, such as \l
    aoStrength, \l aoSoftness, \l aoDistance, \l aoDither, \l aoBias, and \l
    aoSampleRate.

    \sa aoStrength, aoDistance
*/

bool QQuick3DSceneEnvironment::aoEnabled() const
{
    return m_aoEnabled;
}

void QQuick3DSceneEnvironment::setAoEnabled(bool newAoEnabled)
{
    if (m_aoEnabled == newAoEnabled)
        return;

    m_aoEnabled = newAoEnabled;

    if (m_aoEnabled) {
        if (qFuzzyIsNull(m_aoStrength))
            setAoStrength(100.0f);
        if (qFuzzyIsNull(m_aoDistance))
            setAoDistance(defaultAoDistance());
    }

    emit aoEnabledChanged();
    update();
}

/*!
    \qmlproperty QtQuick3D::Fog QtQuick3D::SceneEnvironment::fog
    \since 6.5

    When this property is set to a valid \l {QtQuick3D::Fog}{Fog} object, it is
    used to configure the renderer's built-in fog support.

    The default value is null, which means no fog. This is equivalent to
    setting a Fog object with \l{Fog::enabled}{enabled} set to false.

    \image fog.jpg {Scene with fog effect}

    \sa {QtQuick3D::Fog}{Fog}
 */

QQuick3DFog *QQuick3DSceneEnvironment::fog() const
{
    return m_fog;
}

void QQuick3DSceneEnvironment::setFog(QQuick3DFog *fog)
{
    if (m_fog == fog)
        return;

    if (m_fog)
        m_fog->disconnect(m_fogSignalConnection);

    m_fog = fog;

    m_fogSignalConnection = QObject::connect(m_fog, &QQuick3DFog::changed, this, [this] { update(); });

    QObject::connect(m_fog, &QObject::destroyed, this,
                     [this](QObject *obj)
    {
        if (m_fog == obj) {
            m_fog = nullptr;
            update();
        }
    });

    emit fogChanged();
    update();
}

/*!
    \qmlproperty enumeration QtQuick3D::SceneEnvironment::oitMethod
    \since 6.9

    This property holds the Order Independent Transparency method. Setting this
    property to a valid OIT method causes the renderer to render transparent pixels
    in the correct depth order.

    Possible values are:
    \value SceneEnvironment.OITNone OIT is turned off.
    \value SceneEnvironment.OITWeightedBlended Approximates order independent transparency.
    \value SceneEnvironment.OITLinkedList Accurate order independent transparency.

    The default is \c None and order independent transparency is turned off.

    \b Weighted Blended

    This is an approximation of order independent transparency. The transparent fragments are
    rendered to an offscreen buffer while weighting is applied to each fragment based on its
    distance to the camera. This buffer is then blended to the backbuffer after de-weighting
    each pixel. This method doesn't follow the source-over composition rule for all fragments
    and the result is different from the correct result, however this method works also on
    older hardware and is faster than the other more rigorous methods.

    \b Linked List

    This is an accurate order independent transparency. The transparent fragments are
    rendered to a linked list in the render pass and sorted by their depth and blended
    in the composition pass.
    \since 6.11

    \note OIT might not work with MSAA on devices with GLES 3.1 or lower. It is recommended
    not to use MSAA if oit is wanted on such devices.

    \note Order independent transparenty is only applicaple to source-over blend mode.
    If scene contains objects with other blending modes, OIT is disabled.
*/

QQuick3DSceneEnvironment::QQuick3DEnvironmentOITMethod QQuick3DSceneEnvironment::oitMethod() const
{
    return m_oitMethod;
}

void QQuick3DSceneEnvironment::setOitMethod(QQuick3DSceneEnvironment::QQuick3DEnvironmentOITMethod method)
{
    if (m_oitMethod == method)
        return;
    m_oitMethod = method;
    emit oitMethodChanged();
    update();
}

QT_END_NAMESPACE
