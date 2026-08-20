// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

/*!
    \qmltype ArcItem
    \inqmlmodule QtQuick.Studio.Components
    \since QtQuick.Studio.Components 1.0
    \inherits Shape

    \brief An arc that ends at the specified position and uses the specified
    radius.

    An arc is specified by setting values in degrees for the \l begin and
    \l end properties. The arc can be just a line or a filled outline.
    The \l strokeColor, \l strokeWidth, and \l strokeStyle properties specify
    the appearance of the line or outline. The \l dashPattern and \l dashOffset
    properties specify the appearance of dashed lines.

    The area between the arc's start and end points or the area inside the
    outline are painted using either a solid fill color, specified using the
    \l fillColor property, or a gradient, defined using one of the
    \l ShapeGradient subtypes and set using the \l gradient property.
    If both a color and a gradient are specified, the gradient is used.

    To create an arc with an outline, set the \l outlineArc property to \c true.
    The \l arcWidth property specifies the width of the arc outline, including
    the stroke. The \l arcWidthBegin and \l arcWidthEnd properties can be used
    to specify the width of the start and end points of the outline separately.
    The width of the outline between the start and end points is calculated
    automatically. The inner and outer curves or the outline can be adjusted by
    specifying values for the \l radiusInnerAdjust and \l radiusOuterAdjust
    properties.

    The \l round, \l roundBegin, and \l roundEnd properties specify whether the
    end points of the arc outline have rounded caps. For an arc that does not
    have an outline, the \l capStyle property specifies whether the line ends
    are square or rounded.

    Because an arc has curves, it may be appropriate to set the \c antialiasing
    property that is inherited from \l Item to improve its appearance.

    \section2 Example Usage

    You can use the Arc component in \QDS to create different kinds of arcs.

    \image studio-arc.png

    The QML code looks as follows:

    \code
    ArcItem {
        id: arc
        x: 31
        y: 31
        capStyle: 32
        end: 180
        strokeWidth: 6
        strokeColor: "#000000"
    }

    ArcItem {
        id: arcOutline
        strokeColor: "gray"
        arcWidth: 13
        end: 180
        fillColor: "light gray"
        antialiasing: true
        round: true
        outlineArc: true
    }

    ArcItem {
        id: circle
        end: 360
        strokeWidth: 5
        strokeColor: "#000000"
    }

    ArcItem {
        id: circleOutline
        outlineArc: true
        round: true
        strokeColor: "gray"
        fillColor: "light gray"
        strokeWidth: 1
        end: 360
    }
    \endcode
*/

Shape {
    id: root

    implicitWidth: 100
    implicitHeight: 100

/*!
    The gradient of the arc fill color.

    By default, no gradient is enabled and the value is null. In this case, the
    fill uses a solid color based on the value of \l fillColor.

    When set, \l fillColor is ignored and filling is done using one of the
    \l ShapeGradient subtypes.

    \note The \l Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like \l LinearGradient.
*/
    property alias gradient: path.fillGradient

/*!
    The style of the arc line or outline.

    \value ShapePath.SolidLine
           A solid line. This is the default value.
    \value ShapePath.DashLine
           Dashes separated by a few pixels.
           The \l dashPattern property specifies the dash pattern.

    \sa Qt::PenStyle
*/
    property alias strokeStyle: path.strokeStyle

/*!
    The width of the arc line or outline.

    When set to a negative value, no line is drawn.

    The default value is 4.

    The total width of an arc that has an outline (that is, the outline and the
    fill) is specified by \l arcWidth.
*/
    property alias strokeWidth: path.strokeWidth

/*!
    The color of the arc line or outline.

    When set to \c transparent, no line is drawn.

    The default value is \c red.

    \sa QColor
*/
    property alias strokeColor: path.strokeColor

/*!
    The dash pattern of the arc or arc outline specified as the dashes and the
    gaps between them.

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
    property alias dashPattern: path.dashPattern

    property alias joinStyle: path.joinStyle

/*!
    The arc fill color.

    If the arc is just a line, the area between its \l begin and \l end
    points is filled.

    If the arc has an outline, the area within the outline is filled.

    A gradient for the fill can be specified by using \l gradient. If both a
    color and a gradient are specified, the gradient is used.

    When set to \c transparent, no filling occurs.

    The default value is \c white.
*/
    property alias fillColor: path.fillColor

/*!
    The cap style of the line if the arc does not have an outline.

    \value ShapePath.FlatCap
           A square line end that does not cover the end point of the line.
    \value ShapePath.SquareCap
           A square line end that covers the end point and extends beyond it
           by half the line width. This is the default value.
    \value ShapePath.RoundCap
           A rounded line end.

    \sa round, roundBegin, roundEnd, Qt::PenCapStyle
*/
    property alias capStyle: path.capStyle

/*!
    The starting point of the dash pattern for the arc or arc outline.

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
    property alias dashOffset: path.dashOffset

/*!
    The position in degrees where the arc begins.

    The default value is 0.

    To create a circle, set the value of this property to 0 and the value
    of the \l end property to 360.
*/
    property real begin: 0

/*!
    The position in degrees where the arc ends.

    The default value is 90.

    To create a circle, set the value of this property to 360 and the value of
    the \l begin property to 0.
*/
    property real end: 90

/*!
    The total width of an arc that has an outline, including the outline and
    fill.

    The default value is 10.

    \sa arcWidthBegin, arcWidthEnd, strokeWidth
*/
    property real arcWidth: 10

/*!
    The area between the \l begin and \l end points of the arc.
*/
    property real alpha: root.clamp(root.sortedEnd() - root.sortedBegin(), 0, 359.9)

    layer.enabled: root.antialiasing
    layer.smooth: root.antialiasing
    layer.samples: root.antialiasing ? 4 : 0

/*!
    Whether the arc has an outline.

    \sa arcWidth, arcWidthBegin, arcWidthEnd, round, roundBegin, roundEnd,
*/
    property bool outlineArc: false

/*!
    Whether the arc outline end points have round caps.

    The \l roundBegin and \l roundEnd properties can be used to specify the
    caps separately for the end points.
*/
    property bool round: false

/*!
    Whether the arc outline ends with a round cap.

    \sa Qt::PenCapStyle, round, roundBegin
*/
    property bool roundEnd: root.round

/*!
    Whether the arc outline begins with a round cap.

    \sa Qt::PenCapStyle, round, roundEnd
*/
    property bool roundBegin: root.round

    function clamp(num, min, max) {
        return Math.max(min, Math.min(num, max))
    }

    function toRadians(degrees) {
        return degrees * (Math.PI / 180.0)
    }

    function myCos(angleInDegrees) {
        return Math.cos(root.toRadians(angleInDegrees))
    }

    function mySin(angleInDegrees) {
        return Math.sin(root.toRadians(angleInDegrees))
    }

    function polarToCartesianX(centerX, centerY, radius, angleInDegrees) {
        return centerX + radius * Math.cos(root.toRadians(angleInDegrees))
    }

    function polarToCartesianY(centerX, centerY, radius, angleInDegrees) {
        return centerY + radius * Math.sin(root.toRadians(angleInDegrees))
    }

    function sortedBegin() {
        return Math.min(root.begin, root.end)
    }

    function sortedEnd() {
        return Math.min(Math.max(root.begin, root.end), root.sortedBegin() + 359.9)
    }

    function isArcFull() {
        return root.alpha > 359.5
    }

    onAlphaChanged: {
        if (root.__wasFull !== root.isArcFull())
            root.constructArcItem()

        root.__wasFull = root.isArcFull()
    }
    onOutlineArcChanged: root.constructArcItem()
    onRoundChanged: root.constructArcItem()
    onRoundBeginChanged: root.constructArcItem()
    onRoundEndChanged: root.constructArcItem()

    property bool __wasFull: false

    property real maxArcWidth: Math.min(path.__xRadius, path.__yRadius)

    ShapePath {
        id: path

        property real __xRadius: root.width / 2 - root.strokeWidth / 2
        property real __yRadius: root.height / 2 - root.strokeWidth / 2

        property real __arcWidth: Math.min(Math.min(path.__xRadius, path.__yRadius), root.arcWidth)

        property real __xCenter: root.width / 2
        property real __yCenter: root.height / 2

        strokeColor: "red"
        strokeWidth: 4
        capStyle: ShapePath.FlatCap

        startX: root.polarToCartesianX(path.__xCenter, path.__yCenter, path.__xRadius, root.sortedBegin() - 90)
        startY: root.polarToCartesianY(path.__xCenter, path.__yCenter, path.__yRadius, root.sortedBegin() - 90)
    }

    function constructArcItem() {
        root.clearPathElements()

        // Outer arc
        let outerArc = Qt.createQmlObject('import QtQuick; PathArc {}', path)
        outerArc.x = Qt.binding(function() {
            return root.polarToCartesianX(path.__xCenter, path.__yCenter, path.__xRadius, root.sortedEnd() - 90)
        })
        outerArc.y = Qt.binding(function() {
            return root.polarToCartesianY(path.__xCenter, path.__yCenter, path.__yRadius, root.sortedEnd() - 90)
        })
        outerArc.radiusX = Qt.binding(function() { return path.__xRadius })
        outerArc.radiusY = Qt.binding(function() { return path.__yRadius })
        outerArc.useLargeArc = Qt.binding(function() { return root.alpha > 180 })
        path.pathElements.push(outerArc)

        // Straight end
        if (!root.roundEnd && root.outlineArc && !root.isArcFull()) {
            let pathLine = Qt.createQmlObject('import QtQuick; PathLine {}', path)
            pathLine.relativeX = Qt.binding(function() {
                return -path.__arcWidth * root.myCos(root.sortedEnd() - 90)
            })
            pathLine.relativeY = Qt.binding(function() {
                return -path.__arcWidth * root.mySin(root.sortedEnd() - 90)
            })
            path.pathElements.push(pathLine)
        }

        // Round end
        if (root.roundEnd && root.outlineArc && !root.isArcFull()) {
            let pathArc = Qt.createQmlObject('import QtQuick; PathArc {}', path)
            pathArc.relativeX = Qt.binding(function() {
                return -path.__arcWidth * root.myCos(root.sortedEnd() - 90)
            })
            pathArc.relativeY = Qt.binding(function() {
                return -path.__arcWidth * root.mySin(root.sortedEnd() - 90)
            })
            pathArc.radiusX = Qt.binding(function() { return path.__arcWidth / 2 })
            pathArc.radiusY = Qt.binding(function() { return path.__arcWidth / 2 })
            path.pathElements.push(pathArc)
        }

        // Open end
        if (root.outlineArc && root.isArcFull()) {
            let pathMove = Qt.createQmlObject('import QtQuick; PathMove {}', path)
            pathMove.relativeX = Qt.binding(function() {
                return -path.__arcWidth * root.myCos(root.sortedEnd() - 90)
            })
            pathMove.relativeY = Qt.binding(function() {
                return -path.__arcWidth * root.mySin(root.sortedEnd() - 90)
            })
            path.pathElements.push(pathMove)
        }

        // Inner arc
        if (root.outlineArc) {
            let innerArc = Qt.createQmlObject('import QtQuick; PathArc {}', path)
            innerArc.x = Qt.binding(function() {
                return path.startX - path.__arcWidth * root.myCos(root.sortedBegin() - 90)
            })
            innerArc.y = Qt.binding(function() {
                return path.startY - path.__arcWidth * root.mySin(root.sortedBegin() - 90)
            })
            innerArc.radiusX = Qt.binding(function() { return path.__xRadius - path.__arcWidth })
            innerArc.radiusY = Qt.binding(function() { return path.__yRadius - path.__arcWidth })
            innerArc.useLargeArc = Qt.binding(function() { return root.alpha > 180 })
            innerArc.direction = PathArc.Counterclockwise
            path.pathElements.push(innerArc)
        }

        // Straight begin
        if (!root.roundBegin && root.outlineArc && !root.isArcFull()) {
            let pathLine = Qt.createQmlObject('import QtQuick; PathLine {}', path)
            pathLine.x = Qt.binding(function() { return path.startX })
            pathLine.y = Qt.binding(function() { return path.startY })
            path.pathElements.push(pathLine)
        }

        // Round begin
        if (root.roundBegin && root.outlineArc && !root.isArcFull()) {
            let pathArc = Qt.createQmlObject('import QtQuick; PathArc {}', path)
            pathArc.x = Qt.binding(function() { return path.startX })
            pathArc.y = Qt.binding(function() { return path.startY })
            pathArc.radiusX = Qt.binding(function() { return path.__arcWidth / 2 })
            pathArc.radiusY = Qt.binding(function() { return path.__arcWidth / 2 })
            path.pathElements.push(pathArc)
        }

        // Open begin
        if (root.outlineArc && root.isArcFull()) {
            let pathMove = Qt.createQmlObject('import QtQuick; PathMove {}', path)
            pathMove.x = Qt.binding(function() { return path.startX })
            pathMove.y = Qt.binding(function() { return path.startY })
            path.pathElements.push(pathMove)
        }
    }

    function clearPathElements() {
        for (var i = 0; i !== path.pathElements.length; ++i)
            path.pathElements[i].destroy()

        path.pathElements = []
    }

    Component.onCompleted: {
        root.__wasFull = root.isArcFull()
        root.constructArcItem()
    }
}
