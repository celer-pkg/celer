// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dbakedlightmap_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype BakedLightmap
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Specifies baked lightmap settings for a model.
    \since 6.4

    A BakedLightmap object can be used to enable:

    \list
    \li persistently storing the baked lightmap data - during baking, or
    \li loading the previously generated and stored lightmaps - at run time.
    \endlist

    A Model with \l{Model::usedInBakedLighting}{usedInBakedLighting} set to
    true is considered to be part of the raytraced scene when baking lightmaps,
    meaning the model's geometry and material contribute to direct and indirect
    lighting. This on its own does not however enable generating, including
    full calculation of bounced indirect lighting, and finally saving a
    lightmap for the model. To do that, the model also needs to be associated
    with an \l enabled BakedLightmap object with a unique key set.

    When running in normal mode, the same BakedLightmap object indicates that
    the Model has lightmap data, and that the engine should attempt to load
    this data (based on the unique key) and use it when rendering.

    For more information on how to bake lightmaps, see the \l Lightmapper
    documentation.

    \note As of Qt 6.4, lightmap baking is in an early technology preview state.
    Changes to features, quality, and API are likely happen in future releases.

    \sa Lightmapper, Model::usedInBakedLighting
 */

/*!
    \qmlproperty bool BakedLightmap::enabled

    When false, the lightmap generated for the model is not stored during
    lightmap baking, even though \l key is set to a non-empty value.

    The default value is true.
 */

/*!
    \qmlproperty string BakedLightmap::key

    When non-empty and \l enabled is true, the lightmap generated for the model
    is stored persistently during lightmap baking.
    No other Model in the scene must use the same key.

    The default value is empty.
 */

/*!
    \qmlproperty string BakedLightmap::loadPrefix
    \deprecated [6.10] This has no effect. See \l Lightmapper documentation.
 */

bool QQuick3DBakedLightmap::isEnabled() const
{
    return m_enabled;
}

void QQuick3DBakedLightmap::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    emit enabledChanged();
    emit changed();
}

QString QQuick3DBakedLightmap::key() const
{
    return m_key;
}

void QQuick3DBakedLightmap::setKey(const QString &key)
{
    if (m_key == key)
        return;

    m_key = key;
    emit keyChanged();
    emit changed();
}

QString QQuick3DBakedLightmap::loadPrefix() const
{
    return m_loadPrefix;
}

void QQuick3DBakedLightmap::setLoadPrefix(const QString &loadPrefix)
{
    if (m_loadPrefix == loadPrefix)
        return;

    m_loadPrefix = loadPrefix;
    emit loadPrefixChanged();
    emit changed();
}

QT_END_NAMESPACE
