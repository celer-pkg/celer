// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtGraphs/qgraphs3dnamespace.h>
QT_BEGIN_NAMESPACE
/*!
    \namespace QtGraphs3D
    \inmodule QtGraphs
    \ingroup graphs_3D
    \brief The QtGraphs3D module provides enums used by QtGraphs' 3D API.

*/

/*!
    \enum QtGraphs3D::SelectionFlag

    Item selection modes. Values of this enumeration can be combined with OR
    operator.

    \value None
           Selection mode disabled.
    \value Item
           Selection highlights a single item.
    \value Row
           Selection highlights a single row.
    \value ItemAndRow
           Combination flag for highlighting both item and row with different colors.
    \value Column
           Selection highlights a single column.
    \value ItemAndColumn
           Combination flag for highlighting both item and column with different colors.
    \value RowAndColumn
           Combination flag for highlighting both row and column.
    \value ItemRowAndColumn
           Combination flag for highlighting item, row, and column.
    \value Slice
           Setting this mode flag indicates that the graph should take care of the slice view handling
           automatically. If you wish to control the slice view yourself via Q3DScene, do not set this
           flag. When setting this mode flag, either \c Row or \c Column must also
           be set, but not both. Slicing is supported by Q3DBarsWidgetItem and Q3DSurfaceWidgetItem only.
           When this flag is set, slice mode is entered in the following situations:
           \list
           \li When selection is changed explicitly via series API to a visible item
           \li When selection is changed by clicking on the graph
           \li When the selection mode changes and the selected item is visible
           \endlist
    \value MultiSeries
           Setting this mode means that items for all series at same position are highlighted, instead
           of just the selected item. The actual selection in the other series doesn't change.
           When setting this mode flag, one or more of the basic selection flags (\c {Item},
           \c {Row}, or \c Column) must also be set.
           Multi-series selection is not supported for Q3DScatterWidgetItem.
*/

/*!
    \enum QtGraphs3D::SliceCaptureType
    \since 6.10

    Type of slice to grab to an image.

    \value NoImage
           Slice type to capture is not defined.
    \value RowImage
           Capture slice for rows.
    \value ColumnImage
           Capture slice for columns.
*/

/*!
    \enum QtGraphs3D::ShadowQuality

    Quality of shadows.

    \value None
           Shadows are disabled.
    \value Low
           Shadows are rendered in low quality.
    \value Medium
           Shadows are rendered in medium quality.
    \value High
           Shadows are rendered in high quality.
    \value SoftLow
           Shadows are rendered in low quality with softened edges.
    \value SoftMedium
           Shadows are rendered in medium quality with softened edges.
    \value SoftHigh
           Shadows are rendered in high quality with softened edges.
*/

/*!
    \enum QtGraphs3D::ElementType

    Type of an element in the graph.

    \value None
           No defined element.
    \value Series
           An item in a series.
    \value AxisXLabel
           The x-axis label.
    \value AxisYLabel
           The y-axis label.
    \value AxisZLabel
           The z-axis label.
    \value CustomItem
           A custom item.
*/

/*!
    \enum QtGraphs3D::OptimizationHint

    The optimization hint for rendering.

    \value Default
           Provides the full feature set with instancing at a good performance.
    \value Legacy
           Provides the full feature set at a reasonable performance. To be used if
           OptimizationHint.Default performs poorly or does not work.
*/

/*!
    \enum QtGraphs3D::CameraPreset

    Predefined positions for camera.

    \value NoPreset
        Used to indicate a preset has not been set, or the scene has been rotated freely.
    \value FrontLow
        Both x and y rotations of the camera are 0.
    \value Front
        X rotation is 0 and y rotation is 22.5 degrees.
    \value FrontHigh
        X rotation is 0 and y rotation is 45 degrees.
    \value LeftLow
        X rotation is 90 and y rotation is 0 degrees.
    \value Left
        X rotation is 90 and y rotation is 22.5 degrees.
    \value LeftHigh
        X rotation is 90 and y rotation is 45 degrees.
    \value RightLow
        X rotation is -90 and y rotation is 0 degrees.
    \value Right
        X rotation is -90 and y rotation is 22.5 degrees.
    \value RightHigh
        X rotation is -90 and y rotation is 45 degrees.
    \value BehindLow
        X rotation is 180 and y rotation is 0 degrees.
    \value Behind
        X rotation is 180 and y rotation is 22.5 degrees.
    \value BehindHigh
        X rotation is 180 and y rotation is 45 degrees.
    \value IsometricLeft
        X rotation is 45 and y rotation is 22.5 degrees.
    \value IsometricLeftHigh
        X rotation is 45 and y rotation is 45 degrees.
    \value IsometricRight
        X rotation is -45 and y rotation is 22.5 degrees.
    \value IsometricRightHigh
        X rotation is -45 and y rotation is 45 degrees.
    \value DirectlyAbove
        X rotation is 0 and y rotation is 90 degrees.
    \value DirectlyAboveCW45
        X rotation is -45 and y rotation is 90 degrees.
    \value DirectlyAboveCCW45
        X rotation is 45 and y rotation is 90 degrees.
    \value FrontBelow
        X rotation is 0 and y rotation is -45 degrees.
        In Q3DBarsWidgetItem from FrontBelow onward these only work for graphs including negative
        values. They act as CameraPreset...Low for positive-only values.
    \value LeftBelow
        X rotation is 90 and y rotation is -45 degrees.
    \value RightBelow
        X rotation is -90 and y rotation is -45 degrees.
    \value BehindBelow
        X rotation is 180 and y rotation is -45 degrees.
    \value DirectlyBelow
        X rotation is 0 and y rotation is -90 degrees.
        Acts as FrontLow for positive-only bars.

 */

/*!
    \enum QtGraphs3D::GridLineType

    \value Shader
        Grid lines are rendered with GPU in a shader.
    \value Geometry
        Grid lines are rendered with 3D models.
*/

/*!
    \enum QtGraphs3D::RenderingMode

    \value DirectToBackground
           Indicates that the graph will be rendered directly on the window
    background and QML items are rendered on top of it. Using non-transparent QML
    item as a background will hide the graph. Clears the whole window before
    rendering the graph, including the areas outside the graph. If the surface
    format of the window supports antialiasing, it will be used (see
    \l {QQuick3D::idealSurfaceFormat()}).
    This rendering mode offers the best performance at the expense of
    non-standard QML behavior. For example, the graphs do not obey the z ordering
    of QML items and the opacity value has no effect on them.

    \value Indirect
           Indicates the graph will be first rendered to an offscreen surface
    that is then drawn during normal QML item rendering. The rendered image is
    antialiased using the multisampling method if it is supported in the current
    environment and the msaaSamples property value is greater than zero.
    This rendering mode offers good quality and normal QML item behavior at the
    expense of performance.
 */

/*!
    \enum QtGraphs3D::TransparencyTechnique
    \since 6.9

    \brief Specifies which transparency technique to use. The Default value is \c{Default}.
           When rendering transparent surface graphs, use \c{Approximate} or \c{Accurate}.
           \c Default should be used for other graph types.

    \value Default
           Indicates that order-independent transparency techniques are not used.
           Offers the best performance. Use when graphs don't contain
           transparency or when a bar or scatter graph is also using instancing,
           that is \l optimizationHint is {QtGraphs3D::OptimizationHint::Default}.

    \value Approximate
           Indicates that a graph attempts an approximation of order-independent
           transparency. This method is faster than \c Accurate and works on older
           hardware but may yield inaccurate results. Use when the order-independent
           transparency is needed, but the performance cost has to be lower than
           when using accurate order-independent transparency.

    \value Accurate
           Indicates that accurate order-independent transparency is used.
           Use when perfect transparency rendering is needed.
 */

QT_END_NAMESPACE

#include "moc_qgraphs3dnamespace.cpp"
