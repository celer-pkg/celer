// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dtexture_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderloadedtexture_p.h>
#include <QtQml/QQmlFile>
#include <QtQuick/QQuickItem>
#include <QtQuick/private/qquickitem_p.h>
#include <QSGTextureProvider>
#include <QtCore/qmath.h>

#include <ssg/qssgrenderextensions.h>

#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dutils_p.h"
#include "qquick3drenderextensions.h"
#include "qquick3dviewport_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Texture
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Defines a texture for use in 3D scenes.

    A texture is technically any array of pixels (1D, 2D or 3D) and its related
    settings, such as minification and magnification filters, scaling and UV
    transformations.

    The Texture type in Qt Quick 3D represents a two-dimensional image. Its use
    is typically to map onto / wrap around three-dimensional geometry to emulate
    additional detail which cannot be efficiently modelled in 3D. It can also be
    used to emulate other lighting effects, such as reflections.

    While Texture itself always represents a 2D texture, other kinds of
    textures are available as well via subclasses of Texture. For example, to
    create a cube map texture with 6 faces, use the \l CubeMapTexture type.

    When the geometry is being rendered, each location on its surface will be
    transformed to a corresponding location in the texture by transforming and
    interpolating the UV coordinates (texture coordinate) that have been set for
    the mesh's vertexes. The fragment shader program that is being used to render
    the active material will then typically sample the material's texture(s) at
    the given coordinates and use the sampled data in its light calculations.

    \note A Material may use multiple textures to give the desired interaction with
    light in the 3D scene. It can represent the color of each texel on the geometry
    surface, but also other attributes of the surface. For instance, a "normal map"
    can represent the deviation from the geometry normals for each texel on the
    surface, emulating light interaction with finer details on the surface, such
    as cracks or bumps. See \l{Qt Quick 3D - Principled Material Example}{the principled
    material example} for a demonstration of a material with multiple texture maps.

    Texture objects can source image data from:
    \list
    \li an image or texture file by using the \l source property,
    \li a Qt Quick \l Item by using the sourceItem property,
    \li or by setting the \l textureData property to a \l TextureData item
    subclass for defining the custom texture contents.
    \endlist

    The following example maps the image "madewithqt.png" onto the default sphere
    mesh, and scales the UV coordinates to tile the image on the sphere surface.
    \qml
    Model {
        source: "#Sphere"
        materials: [ PrincipledMaterial {
                baseColorMap: Texture {
                    source: "madewithqt.png"
                    scaleU: 4.0
                    scaleV: 4.0
                }
            }
        ]
    }
    \endqml

    The result looks as follows:
    \table
    \header
    \li Original image
    \li Mapped onto a sphere
    \row
    \li \image madewithqt.png {Built with Qt logo}
    \li \image spheremap.png {Sphere with Qt logo as texture map}
    \endtable

    \sa {Qt Quick 3D - Procedural Texture Example}
*/

QQuick3DTexture::QQuick3DTexture(QQuick3DObject *parent)
    : QQuick3DTexture(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::Image2D, QQuick3DObjectPrivate::Flags::RequiresSecondaryUpdate)), parent)
{
}

QQuick3DTexture::QQuick3DTexture(QQuick3DObjectPrivate &dd, QQuick3DObject *parent)
    : QQuick3DObject(dd, parent)
{
}

QQuick3DTexture::~QQuick3DTexture()
{
    if (m_layer) {
        if (m_sceneManagerForLayer)
            m_sceneManagerForLayer->qsgDynamicTextures.removeAll(m_layer);
        m_layer->deleteLater(); // uhh...
    }

    if (m_sourceItem) {
        QQuickItemPrivate *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
        sourcePrivate->removeItemChangeListener(this, QQuickItemPrivate::Geometry);
    }
}

/*!
    \qmlproperty url QtQuick3D::Texture::source

    This property holds the location of an image or texture file containing the data used by the
    texture.

    The property is a URL, with the same rules as other source properties, such
    as \l{Image::source}{Image.source}. With Texture, only the \c qrc and \c
    file schemes are supported. When no scheme is present and the value is a
    relative path, it is assumed to be relative to the component's (i.e. the
    \c{.qml} file's) location.

    The source file can have any conventional image file format
    \l{QImageReader::supportedImageFormats()}{supported by Qt}. In addition, Texture supports the
    same \l [QtQuick]{Compressed Texture Files}{compressed texture file types} as QtQuick::Image.

    \note Texture data read from image files such as .png or .jpg involves
    storing the rows of pixels within the texture in an order defined the Qt
    Quick 3D rendering engine. When the source file is a container for -
    possibly compressed - texture data, such transformations cannot happen on
    the pixel data level. Examples of this are .ktx or .pkm files. Instead, the
    Texture implicitly enables vertical flipping in the fragment shader code in
    order to get identical on-screen results. This is controlled by the \l
    autoOrientation property and can be disabled, if desired.

    \note Some texture compression tools may apply automatic vertical mirroring
    (flipping) on the image data. In modern tools this is often an opt-in
    setting. It is important to be aware of the settings used in the asset
    conditioning pipeline, because an unexpectedly flipped texture, and thus
    incorrect texturing of objects, can have its root cause in the asset
    itself, outside the application's and rendering engine's control. When the
    asset requires it, applications can always set the \l flipV property
    themselves.

    \sa sourceItem, textureData, autoOrientation, flipV
*/
QUrl QQuick3DTexture::source() const
{
    return m_source;
}

/*!
    \qmlproperty Item QtQuick3D::Texture::sourceItem

    This property defines a Item to be used as the source of the texture. Using
    this property allows any 2D Qt Quick content to be used as a texture source
    by rendering that item as an offscreen layer.

    If the item is a \l{QQuickItem::textureProvider()}{texture provider}, no
    additional texture is used.

    If this property is set, then the value of \l source will be ignored. A
    Texture should use one method to provide image data, and set only one of
    source, \l sourceItem, or \l textureData.

    \note Currently input events are forwarded to the Item used as a texture
    source only if the user is limited to interacting with one sourceItem
    instance at a time. In other words: you can share the same Item between
    multiple Textures, but then you cannot have multi-touch interaction with
    the same item on multiple textures at the same time. So it's best to use a
    separate 2D subscene instance for each Texture instance, if you expect to
    manipulate interactive items inside.

    \note Using this property in a Texture that is referenced from multiple
    windows is strongly discouraged. This includes usage via
    \l{View3D::importScene}. As the source texture created by this property is
    only accessible by one render thread, attempting to share it between
    multiple QQuickWindow instances is going to fail, unless the \c basic
    render loop of Qt Quick is used instead of the default \c threaded one. See
    \l{Qt Quick Scene Graph} on more information about the Qt Quick render
    loops.

    \note A Texture that contains the results of a Qt Quick offscreen render
    pass will in effect have an Y axis orientation that is different from what
    a Texture that receives its content via the source property uses. When used
    in combination with DefaultMaterial or PrincipledMaterial, this is all
    transparent to the application as the necessary UV transformations are
    applied automatically as long as the autoOrientation property is set to
    true, and so no further action is needed, regardless of how the texture was
    sourced. However, when developing \l{QtQuick3D::CustomMaterial}{custom
    materials} this needs to be kept in mind by the shader code author when
    sampling the texture and working with UV coordinates.

    \sa source, textureData, autoOrientation
*/
QQuickItem *QQuick3DTexture::sourceItem() const
{
    return m_sourceItem;
}

/*!
    \qmlproperty real QtQuick3D::Texture::scaleU

    This property defines how to scale the U texture coordinate when mapping to
    a mesh's UV coordinates.

    Scaling the U value when using horizontal tiling will define how many times the
    texture is repeated from left to right.

    The default is 1.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa tilingModeHorizontal
 */
float QQuick3DTexture::scaleU() const
{
    return m_scaleU;
}

/*!
    \qmlproperty real QtQuick3D::Texture::scaleV

    This property defines how to scale the V texture coordinate when mapping to
    a mesh's UV coordinates.

    Scaling the V value when using vertical tiling will define how many times a
    texture is repeated from bottom to top.

    The default is 1.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa tilingModeVertical
*/
float QQuick3DTexture::scaleV() const
{
    return m_scaleV;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::mappingMode

    This property defines which method of mapping to use when sampling this
    texture.

    \value Texture.UV The default value. Suitable for base color, diffuse,
    opacity, and most other texture maps. Performs standard UV mapping. The
    same portion of the image will always appear on the same vertex, unless the
    UV coordinates are transformed and animated.

    \value Texture.Environment Used for
    \l{PrincipledMaterial::specularReflectionMap}{specular reflection}, this
    causes the image to be projected onto the material as though it was being
    reflected. Using this mode for other type of texture maps provides a mirror
    effect.

    \value Texture.LightProbe The default for HDRI sphere maps used by light
    probes. This mode does not need to be manually set for Texture objects
    associated with the \l{SceneEnvironment::lightProbe}{lightProbe} property,
    because it is implied automatically.
*/
QQuick3DTexture::MappingMode QQuick3DTexture::mappingMode() const
{
    return m_mappingMode;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::tilingModeHorizontal

    Controls how the texture is mapped when the U scaling value is greater than 1.

    By default, this property is set to \c{Texture.Repeat}.

    \value Texture.ClampToEdge Texture is not tiled, but the value on the edge is used instead.
    \value Texture.MirroredRepeat Texture is repeated and mirrored over the X axis.
    \value Texture.Repeat Texture is repeated over the X axis.

    \sa scaleU
*/
QQuick3DTexture::TilingMode QQuick3DTexture::horizontalTiling() const
{
    return m_tilingModeHorizontal;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::tilingModeVertical

    This property controls how the texture is mapped when the V scaling value
    is greater than 1.

    By default, this property is set to \c{Texture.Repeat}.

    \value Texture.ClampToEdge Texture is not tiled, but the value on the edge is used instead.
    \value Texture.MirroredRepeat Texture is repeated and mirrored over the Y axis.
    \value Texture.Repeat Texture is repeated over the Y axis.

    \sa scaleV
*/
QQuick3DTexture::TilingMode QQuick3DTexture::verticalTiling() const
{
    return m_tilingModeVertical;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::tilingModeDepth

    This property controls how the texture is mapped when the Z scaling value
    is greater than 1.

    By default, this property is set to \c{Texture.Repeat}.

    \value Texture.ClampToEdge Texture is not tiled, but the value on the edge is used instead.
    \value Texture.MirroredRepeat Texture is repeated and mirrored over the Z axis.
    \value Texture.Repeat Texture is repeated over the Z axis.
*/
QQuick3DTexture::TilingMode QQuick3DTexture::depthTiling() const
{
    return m_tilingModeDepth;
}

/*!
    \qmlproperty real QtQuick3D::Texture::rotationUV

    This property rotates the texture around the pivot point. This is defined
    using euler angles and for a positive value rotation is clockwise.

    The default is 0.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa pivotU, pivotV
*/
float QQuick3DTexture::rotationUV() const
{
    return m_rotationUV;
}

/*!
    \qmlproperty real QtQuick3D::Texture::positionU

    This property offsets the U coordinate mapping from left to right.

    The default is 0.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa positionV
*/
float QQuick3DTexture::positionU() const
{
    return m_positionU;
}

/*!
    \qmlproperty real QtQuick3D::Texture::positionV

    This property offsets the V coordinate mapping from bottom to top.

    The default is 0.0.

    \note Qt Quick 3D uses OpenGL-style vertex data, regardless of the graphics
    API used at run time. The UV position \c{(0, 0)} is therefore referring to
    the bottom-left corner of the image data.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa positionU
*/
float QQuick3DTexture::positionV() const
{
    return m_positionV;
}

/*!
    \qmlproperty real QtQuick3D::Texture::pivotU

    This property sets the pivot U position which is used when applying a
    \l{QtQuick3D::Texture::rotationUV}{rotationUV}.

    The default is 0.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa rotationUV
*/
float QQuick3DTexture::pivotU() const
{
    return m_pivotU;
}

/*!
    \qmlproperty real QtQuick3D::Texture::pivotV

    This property sets the pivot V position which is used when applying a
    \l{QtQuick3D::Texture::rotationUV}{rotationUV}.

    The default is 0.0.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa pivotU, rotationUV
*/
float QQuick3DTexture::pivotV() const
{
    return m_pivotV;
}

/*!
    \qmlproperty bool QtQuick3D::Texture::flipU

    This property sets the use of the horizontally flipped texture coordinates.

    The default is false.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa flipV
*/
bool QQuick3DTexture::flipU() const
{
    return m_flipU;
}

/*!
    \qmlproperty bool QtQuick3D::Texture::flipV

    This property sets the use of the vertically flipped texture coordinates.

    The default is false.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \sa flipU
*/
bool QQuick3DTexture::flipV() const
{
    return m_flipV;
}

/*!
    \qmlproperty int QtQuick3D::Texture::indexUV

    This property sets the UV coordinate index used by this texture. Since
    QtQuick3D supports 2 UV sets(0 or 1) for now, the value will be saturated
    to the range.

    The default is 0.
*/
int QQuick3DTexture::indexUV() const
{
    return m_indexUV;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::magFilter

    This property determines how the texture is sampled when it is "magnified",
    i.e. a texel covers \e more than one pixel in screen space.

    The default value is \c{Texture.Linear}.

    \value Texture.Nearest uses the value of the closest texel.
    \value Texture.Linear takes the four closest texels and bilinearly interpolates them.

    \note Using \c Texture.None here will default to \c Texture.Linear instead.

    \sa minFilter, mipFilter
*/
QQuick3DTexture::Filter QQuick3DTexture::magFilter() const
{
    return m_magFilter;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::minFilter

    This property determines how the texture is sampled when it is "minimized",
    i.e. a texel covers \e less than one pixel in screen space.

    The default value is \c{Texture.Linear}.

    \value Texture.Nearest uses the value of the closest texel.
    \value Texture.Linear takes the four closest texels and bilinearly interpolates them.

    \note Using \c Texture.None here will default to \c Texture.Linear instead.

    \sa magFilter, mipFilter
*/
QQuick3DTexture::Filter QQuick3DTexture::minFilter() const
{
    return m_minFilter;
}

/*!
    \qmlproperty enumeration QtQuick3D::Texture::mipFilter

    This property determines how the texture mipmaps are sampled when a texel covers
    less than one pixel.

    The default value is \c{Texture.None}.

    \value Texture.None disables the usage of mipmap sampling.
    \value Texture.Nearest uses mipmapping and samples the value of the closest texel.
    \value Texture.Linear uses mipmapping and interpolates between multiple texel values.

    \note This property will have no effect on Textures that do not have mipmaps.

    \sa minFilter, magFilter
*/
QQuick3DTexture::Filter QQuick3DTexture::mipFilter() const
{
    return m_mipFilter;
}

/*!
    \qmlproperty TextureData QtQuick3D::Texture::textureData

    This property holds a reference to a \l TextureData component which
    defines the contents and properties of raw texture data.

    If this property is used, then the value of \l source will be ignored. A
    Texture should use one method to provide image data, and set only one of
    source, \l sourceItem, or \l textureData.

    \sa source, sourceItem, {Qt Quick 3D - Procedural Texture Example}
*/

QQuick3DTextureData *QQuick3DTexture::textureData() const
{
    return m_textureData;
}

/*!
    \qmlproperty bool QtQuick3D::Texture::generateMipmaps

    This property determines if mipmaps are generated for textures that
    do not provide mipmap levels themselves. Using mipmaps along with mip
    filtering gives better visual quality when viewing textures at a distance
    compared rendering without them, but it may come at a performance
    cost (both when initializing the image and during rendering).

    By default, this property is set to false.

    \note It is necessary to set a \l{QtQuick3D::Texture::mipFilter}{mipFilter} mode
    for the generated mipmaps to be be used.

    \note This property is not applicable when the texture content is based on
    a Qt Quick item referenced by the \l sourceItem property. Mipmap generation
    for dynamic textures is not feasible due to the performance implications.
    Therefore, the value of this property is ignored for such textures.

    \sa mipFilter
*/
bool QQuick3DTexture::generateMipmaps() const
{
    return m_generateMipmaps;
}

/*!
    \qmlproperty bool QtQuick3D::Texture::autoOrientation

    This property determines if a texture transformation, such as flipping the
    V texture coordinate, is applied automatically for textures where this is
    typically relevant.

    By default, this property is set to true.

    Certain type of texture data, such as compressed textures loaded via the \l
    source property from a .ktx or .pkm file, or textures generated by
    rendering a Qt Quick scene via the \l sourceItem property, often have a
    different Y axis orientation when compared to textures loaded from image
    files, such as, .png or .jpg. Therefore, such a Texture would appear
    "upside down" compared to a Texture with its source set to a regular image
    file. To remedy this, any qualifying Texture gets an implicit UV
    transformation as if the flipV property was set to true. If this is not
    desired, set this property to false.

    \note This property is effective when the Texture is used in combination
    with a DefaultMaterial or PrincipledMaterial.
    \l{QtQuick3D::CustomMaterial}{Custom materials} provide their own shader
    code, and so transformations such as the one configured by this property
    are ignored and are up to the application-provided shader code to
    implement.

    \since 6.2

    \sa flipV
*/

bool QQuick3DTexture::autoOrientation() const
{
    return m_autoOrientation;
}

/*!
    \qmlproperty RenderExtension QtQuick3D::Texture::textureProvider

    This property holds the RenderExtension that will provide the \l QRhiTexture
    that will be used by this item.

    \note The texture created by RenderExtension needs to be made available by
    \l{QSSGRenderExtensionHelpers::registerRenderResult}{registering} it with the engine.

    \since 6.7

    \sa TextureProviderExtension, RenderExtension, QSSGRenderExtensionHelpers
*/

QQuick3DRenderExtension *QQuick3DTexture::textureProvider() const
{
    return m_renderExtension;
}

void QQuick3DTexture::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
    m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty);
    emit sourceChanged();
    update();
}

void QQuick3DTexture::trySetSourceParent()
{
    if (m_sourceItem->parentItem() && m_sourceItemRefed)
        return;

    auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);

    if (!m_sourceItem->parentItem()) {
        if (const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager) {
            if (auto *window = manager->window()) {
                if (m_sourceItemRefed) {
                    // Item was already refed but probably with hide set to false...
                    // so we need to deref before we ref again below.
                    const bool hide = m_sourceItemReparented;
                    sourcePrivate->derefFromEffectItem(hide);
                    m_sourceItemRefed = false;
                }

                m_sourceItem->setParentItem(window->contentItem());
                m_sourceItemReparented = true;
                update();
            }
        }
    }

    if (!m_sourceItemRefed) {
        const bool hide = m_sourceItemReparented;
        sourcePrivate->refFromEffectItem(hide);
    }
}

void QQuick3DTexture::setSourceItem(QQuickItem *sourceItem)
{
    if (m_sourceItem == sourceItem)
        return;

    disconnect(m_textureProviderConnection);
    disconnect(m_textureUpdateConnection);

    if (m_sourceItem) {
        QQuickItemPrivate *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);

        const bool hide = m_sourceItemReparented;
        sourcePrivate->derefFromEffectItem(hide);
        m_sourceItemRefed = false;

        sourcePrivate->removeItemChangeListener(this, QQuickItemPrivate::Geometry);
        disconnect(m_sourceItem, SIGNAL(destroyed(QObject*)), this, SLOT(sourceItemDestroyed(QObject*)));
        disconnect(m_sourceItem, &QQuickItem::windowChanged, this, &QQuick3DTexture::sourceItemWindowChanged);
        if (m_sourceItemReparented) {
            m_sourceItem->setParentItem(nullptr);
            m_sourceItemReparented = false;
        }
    }

    m_sourceItem = sourceItem;

    if (sourceItem) {
        trySetSourceParent();
        QQuickItemPrivate *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
        sourcePrivate->addItemChangeListener(this, QQuickItemPrivate::Geometry);
        connect(m_sourceItem, SIGNAL(destroyed(QObject*)), this, SLOT(sourceItemDestroyed(QObject*)));
        connect(m_sourceItem, &QQuickItem::windowChanged, this, &QQuick3DTexture::sourceItemWindowChanged);
        sourcePrivate->ensureSubsceneDeliveryAgent();
    }

    if (m_layer) {
        const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager;
        manager->qsgDynamicTextures.removeAll(m_layer);
        m_sceneManagerForLayer = nullptr;
        // cannot touch m_layer here
    }
    m_initializedSourceItem = nullptr;
    m_initializedSourceItemSize = QSize();

    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
    m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty);
    emit sourceItemChanged();
    update();
}

void QQuick3DTexture::setScaleU(float scaleU)
{
    if (qFuzzyCompare(m_scaleU, scaleU))
        return;

    m_scaleU = scaleU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit scaleUChanged();
    update();
}

void QQuick3DTexture::setScaleV(float scaleV)
{
    if (qFuzzyCompare(m_scaleV, scaleV))
        return;

    m_scaleV = scaleV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit scaleVChanged();
    update();
}

void QQuick3DTexture::setMappingMode(QQuick3DTexture::MappingMode mappingMode)
{
    if (m_mappingMode == mappingMode)
        return;

    m_mappingMode = mappingMode;
    emit mappingModeChanged();
    update();
}

void QQuick3DTexture::setHorizontalTiling(QQuick3DTexture::TilingMode tilingModeHorizontal)
{
    if (m_tilingModeHorizontal == tilingModeHorizontal)
        return;

    m_tilingModeHorizontal = tilingModeHorizontal;
    emit horizontalTilingChanged();
    update();
}

void QQuick3DTexture::setVerticalTiling(QQuick3DTexture::TilingMode tilingModeVertical)
{
    if (m_tilingModeVertical == tilingModeVertical)
        return;

    m_tilingModeVertical = tilingModeVertical;
    emit verticalTilingChanged();
    update();
}

void QQuick3DTexture::setDepthTiling(QQuick3DTexture::TilingMode tilingModeDepth)
{
    if (m_tilingModeDepth == tilingModeDepth)
        return;
    m_tilingModeDepth = tilingModeDepth;
    emit depthTilingChanged();
    update();
}

void QQuick3DTexture::setRotationUV(float rotationUV)
{
    if (qFuzzyCompare(m_rotationUV, rotationUV))
        return;

    m_rotationUV = rotationUV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit rotationUVChanged();
    update();
}

void QQuick3DTexture::setPositionU(float positionU)
{
    if (qFuzzyCompare(m_positionU, positionU))
        return;

    m_positionU = positionU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit positionUChanged();
    update();
}

void QQuick3DTexture::setPositionV(float positionV)
{
    if (qFuzzyCompare(m_positionV, positionV))
        return;

    m_positionV = positionV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit positionVChanged();
    update();
}

void QQuick3DTexture::setPivotU(float pivotU)
{
    if (qFuzzyCompare(m_pivotU, pivotU))
        return;

    m_pivotU = pivotU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit pivotUChanged();
    update();
}

void QQuick3DTexture::setPivotV(float pivotV)
{
    if (qFuzzyCompare(m_pivotV, pivotV))
        return;

    m_pivotV = pivotV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit pivotVChanged();
    update();
}

void QQuick3DTexture::setFlipU(bool flipU)
{
    if (m_flipU == flipU)
        return;

    m_flipU = flipU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit flipUChanged();
    update();
}

void QQuick3DTexture::setFlipV(bool flipV)
{
    if (m_flipV == flipV)
        return;

    m_flipV = flipV;
    m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty);
    emit flipVChanged();
    update();
}

void QQuick3DTexture::setIndexUV(int indexUV)
{
    if (m_indexUV == indexUV)
        return;

    if (indexUV < 0)
        m_indexUV = 0;
    else if (indexUV > 1)
        m_indexUV = 1;
    else
        m_indexUV = indexUV;

    m_dirtyFlags.setFlag(DirtyFlag::IndexUVDirty);
    emit indexUVChanged();
    update();
}

void QQuick3DTexture::setTextureData(QQuick3DTextureData *textureData)
{
    if (m_textureData == textureData)
        return;

    // Make sure to disconnect if the geometry gets deleted out from under us
    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DTexture::setTextureData, textureData, m_textureData);

    if (m_textureData)
        QObject::disconnect(m_textureDataConnection);
    m_textureData = textureData;

    if (m_textureData) {
        m_textureDataConnection
                = QObject::connect(m_textureData, &QQuick3DTextureData::textureDataNodeDirty, this, [this]() {
            markDirty(DirtyFlag::TextureDataDirty);
        });
    }

    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
    m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty);
    emit textureDataChanged();
    update();
}

void QQuick3DTexture::setGenerateMipmaps(bool generateMipmaps)
{
    if (m_generateMipmaps == generateMipmaps)
        return;

    m_generateMipmaps = generateMipmaps;
    m_dirtyFlags.setFlag(DirtyFlag::SamplerDirty);
    emit generateMipmapsChanged();
    update();
}

void QQuick3DTexture::setAutoOrientation(bool autoOrientation)
{
    if (m_autoOrientation == autoOrientation)
        return;

    m_autoOrientation = autoOrientation;
    m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty);
    emit autoOrientationChanged();
    update();
}

void QQuick3DTexture::setMagFilter(QQuick3DTexture::Filter magFilter)
{
    if (m_magFilter == magFilter)
        return;

    m_magFilter = magFilter;
    m_dirtyFlags.setFlag(DirtyFlag::SamplerDirty);
    emit magFilterChanged();
    update();
}

void QQuick3DTexture::setMinFilter(QQuick3DTexture::Filter minFilter)
{
    if (m_minFilter == minFilter)
        return;

    m_minFilter = minFilter;
    m_dirtyFlags.setFlag(DirtyFlag::SamplerDirty);
    emit minFilterChanged();
    update();
}

void QQuick3DTexture::setMipFilter(QQuick3DTexture::Filter mipFilter)
{
    if (m_mipFilter == mipFilter)
        return;

    m_mipFilter = mipFilter;
    m_dirtyFlags.setFlag(DirtyFlag::SamplerDirty);
    emit mipFilterChanged();
    update();
}

// this function may involve file system access and hence can be expensive
bool QQuick3DTexture::effectiveFlipV(const QSSGRenderImage &imageNode) const
{
    // No magic when autoOrientation is false.
    if (!m_autoOrientation)
        return m_flipV;

    // Keep the same order as in QSSGBufferManager: sourceItem > textureData > source

    // Using sourceItem implies inverting (the effective, internal) flipV,
    // transparently to the user. Otherwise two #Rectangle models textured with
    // two Textures where one has its content loaded from an image file via
    // QImage while the other is generated by Qt Quick rendering into the
    // texture would appear upside-down relative to each other, and that
    // discrepancy is not ideal. (that said, this won't help CustomMaterial, as
    // documented for flipV and co.)

    if (m_sourceItem)
        return !m_flipV;

    // With textureData and renderExtension we assume the application knows what it is doing,
    // because there the application is controlling the content itself.

    if (m_textureData || m_renderExtension)
        return m_flipV;

    // Compressed textures (or any texture that is coming from the associated
    // container formats, such as KTX, i.e. not via QImage but through
    // QTextureFileReader) get the implicit flip, like sourceItem. This is done
    // mainly for parity with Qt Quick's Image, see QTBUG-93972.

    if (!m_source.isEmpty()) {
        const QString filePath = imageNode.m_imagePath.path();
        if (!filePath.isEmpty()) {
            QSSGInputUtil::FileType fileType = QSSGInputUtil::UnknownFile;
            if (QSSGInputUtil::getStreamForTextureFile(filePath, true, nullptr, &fileType)) {
                if (fileType == QSSGInputUtil::TextureFile)
                    return !m_flipV;
            }
        }
    }

    return m_flipV;
}

static QSSGRenderPath resolveImagePath(const QUrl &url, const QQmlContext *context)
{
    if (context && url.isRelative()) {
        QString path = url.path();
        QChar separator = QChar::fromLatin1(';');
        if (path.contains(separator)) {
            QString resolvedPath;
            const QStringList paths = path.split(separator);
            bool first = true;
            for (auto &s : paths) {
                auto mapped =  QQmlFile::urlToLocalFileOrQrc(context->resolvedUrl(QUrl{s}));
                if (!first)
                    resolvedPath.append(separator);
                resolvedPath.append(mapped);
                first = false;
            }
            return QSSGRenderPath(resolvedPath);
        }
    }
    return QSSGRenderPath(QQmlFile::urlToLocalFileOrQrc(context ? context->resolvedUrl(url) : url));
}

QSSGRenderGraphObject *QQuick3DTexture::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderImage(QQuick3DObjectPrivate::get(this)->type);
    }
    QQuick3DObject::updateSpatialNode(node);
    auto imageNode = static_cast<QSSGRenderImage *>(node);

    if (m_dirtyFlags.testFlag(DirtyFlag::TransformDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::TransformDirty, false);

        // flipV and indexUV have their own dirty flags, handled separately below
        imageNode->m_flipU = m_flipU;
        imageNode->m_scale = QVector2D(m_scaleU, m_scaleV);
        imageNode->m_pivot = QVector2D(m_pivotU, m_pivotV);
        imageNode->m_rotation = m_rotationUV;
        imageNode->m_position = QVector2D(m_positionU, m_positionV);

        imageNode->m_flags.setFlag(QSSGRenderImage::Flag::TransformDirty);
    }

    bool nodeChanged = false;
    if (m_dirtyFlags.testFlag(DirtyFlag::SourceDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::SourceDirty, false);
        m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty, true);
        if (!m_source.isEmpty()) {
            const QQmlContext *context = qmlContext(this);
            imageNode->m_imagePath = resolveImagePath(m_source, context);
        } else {
            imageNode->m_imagePath = QSSGRenderPath();
        }
        nodeChanged = true;
    }
    if (m_dirtyFlags.testFlag(DirtyFlag::IndexUVDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::IndexUVDirty, false);
        imageNode->m_indexUV = m_indexUV;
    }
    nodeChanged |= qUpdateIfNeeded(imageNode->m_mappingMode,
                                  QSSGRenderImage::MappingModes(m_mappingMode));
    nodeChanged |= qUpdateIfNeeded(imageNode->m_horizontalTilingMode,
                                  QSSGRenderTextureCoordOp(m_tilingModeHorizontal));
    nodeChanged |= qUpdateIfNeeded(imageNode->m_verticalTilingMode,
                                  QSSGRenderTextureCoordOp(m_tilingModeVertical));
    nodeChanged |= qUpdateIfNeeded(imageNode->m_depthTilingMode,
                                  QSSGRenderTextureCoordOp(m_tilingModeDepth));

    if (m_dirtyFlags.testFlag(DirtyFlag::SamplerDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::SamplerDirty, false);
        nodeChanged |= qUpdateIfNeeded(imageNode->m_minFilterType,
                                       QSSGRenderTextureFilterOp(m_minFilter));
        nodeChanged |= qUpdateIfNeeded(imageNode->m_magFilterType,
                                       QSSGRenderTextureFilterOp(m_magFilter));
        nodeChanged |= qUpdateIfNeeded(imageNode->m_mipFilterType,
                                       QSSGRenderTextureFilterOp(m_mipFilter));
        nodeChanged |= qUpdateIfNeeded(imageNode->m_generateMipmaps,
                                       m_generateMipmaps);
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::TextureDataDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty, false);
        m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty, true);
        if (m_textureData)
            imageNode->m_rawTextureData = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::get(m_textureData)->spatialNode);
        else
            imageNode->m_rawTextureData = nullptr;
        nodeChanged = true;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ExtensionDirty)) {
        bool extDirty = false;
        if (m_renderExtension) {
            auto *sn = QQuick3DObjectPrivate::get(m_renderExtension)->spatialNode;
            // NOTE: We don't clear if we haven't gotten the spatial node yet, as
            // we'll be called once _again_ when the extensions have been processed.
            extDirty = (sn == nullptr);
            if (sn && QSSG_GUARD(QSSGRenderGraphObject::isExtension(sn->type)))
                imageNode->m_extensionsSource = static_cast<QSSGRenderExtension *>(sn);
        }

        if (extDirty)
            markDirty(DirtyFlag::ExtensionDirty, true);
        m_dirtyFlags.setFlag(DirtyFlag::ExtensionDirty, extDirty);
        m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty, true);

        nodeChanged = true;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::SourceItemDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty, false);
        m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty, true);
        if (m_sourceItem) {
            QQuickWindow *window = m_sourceItem->window();
            // If it was an inline declared item (very common, e.g. Texture {
            // sourceItem: Rectangle { ... } } then it is likely it won't be
            // associated with a window (Qt Quick scene) unless we help it to
            // one via refWindow. However, this here is only the last resort,
            // ideally there is a refWindow upon ItemSceneChange already.
             if (!window) {
                window = QQuick3DObjectPrivate::get(this)->sceneManager->window();
                if (window)
                    QQuickItemPrivate::get(m_sourceItem)->refWindow(window);
                else
                    qWarning("Unable to get window, this will probably not work");
            }

            // Workaround: Due to limitation in how the texture provider works in View3D we can't use it in the
            // special case when a non-visible View3D is used as a sourceItem, at least not in its current incarnation.
            // There's also a drawback here, since we now have an extra indirection by rendering to the layer texture first..
            const bool isHiddenView3D = !m_sourceItem->isVisible() && (qobject_cast<QQuick3DViewport *>(m_sourceItem) != nullptr);

            // This assumes that the QSGTextureProvider returned never changes,
            // which is hopefully the case for both Image and Item layers.
            if (QSGTextureProvider *provider = m_sourceItem->textureProvider(); provider != nullptr && !isHiddenView3D) {
                imageNode->m_qsgTexture = provider->texture();

                disconnect(m_textureProviderConnection);
                m_textureProviderConnection = connect(provider, &QSGTextureProvider::textureChanged, this, [this, provider] () {
                    // called on the render thread, if there is one; the gui
                    // thread may or may not be blocked (e.g. if the source is
                    // a View3D, that emits textureChanged() from preprocess,
                    // so after sync, whereas an Image emits in
                    // updatePaintNode() where gui is blocked)
                    auto imageNode = static_cast<QSSGRenderImage *>(QQuick3DObjectPrivate::get(this)->spatialNode);
                    if (!imageNode)
                        return;

                    imageNode->m_qsgTexture = provider->texture();
                    // the QSGTexture may be different now, go through loadRenderImage() again
                    imageNode->m_flags.setFlag(QSSGRenderImage::Flag::Dirty);
                    // Call update() on the main thread - otherwise we could
                    // end up in a situation where the 3D scene does not update
                    // due to nothing else changing, even though the source
                    // texture is now different.
                    QMetaObject::invokeMethod(this, &QQuick3DTexture::update, Qt::AutoConnection);
                }, Qt::DirectConnection);

                disconnect(m_textureUpdateConnection);
                auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
                if (sourcePrivate->window) {
                    QQuickItem *sourceItem = m_sourceItem; // for capturing, recognizing in the lambda that m_sourceItem has changed is essential

                    // Why after, not beforeSynchronizing? Consider the case of an Item layer:
                    // if the View3D gets to sync (updatePaintNode) first, doing an
                    // updateTexture() is futile, the QSGLayer is not yet initialized (not
                    // associated with an Item, has no size, etc.). That happens only once the
                    // underlying QQuickShaderEffectSource hits its updatePaintNode. And that
                    // may well happen happen only after the View3D has finished with its sync
                    // step. By connecting to afterSynchronizing, we still get a chance to
                    // trigger a layer texture update and so have a QSGTexture with real
                    // content ready by the time the View3D prepares/renders the 3D scene upon
                    // the scenegraph's preprocess step (Offscreen) or before/after the
                    // scenegraph rendering (if Underlay/Overlay).
                    //
                    // This eliminates, or in the worst case reduces, the ugly effects of not
                    // having a texture ready when rendering the 3D scene.

                    m_textureUpdateConnection = connect(sourcePrivate->window, &QQuickWindow::afterSynchronizing, this, [this, sourceItem]() {
                        // Called on the render thread with gui blocked (if there is a render thread, that is).
                        if (m_sourceItem != sourceItem) {
                            disconnect(m_textureProviderConnection);
                            disconnect(m_textureUpdateConnection);
                            return;
                        }
                        auto imageNode = static_cast<QSSGRenderImage *>(QQuick3DObjectPrivate::get(this)->spatialNode);
                        if (!imageNode)
                            return;

                        if (QSGDynamicTexture *t = qobject_cast<QSGDynamicTexture *>(imageNode->m_qsgTexture)) {
                            if (t->updateTexture())
                                update(); // safe because the gui thread is blocked
                        }
                    }, Qt::DirectConnection);
                } else {
                    qWarning("No window for item, texture updates are doomed");
                }

                if (m_layer) {
                    delete m_layer;
                    m_layer = nullptr;
                }
            } else {
                // Not a texture provider, so not an Image or an Item with
                // layer.enabled: true, create our own QSGLayer.
                if (m_initializedSourceItem != m_sourceItem || m_initializedSourceItemSize != m_sourceItem->size()) {
                    // If there was a previous sourceItem and m_layer is valid
                    // then set its content to null until we get to
                    // afterSynchronizing, otherwise things can blow up.
                    if (m_layer)
                        m_layer->setItem(nullptr);

                    m_initializedSourceItem = m_sourceItem;
                    m_initializedSourceItemSize = m_sourceItem->size();

                    // The earliest next point where we can do anything is
                    // after the scenegraph's QQuickItem sync round has completed.
                    connect(window, &QQuickWindow::afterSynchronizing, this, [this, window]() {
                        auto imageNode = static_cast<QSSGRenderImage *>(QQuick3DObjectPrivate::get(this)->spatialNode);
                        if (!imageNode)
                            return;

                        // Called on the render thread with gui blocked (if there is a render thread, that is).
                        disconnect(window, &QQuickWindow::afterSynchronizing, this, nullptr);
                        if (m_layer) {
                            const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager;
                            manager->qsgDynamicTextures.removeAll(m_layer);
                            delete m_layer;
                            m_layer = nullptr;
                        }

                        QQuickItemPrivate *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
                        QSGRenderContext *rc = sourcePrivate->sceneGraphRenderContext();
                        Q_ASSERT(QThread::currentThread() == rc->thread()); // must be on the render thread
                        QSGLayer *layer = rc->sceneGraphContext()->createLayer(rc);
                        connect(sourcePrivate->window, SIGNAL(sceneGraphInvalidated()), layer, SLOT(invalidated()), Qt::DirectConnection);

                        QQuick3DSceneManager *manager = QQuick3DObjectPrivate::get(this)->sceneManager;
                        manager->qsgDynamicTextures << layer;
                        m_sceneManagerForLayer = manager;

                        connect(layer, &QObject::destroyed, manager, [manager, layer]()
                        {
                            // this is on the render thread so all borked threading-wise (all data here is gui thread stuff...) but will survive
                            manager->qsgDynamicTextures.removeAll(layer);
                        }, Qt::DirectConnection);

                        QQuickItem *sourceItem = m_sourceItem; // for capturing, recognizing in the lambda that m_sourceItem has changed is essential
                        connect(layer, &QObject::destroyed, this, [this, sourceItem]()
                        {
                            // just as dubious as the previous connection
                            if (m_initializedSourceItem == sourceItem) {
                                m_sceneManagerForLayer = nullptr;
                                m_initializedSourceItem = nullptr;
                            }
                        }, Qt::DirectConnection);

                        // With every frame try to update the texture. Use
                        // afterSynchronizing like in the other branch. (why
                        // after: a property changing something in the 2D
                        // subtree leading to updates in the content will only
                        // be "visible" after the (2D item) sync, not before)
                        //
                        // If updateTexture() returns false, content hasn't
                        // changed. This complements qsgDynamicTextures and
                        // QQuick3DViewport::updateDynamicTextures().
                        m_textureUpdateConnection = connect(sourcePrivate->window, &QQuickWindow::afterSynchronizing,
                                                            this, [this, sourceItem]()
                        {
                            // Called on the render thread with gui blocked (if there is a render thread, that is).
                            if (!m_layer)
                                return;
                            if (m_sourceItem != sourceItem) {
                                disconnect(m_textureUpdateConnection);
                                return;
                            }
                            if (m_layer->updateTexture())
                                update();
                        }, Qt::DirectConnection);

                        m_layer = layer;
                        m_layer->setItem(QQuickItemPrivate::get(m_sourceItem)->itemNode());

                        QRectF sourceRect = QRectF(0, 0, m_sourceItem->width(), m_sourceItem->height());
                        if (qFuzzyIsNull(sourceRect.width()))
                            sourceRect.setWidth(256);
                        if (qFuzzyIsNull(sourceRect.height()))
                            sourceRect.setHeight(256);
                        m_layer->setRect(sourceRect);

                        QSize textureSize(qCeil(qAbs(sourceRect.width())), qCeil(qAbs(sourceRect.height())));
                        const QSize minTextureSize = sourcePrivate->sceneGraphContext()->minimumFBOSize();
                        while (textureSize.width() < minTextureSize.width())
                            textureSize.rwidth() *= 2;
                        while (textureSize.height() < minTextureSize.height())
                            textureSize.rheight() *= 2;
                        m_layer->setSize(textureSize);

                        // now that the layer has an item and a size, it can render into the texture
                        m_layer->updateTexture();

                        imageNode->m_qsgTexture = m_layer;
                        imageNode->m_flags.setFlag(QSSGRenderImage::Flag::Dirty);
                    }, Qt::DirectConnection);
                }
            }
        } else {
            if (m_layer) {
                m_layer->setItem(nullptr);
                delete m_layer;
                m_layer = nullptr;
            }
            imageNode->m_qsgTexture = nullptr;
        }
        nodeChanged = true;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::FlipVDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::FlipVDirty, false);
        imageNode->m_flipV = effectiveFlipV(*imageNode);
        imageNode->m_flags.setFlag(QSSGRenderImage::Flag::TransformDirty);
    }

    if (nodeChanged)
        imageNode->m_flags.setFlag(QSSGRenderImage::Flag::Dirty);

    return imageNode;
}

void QQuick3DTexture::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    QQuick3DObject::itemChange(change, value);
    if (change == QQuick3DObject::ItemChange::ItemSceneChange) {
        // Source item
        if (m_sourceItem) {
            disconnect(m_sceneManagerWindowChangeConnection);

            if (m_sceneManagerForLayer) {
                m_sceneManagerForLayer->qsgDynamicTextures.removeOne(m_layer);
                m_sceneManagerForLayer = nullptr;
            }
            trySetSourceParent();
            const auto &sceneManager = value.sceneManager;
            Q_ASSERT(QQuick3DObjectPrivate::get(this)->sceneManager == sceneManager);
            if (m_layer) {
                if (sceneManager)
                    sceneManager->qsgDynamicTextures << m_layer;
                m_sceneManagerForLayer = sceneManager;
            }

            // If m_sourceItem was an inline declared item (very common, e.g.
            // Texture { sourceItem: Rectangle { ... } } then it is highly
            // likely it won't be associated with a window (Qt Quick scene)
            // yet. Associate with one as soon as possible, do not leave it to
            // updateSpatialNode, because that, while safe, would defer
            // rendering into the texture to a future frame (adding a 2 frame
            // lag for the first rendering of the mesh textured with the 2D
            // item content), since a refWindow needs to be followed by a
            // scenegraph sync round to get QSGNodes created (updatePaintNode),
            // whereas updateSpatialNode is in the middle of a sync round, so
            // would need to wait for another one, etc.
            if (sceneManager && m_sourceItem && !m_sourceItem->window()) {
                if (sceneManager->window()) {
                    QQuickItemPrivate::get(m_sourceItem)->refWindow(sceneManager->window());
                } else {
                    m_sceneManagerWindowChangeConnection = connect(sceneManager, &QQuick3DSceneManager::windowChanged, this,
                                                                   [this, sceneManager]
                    {
                        if (m_sourceItem && !m_sourceItem->window() && sceneManager->window())
                            QQuickItemPrivate::get(m_sourceItem)->refWindow(sceneManager->window());
                    });
                }
            }
        }
        // TextureData
        if (m_textureData) {
            const auto &sceneManager = value.sceneManager;
            if (sceneManager)
                QQuick3DObjectPrivate::refSceneManager(m_textureData, *sceneManager);
            else
                QQuick3DObjectPrivate::derefSceneManager(m_textureData);
        }

        // Texture Providers
        if (m_renderExtension) {
            const auto &sceneManager = value.sceneManager;
            if (sceneManager)
                QQuick3DObjectPrivate::refSceneManager(m_renderExtension, *sceneManager);
            else
                QQuick3DObjectPrivate::derefSceneManager(m_renderExtension);
        }
    }
}

void QQuick3DTexture::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &geometry)
{
    Q_ASSERT(item == m_sourceItem);
    Q_UNUSED(item);
    Q_UNUSED(geometry);
    if (change.sizeChange()) {
        m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
        update();
    }
}

void QQuick3DTexture::sourceItemDestroyed(QObject *item)
{
    Q_ASSERT(item == m_sourceItem);
    Q_UNUSED(item);

    m_sourceItem = nullptr;

    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
    m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty);
    emit sourceItemChanged();
    update();
}

class QQuick3DLayerCleanupJob : public QRunnable
{
public:
    explicit QQuick3DLayerCleanupJob(QSGLayer *l)
        : layer(l)
    {}
    void run() final
    {
        delete layer;
    }

private:
    QSGLayer *layer = nullptr;
};

void QQuick3DTexture::sourceItemWindowChanged(QQuickWindow *window)
{
    if (m_layer != nullptr && window == nullptr) {
        // During teardown the sourceItem may lose its window first after
        // the 3D scene has been torn down, so we need to make sure we still
        // have a valid sceneManager to remove the layer from.
        if (const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager) {
            manager->qsgDynamicTextures.removeAll(m_layer);
            // Make sure we get set up again once updateSpatialNode is called next.
            m_initializedSourceItem = nullptr;
            m_initializedSourceItemSize = QSize();

            if (manager->window()) {
                manager->window()->scheduleRenderJob(new QQuick3DLayerCleanupJob(m_layer),
                                                     QQuickWindow::AfterSynchronizingStage);
                m_layer = nullptr;
            }
            markAllDirty();
            update();
        }
    } else {
        markAllDirty();
        update();
    }
}

void QQuick3DTexture::markDirty(QQuick3DTexture::DirtyFlag type, bool requestSecondaryUpdate)
{
    if (!m_dirtyFlags.testFlag(type)) {
        m_dirtyFlags.setFlag(type, true);
        update();
    }

    if (requestSecondaryUpdate)
        QQuick3DObjectPrivate::get(this)->requestSecondaryUpdate();
}

QSSGRenderImage *QQuick3DTexture::getRenderImage()
{
    QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(this);
    return static_cast<QSSGRenderImage *>(p->spatialNode);
}

void QQuick3DTexture::markAllDirty()
{
    m_dirtyFlags = DirtyFlags(0xFFFF);
    QQuick3DObject::markAllDirty();
}

void QQuick3DTexture::setTextureProvider(QQuick3DRenderExtension *textureProvider)
{
    if (m_renderExtension == textureProvider)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DTexture::setTextureProvider, textureProvider, m_renderExtension);

    m_renderExtension = textureProvider;

    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    m_dirtyFlags.setFlag(DirtyFlag::SourceItemDirty);
    m_dirtyFlags.setFlag(DirtyFlag::TextureDataDirty);
    m_dirtyFlags.setFlag(DirtyFlag::ExtensionDirty);

    emit textureProviderChanged();
    update();
}

QT_END_NAMESPACE
