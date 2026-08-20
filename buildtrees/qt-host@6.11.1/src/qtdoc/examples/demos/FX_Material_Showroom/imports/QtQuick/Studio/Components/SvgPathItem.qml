// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

/*!
    \qmltype SvgPathItem
    \inqmlmodule QtQuick.Studio.Components
    \since QtQuick.Studio.Components 1.0
    \inherits Shape

    \brief A path defined using an SVG path data string.

    The SvgPathItem type uses an SVG path data string to draw a path as a line.

    The \l strokeColor, \l strokeWidth, and \l strokeStyle, properties specify
    the appearance of the path. The \l dashPattern and \l dashOffset properties
    specify the appearance of dashed lines.

    The \l capStyle property specifies whether line ends are square or
    rounded.

    The \l joinStyle property specifies how to connect two path segments.
    If the path segments enclose areas, they can be painted using either
    a solid fill color, specified using the \l fillColor property, or a
    gradient, defined using one of the \l ShapeGradient subtypes and set
    using the \l gradient property. If both a color and a gradient are
    specified, the gradient is used.

    If the path has curves, it may be appropriate to set the \c antialiasing
    property that is inherited from \l Item to improve its appearance.
*/

Shape {
    id: root
    width: 200
    height: 200

/*!
    The gradient of the fill color.

    By default, no gradient is enabled and the value is null. In this case, the
    fill uses a solid color based on the value of \l fillColor.

    When set, \l fillColor is ignored and filling is done using one of the
    \l ShapeGradient subtypes.

    \note The \l Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like \l LinearGradient.
*/
    property alias gradient: shape.fillGradient

/*!
    The style of the line.

    \value Shape.SolidLine
           A solid line. This is the default value.
    \value Shape.DashLine
           Dashes separated by a few pixels.
           The \l dashPattern property specifies the dash pattern.

    \sa Qt::PenStyle
*/
    property alias strokeStyle: shape.strokeStyle

/*!
    The width of the line.

    When set to a negative value, no line is drawn.

    The default value is 4.
*/
    property alias strokeWidth: shape.strokeWidth

/*!
    The color of the line.

    When set to \c transparent, no line is drawn.

    The default value is \c red.

    \sa QColor
*/
    property alias strokeColor: shape.strokeColor

/*!
    The dash pattern of the line specified as the dashes and the gaps between
    them.

    The dash pattern is specified in units of the pen's width. That is, a dash
    with the length 5 and width 10 is 50 pixels long.

    Each dash is also subject to cap styles, and therefore a dash of 1 with
    square cap set will extend 0.5 pixels out in each direction resulting in
    a total width of 2.

    The default \l capStyle is \c {ShapePath.SquareCap}, meaning that a square
    line end covers the end point and extends beyond it by half the line width.

    The default value is (4, 2), meaning a dash of 4 * \l strokeWidth pixels
    followed by a space of 2 * \l strokeWidth pixels.

    \sa QPen::setDashPattern()
*/
    property alias dashPattern: shape.dashPattern

/*!
    The join style used to connect two path segments.

    \value Shape.MiterJoin
           The outer edges of the lines are extended to meet at an angle, and
           this area is filled.
    \value Shape.BevelJoin
           The triangular notch between the two lines is filled.
           This is the default value.
    \value Shape.RoundJoin
           A circular arc between the two lines is filled.

    \sa Qt::PenJoinStyle
*/
    property alias joinStyle: shape.joinStyle

/*!
    The fill color of enclosed path segments.

    A gradient for the fill can be specified by using \l gradient. If both a
    color and a gradient are specified, the gradient is used.

    When set to \c transparent, no filling occurs.

    The default value is \c white.
*/
    property alias fillColor: shape.fillColor

/*!
    \qmlproperty string SvgPathItem::path

    The SVG path data string specifying the path.

    For more information, see \l{https://www.w3.org/TR/SVG/paths.html#PathData}
    {W3C SVG Path Data}.
*/
    property alias path: pathSvg.path

/*!
    The starting point of the dash pattern for the line.

    The offset is measured in terms of the units used to specify the dash
    pattern. For example, a pattern where each stroke is four units long,
    followed by a gap of two units, will begin with the stroke when drawn
    as a line. However, if the dash offset is set to 4.0, any line drawn
    will begin with the gap. Values of the offset up to 4.0 will cause part
    of the stroke to be drawn first, and values of the offset between 4.0 and
    6.0 will cause the line to begin with part of the gap.

    The default value is 0.

    \sa QPen::setDashOffset()
*/
    property alias dashOffset: shape.dashOffset

/*!
    The cap style of the line.

    \value Shape.FlatCap
           A square line end that does not cover the end point of the line.
    \value Shape.SquareCap
           A square line end that covers the end point and extends beyond it
           by half the line width. This is the default value.
    \value Shape.RoundCap
           A rounded line end.

    \sa Qt::PenCapStyle
*/
    property alias capStyle: shape.capStyle

    layer.enabled: root.antialiasing
    layer.smooth: root.antialiasing
    layer.samples: root.antialiasing ? 4 : 0

    ShapePath {
        id: shape
        strokeWidth: 4
        strokeColor: "red"
        joinStyle: ShapePath.MiterJoin

        PathSvg {
            id: pathSvg

            path: "M91,70.6c4.6,0,8.6,2.4,10.9,6.3l19.8,34.2c2.3,3.9,2.3,8.7,0,12.6c-2.3,3.9-6.4,6.3-10.9,6.3H71.2 c-4.6,0-8.6-2.4-10.9-6.3c-2.3-3.9-2.3-8.7,0-12.6l19.8-34.2C82.4,72.9,86.4,70.6,91,70.6z"
        }
    }
}
