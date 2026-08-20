// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcubemaptexture_p.h"
#include "qquick3dobject_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CubeMapTexture
    \inherits Texture
    \inqmlmodule QtQuick3D
    \brief Defines a cube map texture for use in 3D scenes.

    CubeMapTexture is a Texture that represents a cube map texture. A cube map
    texture has 6 faces (X+, X-, Y+, Y-, Z+, Z-), where each face is an
    individual 2D image. CubeMapTexture allows \l{CustomMaterial}{custom
    materials} and \l{Effect}{post-processing effects} to work with cube map
    textures in their shaders. A cube map can also be used to define the scene
    environment's \l{SceneEnvironment::skyBoxCubeMap}{skybox}.

    \qml
    CustomMaterial {
        property TextureInput customTexture: TextureInput {
            texture: CubeMapTexture {
                source: "cubemap.ktx"
            }
        }
        fragmentShader: "shader.frag"
    }
    \endqml

    Here shader.frag can be implemented assuming \c customTexture is sampler
    uniform with the GLSL type a \c samplerCube. This means that the
    \c{texture()} GLSL function takes a \c vec3 as the texture coordinate for
    that sampler. If we used \l Texture, the type would have been \c sampler2D.

    \badcode
    void MAIN()
    {
        vec4 c = texture(customTexture, NORMAL);
        BASE_COLOR = vec4(c.rgb, 1.0);
    }
    \endcode

    Sourcing a Texture from a container with a cubemap only loads face 0 (X+)
    and results in a 2D texture. Whereas sourcing a CubeMapTexture from the
    same asset loads all 6 faces and results in a cubemap texture.

    CubeMapTexture inherits all its properties from Texture. The important
    difference is that \l {Texture::}{source} must refer to a image file
    containing a cubemap, or to a list of image files. In practice a single
    file means a \l{https://www.khronos.org/ktx/}{KTX} container containing 6
    face images.

    Sourcing a CubeMapTexture from 6 individual images can be done in two
    different ways. Either as a semicolon-separated list of file names in
    X+, X-, Y+, Y-, Z+, Z- order:
    \qml
    CubeMapTexture {
        source: "maps/right.jpg;maps/left.jpg;maps/top.jpg;maps/bottom.jpg;maps/front.jpg;maps/back.jpg"
    }
    \endqml
    or as a string containing a "%p" placeholder, where "%p" will be replaced by the strings
    "posx", "negx", "posy", "negy", "posz", and "negz" to generate the six filenames:
    \qml
    CubeMapTexture {
        source: "maps/sky_%p.png"
        // equivalent to:
        // source: "maps/sky_posx.png;maps/sky_negx.png;maps/sky_posy.png;maps/sky_negy.png;maps/sky_posz.png;maps/sky_negz.png"
    }
    \endqml

    \note Sourcing image data via other means, such as \l {Texture::}{sourceItem}
    or \l {Texture::}{textureData} is not supported for CubeMapTexture at the
    moment.

    \sa Texture, CustomMaterial, Effect
*/

QQuick3DCubeMapTexture::QQuick3DCubeMapTexture(QQuick3DObject *parent)
    : QQuick3DTexture(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::ImageCube)), parent)
{
}

QQuick3DCubeMapTexture::~QQuick3DCubeMapTexture()
{
}

QT_END_NAMESPACE
