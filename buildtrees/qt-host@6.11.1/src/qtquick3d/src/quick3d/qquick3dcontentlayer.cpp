// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcontentlayer_p.h"

/*!
    \qmltype ContentLayer
    \inqmlmodule QtQuick3D
    \since 6.11
    \brief Singleton providing layer flag constants for grouping and filtering scene nodes.

    ContentLayer is a singleton that exposes the \c LayerFlag enumeration, which provides
    bit flag constants for assigning nodes to named content layers. Layers can be used to
    group scene nodes together or to filter which nodes are rendered by a camera.

    The predefined layer names are not necessarily descriptive for specific use cases. You
    can define your own semantic layer names by aliasing the \l ContentLayer values. For
    example, in a city scene with buildings, streets, and vehicles:

    \qml
    readonly property int layerBuildings: ContentLayer.Layer1
    readonly property int layerStreets: ContentLayer.Layer2
    readonly property int layerVehicles: ContentLayer.Layer3 | ContentLayer.Layer4
    \endqml

    Once defined, assign these flags to nodes via \l {Node::layers}{Node.layers} and set
    the matching flags on a \l Camera to control which layers it renders.

    \sa {Qt Quick 3D - Layers Example}{Layers Example}

    \note Some layers have special meanings:

    \table
    \header
        \li Layer
        \li Description
    \row
        \li LayerNone
        \li No layer assigned. Nodes with this value do not belong to any layer and are not rendered.
    \row
        \li Layer0
        \li The default main layer. Nodes are assigned to this layer by default.
    \row
        \li Layer1–Layer23
        \li Freely assignable user layers for grouping nodes by purpose or functionality.
    \row
        \li Layer24 and above
        \li Reserved for internal use by Qt Quick 3D.
    \row
        \li LayerAll
        \li Matches all layers; nodes with this value are rendered by every camera.
    \endtable

    \note Layers are bit flags and can be combined with the \c | operator.

    \note ContentLayer is a singleton and is not intended to be instantiated directly.
*/

QT_BEGIN_NAMESPACE

QQuick3DContentLayer::QQuick3DContentLayer(QObject *parent)
    : QObject(parent)
{

}

QQuick3DContentLayer::~QQuick3DContentLayer()
{

}

QT_END_NAMESPACE
