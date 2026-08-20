// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qgraphstheme.h"
#include "commonutils_p.h"

#include <QGuiApplication>
#include <QLinearGradient>
#include <QQmlListProperty>
#include <QStyleHints>

#include <private/qgraphsglobal_p.h>
#include <private/qgraphstheme_p.h>
#include <private/qquickgraphscolor_p.h>
#include <private/qquickrectangle_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcGraphsCommonTheme, "qt.graphs.common.theme")

/*!
 * \class QGraphsTheme
 * \inmodule QtGraphs
 * \ingroup graphs_common
 * \brief QGraphsTheme class provides a visual style for graphs.
 *
 * Specifies visual properties that affect the whole graph. There are several
 * built-in themes that can be used as is or modified freely.
 *
 * Themes can be created from scratch using the UserDefined enum value.
 * Creating a theme using the default constructor produces a new user-defined
 * theme.
 *
 * \section1 Customizing Theme
 *
 * The default theme is QtGreen, but it is possible to customize each property.
 *
 * The following table lists the properties controlled by a theme and the
 * default values for UserDefined.
 *
 * \table
 *   \header
 *     \li Property
 *     \li Default Value
 *   \row
 *     \li backgroundVisible
 *     \li \c true
 *   \row
 *     \li seriesColors
 *     \li Qt::black
 *   \row
 *     \li seriesGradients
 *     \li QLinearGradient. Essentially fully black.
 *   \row
 *     \li colorStyle
 *     \li Uniform
 *   \row
 *     \li \l labelFont
 *     \li QFont
 *   \row
 *     \li gridVisible
 *     \li \c true
 *   \row
 *     \li labelBackgroundVisible
 *     \li \c true
 *   \row
 *     \li labelBorderVisible
 *     \li \c true
 *   \row
 *     \li labelsVisible
 *     \li \c true
 * \endtable
 *
 * \section1 Usage Examples
 *
 * Creating a built-in theme without any modifications:
 *
 * \snippet doc_src_qgraphstheme.cpp 0
 *
 * Creating a built-in theme and modifying some properties:
 *
 * \snippet doc_src_qgraphstheme.cpp 1
 *
 * Modifying a user-defined theme. The theme has been created the same way it was in the previous
 * snippets:
 *
 * \snippet doc_src_qgraphstheme.cpp 2
 *
 * Modifying some properties after theme has been set to a graph:
 *
 * \snippet doc_src_qgraphstheme.cpp 3
 *
 */

/*!
 * \enum QGraphsTheme::ColorStyle
 *
 * Gradient types.
 *
 * \value Uniform
 *        Objects are rendered in a single color. The color used is specified in seriesColors,
 *        singleHighlightColor and multiHighlightColor properties.
 * \value ObjectGradient
 *        Objects are colored using a full gradient for each object regardless of object height. The
 *        gradient used is specified in seriesGradients, singleHighlightGradient and
 *        multiHighlightGradient properties.
 * \value RangeGradient
 *        Objects are colored using a portion of the full gradient determined by the object's
 *        height and its position on the Y-axis. The gradient used is specified in seriesGradients,
 *        singleHighlightGradient and multiHighlightGradient properties.
 */

/*!
 * \enum QGraphsTheme::Theme
 *
 * Built-in themes.
 *
 * \value QtGreen
 *        A light theme with green as the base color.
 * \value QtGreenNeon
 *        A light theme with green neon as the base color.
 * \value MixSeries
 *        A mixed theme with various colors.
 * \value OrangeSeries
 *        A theme with Orange as the base color.
 * \value YellowSeries
 *        A theme with Yellow as the base color.
 * \value BlueSeries
 *        A theme with Blue as the base color.
 * \value PurpleSeries
 *        A theme with Purple as the base color.
 * \value GreySeries
 *        A theme with Grey as the base color.
 * \value UserDefined
 *        A user-defined theme. For more information, see
 *        \l {QGraphsTheme#Customizing Theme}{Customizing Theme}.
 */

/*!
 * \enum QGraphsTheme::ColorScheme
 *
 * Represents the color scheme of the graph.
 *
 * \value Automatic
 *        The background colors follow the platform color scheme if available.
 *        If unavailable, the Light appearance is used.
 * \value Light
 *        The background colors are lighter than the text color, i.e. the theme is light.
 * \value Dark
 *        The background colors are darker than the text color, i.e. the theme is dark.
 * \sa Qt::ColorScheme
 */

/*!
    \class QGraphsLine
    \inmodule QtGraphs
    \brief The QGraphsLine class encapsulates properties of lines on a graph.
*/

/*!
 * \qmlvaluetype graphsline
 * \nativetype QGraphsLine
 * \ingroup graphs_qml_common
 * \brief a values for lines based on properties of QGraphsLine.
 *
 * The \c graphsline type refers to a line value with the properties of QGraphsLine.
 *
 * Properties of type \c graphsline follows the \l {GraphsTheme::}{theme} unless
 * defined separately.
 *
 * When integrating with C++, note that any QGraphsLine value
 * \l{qtqml-cppintegration-data.html}{passed into QML from C++} is automatically
 * converted into a \c graphsline value, and vice-versa.
 *
 * This value type is provided by the QtQuick import.
 *
 * \sa {QML Value Types}
 */

/*!
    \qmlproperty color graphsline::mainColor
    The color of the main lines.
*/

/*!
    \qmlproperty color graphsline::subColor
    The color of the sub lines.
*/

/*!
    \qmlproperty real graphsline::mainWidth
    The width of the main lines.
*/

/*!
    \qmlproperty real graphsline::subWidth
    The width of the sub lines.
*/

/*!
    \qmlproperty color graphsline::labelTextColor
    The color of the text used for labels.
*/

/*!
 * \qmltype GraphsTheme
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_common
 * \nativetype QGraphsTheme
 * \brief A visual style for graphs.
 *
 * This type is used to specify visual properties that affect the whole graph.
 * There are several built-in themes that can be used as is or modified freely.
 *
 *
 * Themes can be created from scratch by using the
 * \l{QGraphsTheme::Theme::UserDefined}{GraphsTheme.Theme.UserDefined} enum value.
 *
 * \section1 Customizing Theme
 *
 * The default theme is QtGreen, but it is possible to customize each property.
 *
 * The following table lists the properties controlled by a theme and the
 * default values for \l{QGraphsTheme::Theme::UserDefined}
 * {QGraphsTheme.Theme.UserDefined}.
 *
 * \table
 *   \header
 *     \li Property
 *     \li Default Value
 *   \row
 *     \li backgroundVisible
 *     \li \c true
 *   \row
 *     \li seriesColors
 *     \li Qt::black
 *   \row
 *     \li baseGradients
 *     \li QLinearGradient. Essentially fully black.
 *   \row
 *     \li colorStyle
 *     \li Uniform
 *   \row
 *     \li \l labelFont
 *     \li QFont
 *   \row
 *     \li gridVisible
 *     \li \c true
 *   \row
 *     \li labelBackgroundVisible
 *     \li \c true
 *   \row
 *     \li labelBorderVisible
 *     \li \c true
 *   \row
 *     \li labelsVisible
 *     \li \c true
 * \endtable
 *
 * \section1 Usage examples
 *
 * Using a built-in theme without any modifications:
 *
 * \snippet doc_src_qmlgraphstheme.qml scatter
 * \dots
 *
 * Using a built-in theme and modifying some properties:
 *
 * \snippet doc_src_qmlgraphstheme.qml bars
 * \dots
 *
 * Using a user-defined theme:
 *
 * \snippet doc_src_qmlgraphstheme.qml surface
 * \dots
 *
 * For GraphsTheme enums, see \l GraphsTheme::colorStyle and \l{GraphsTheme::theme}.
 */

/*!
    \qmlproperty list<color> GraphsTheme::seriesColors

    The list of colors to be used for all the objects in the graph, series
    by series. If there are more series than colors, color list wraps and starts
    again with the first color in the list. Has no immediate effect if \l colorStyle
    is not \c GraphsTheme.ColorStyle.Uniform.

    Example usage:
    \badcode
    seriesColors: [ "red" ]
    \endcode

    This can be overridden by setting \l{Abstract3DSeries::baseColor}
    {Abstract3DSeries.baseColor} explicitly in the series.
 */

/*!
    \qmlproperty list<Color> GraphsTheme::baseColors

    The list of base colors of type Color to be used for all the objects in the graph, series
    by series. If there are more series than colors, color list wraps and starts
    again with the first color in the list. Has no immediate effect if \l colorStyle
    is not \c GraphsTheme.ColorStyle.Uniform.

    Example usage:
    \badcode
    baseColors: [ Color { color: "red" } ]
    \endcode

    This can be overridden by setting \l{Abstract3DSeries::baseColor}
    {Abstract3DSeries.baseColor} explicitly in the series.
 */

/*!
 * \qmlproperty list<color> GraphsTheme::borderColors
 *
 * The list of border colors to be used for all the objects in the graph,
 * series by series.
 *
 * If there are more series than colors, the color list wraps and starts again
 * with the first color in the list.
 *
 * Has no immediate effect if colorStyle is not Uniform.
 */

/*!
 * \qmlproperty color GraphsTheme::plotAreaBackgroundColor
 *
 * The color of the graph plot area background.
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty color GraphsTheme::backgroundColor
 *
 * The color of the view the graph is drawn into.
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty color GraphsTheme::labelTextColor
 *
 * The color of the font used for labels.
 *
 * If an axis has specified \l{graphsline::labelTextColor}{labelTextColor} explicitly,
 * this has no effect.
 *
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty color GraphsTheme::labelBackgroundColor
 *
 * The color of the label backgrounds. Has no effect if labelBackgroundVisible
 * is \c false.
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty color GraphsTheme::singleHighlightColor
 *
 * The highlight color for a selected object. Used if
 * \l{GraphsItem3D::selectionMode}{selectionMode}
 * has the \c Graphs3D.SelectionFlag.Item flag set.
 * The default value depends on \l colorScheme.
 *
 * \sa Graphs3D.SelectionFlag
 */

/*!
 * \qmlproperty color GraphsTheme::multiHighlightColor
 *
 * The highlight color for selected objects. Used if
 * \l{GraphsItem3D::selectionMode}{selectionMode}
 * has the \c Graphs3D.SelectionFlag.Row or \c Graphs3D.SelectionFlag.Column
 * flag set.
 * The default value depends on \l colorScheme.
 *
 * \sa Graphs3D.SelectionFlag
 */

/*!
   \qmlproperty list<Gradient> GraphsTheme::baseGradients

    The list of base gradients to be used for all the objects in the graph,
    series by series. If there are more series than gradients, the gradient list
    wraps and starts again with the first gradient in the list.

    Has no immediate effect if colorStyle is \l{QGraphsTheme::ColorStyle::Uniform}
    {GraphsTheme.ColorStyle.Uniform}.

    Example usage:
    \badcode
    baseGradients: [ Gradient {
        GradientStop { position: 1.0; color: "#DBEB00" }
        GradientStop { position: 0.0; color: "#373F26" }
    } ]
    \endcode

   This value can be overridden by setting \l{Abstract3DSeries::baseGradient}
   {Abstract3DSeries.baseGradient} explicitly in the series.
 */

/*!
 * \qmlproperty Gradient GraphsTheme::singleHighlightGradient
 *
 * The highlight gradient for a selected object. Used if
 * \l{GraphsItem3D::selectionMode}{selectionMode}
 * has the \c Graphs3D.SelectionFlag.Item flag set.
 * The default value depends on \l colorScheme.
 *
 * \sa Graphs3D.SelectionFlag
 */

/*!
 * \qmlproperty Gradient GraphsTheme::multiHighlightGradient
 *
 * The highlight gradient for selected objects. Used if
 * \l{GraphsItem3D::selectionMode}{selectionMode}
 * has the \c Graphs3D.SelectionFlag.Row or \c Graphs3D.SelectionFlag.Column
 * flag set.
 * The default value depends on \l colorScheme.
 *
 * \sa Graphs3D.SelectionFlag
 */

/*!
 * \qmlproperty bool GraphsTheme::labelBorderVisible
 *
 * Defines whether label borders are drawn for labels that have a background.
 * Has no effect if labelBackgroundVisible is \c false.
 * The default value is \c true.
 */

/*!
 * \qmlproperty font GraphsTheme::labelFont
 *
 * Sets the font to be used for labels.
 */

/*!
 * \qmlproperty font GraphsTheme::axisXLabelFont
 *
 * Sets the font to be used for labels on axisX.
 */

/*!
 * \qmlproperty font GraphsTheme::axisYLabelFont
 *
 * Sets the font to be used for labels on axisY.
 */

/*!
 * \qmlproperty font GraphsTheme::axisZLabelFont
 *
 * Sets the font to be used for labels on axisZ.
 */

/*!
 * \qmlproperty bool GraphsTheme::backgroundVisible
 *
 * Defines whether the view background is drawn by using the value of
 * backgroundColor.
 * The default value is \c true.
 */

/*!
 * \qmlproperty bool GraphsTheme::plotAreaBackgroundVisible
 *
 * Defines whether the plot area background is drawn by using the value of
 * plotAreaBackgroundColor.
 * The default value is \c true.
 */

/*!
 * \qmlproperty bool GraphsTheme::gridVisible
 *
 * Defines whether the grid lines are drawn. This value affects all grid lines.
 * The default value is \c true.
 */

/*!
 * \qmlproperty bool GraphsTheme::labelBackgroundVisible
 *
 * Defines whether the label is drawn with a background that uses
 * labelBackgroundColor (including alpha), or with a fully transparent
 * background. Labels with a background are drawn to equal sizes per axis based
 * on the longest label, and the text is centered in them. Labels without
 * a background are drawn as is and are left or right aligned based on their
 * position in the graph.
 * The default value is \c true.
 */

/*!
 * \qmlproperty GraphsTheme.ColorStyle GraphsTheme::colorStyle
 *
 * The style of the graph colors. One of QGraphsTheme::ColorStyle enum values.
 *
 * This value can be overridden by setting \l{Abstract3DSeries::colorStyle}
 * {Abstract3DSeries.colorStyle} explicitly in the series.
 * \note This property does not have an effect in Qt Graphs for 2D.
 *
 * \sa QGraphsTheme::ColorStyle
 */

/*!
 * \qmlproperty QGraphsTheme::ColorScheme GraphsTheme::colorScheme
 *
 * The color scheme of the graph in use.
 *
 * \sa QGraphsTheme::ColorScheme
 */

/*!
 * \qmlproperty bool GraphsTheme::labelsVisible
 *
 * Defines whether labels are drawn at all. If this is \c{false}, all other
 * label properties have no effect.
 * The default value is \c true.
 */

/*!
 * \qmlproperty GraphsTheme.Theme GraphsTheme::theme
 *
 * The type of the theme. If no type is set, the type is
 * \l{QGraphsTheme::Theme::QtGreen}{GraphsTheme.Theme.QtGreen}.
 * Changing the theme type after the item has been constructed will change all
 * other properties of the theme to what the predefined theme specifies.
 * Changing the theme type of the active theme of the graph will also reset all
 * attached series to use the new theme.
 */

/*!
 * \qmlproperty GraphsLine GraphsTheme::grid
 *
 * Holds the \l{graphsline}{GraphsLine} of the theme.
 * \sa GraphsLine.mainColor GraphsLine.subColor GraphsLine.mainWidth GraphsLine.subWidth
 * \sa GraphsLine.labelTextColor
 */

/*!
 * \qmlproperty GraphsLine GraphsTheme::axisX
 *
 * Holds the \l{graphsline}{GraphsLine} of the X axis.
 * \sa GraphsLine.mainColor GraphsLine.subColor GraphsLine.mainWidth GraphsLine.subWidth
 * \sa GraphsLine.labelTextColor
 */

/*!
 * \qmlproperty GraphsLine GraphsTheme::axisY
 *
 * Holds the \l{graphsline}{GraphsLine} of the Y axis.
 * \sa GraphsLine.mainColor GraphsLine.subColor GraphsLine.mainWidth GraphsLine.subWidth
 * \sa GraphsLine.labelTextColor
 */

/*!
 * \qmlproperty GraphsLine GraphsTheme::axisZ
 *
 * Holds the \l{graphsline}{GraphsLine} of the Z axis.
 * \sa GraphsLine.mainColor GraphsLine.subColor GraphsLine.mainWidth GraphsLine.subWidth
 * \sa GraphsLine.labelTextColor
 */

/*!
 * \qmlproperty color GraphsTheme::GraphsLine.mainColor
 *
 * The color of the main lines.
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty color GraphsTheme::GraphsLine.subColor
 *
 * The color of the sub lines.
 * The default value depends on \l colorScheme.
 */

/*!
 * \qmlproperty real GraphsTheme::GraphsLine.mainWidth
 *
 * The width of the main lines.
 * The default value is \c 2.0.
 *
 * If it is set for grid lines, only has effect if
 * \l{GraphsItem3D::gridLineType} is \c Graphs3D.GridLineType.Shader
 *
 * \sa Graphs3D.GridLineType
 */

/*!
 * \qmlproperty real GraphsTheme::GraphsLine.subWidth
 *
 * The width of the sub lines.
 * The default value is \c 1.0.
 *
 * If it is set for grid lines, only has effect if
 * \l{GraphsItem3D::gridLineType} is \c Graphs3D.GridLineType.Shader
 *
 * \sa Graphs3D.GridLineType
 */

/*!
 * \qmlproperty color GraphsTheme::GraphsLine.labelTextColor
 *
 * The color of the text used for labels.
 * The default value depends on \l colorScheme.
 */

/*!
    \fn QGraphsLine::QGraphsLine(QGraphsLine &&other)

    Move-constructs a new QGraphsLine from \a other.

    \note The moved-from object \a other is placed in a
    partially-formed state, in which the only valid operations are
    destruction and assignment of a new value.
*/

/*!
    \fn QGraphsLine &QGraphsLine::operator=(QGraphsLine &&other)

    Move-assigns \a other to this QGraphsLine instance.

    \note The moved-from object \a other is placed in a
    partially-formed state, in which the only valid operations are
    destruction and assignment of a new value.
*/

/*! \fn void QGraphsLine::swap(QGraphsLine &other)

    Swaps QGraphsLine \a other with this QGraphsLine. This operation is very fast and
    never fails.
*/

bool comparesEqual(const QGraphsLine &lhs, const QGraphsLine &rhs) noexcept
{
    return comparesEqual(*lhs.d, *rhs.d);
}

QGraphsTheme::QGraphsTheme(QObject *parent)
    : QGraphsTheme(*(new QGraphsThemePrivate()), parent)
{}

QGraphsTheme::QGraphsTheme(QGraphsThemePrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    setBackgroundVisible(true);
    setPlotAreaBackgroundVisible(true);
    setLabelBackgroundVisible(true);
    setGridVisible(true);
    setLabelsVisible(true);
    setColorScheme(QGraphsTheme::ColorScheme::Automatic);
    setLabelBorderVisible(true);
    setTheme(Theme::QtGreen, ForceTheme::Yes);
    setLabelFont(QFont(QLatin1String("Arial")));
    setAxisXLabelFont(QFont());
    setAxisYLabelFont(QFont());
    setAxisZLabelFont(QFont());
}

QGraphsTheme::~QGraphsTheme()
{
}

bool QGraphsTheme::themeDirty() const
{
    Q_D(const QGraphsTheme);
    return d->m_themeDirty;
}

void QGraphsTheme::resetThemeDirty()
{
    Q_D(QGraphsTheme);
    d->m_themeDirty = false;
}

void QGraphsTheme::resetColorTheme()
{
    setTheme(QGraphsTheme::Theme::QtGreen);
}

QGraphsThemeDirtyBitField *QGraphsTheme::dirtyBits()
{
    Q_D(QGraphsTheme);
    return &d->m_dirtyBits;
}

void QGraphsTheme::resetDirtyBits()
{
    Q_D(QGraphsTheme);
    d->m_dirtyBits.plotAreaBackgroundColorDirty = true;
    d->m_dirtyBits.plotAreaBackgroundVisibilityDirty = true;
    d->m_dirtyBits.seriesColorsDirty = true;
    d->m_dirtyBits.seriesGradientDirty = true;
    d->m_dirtyBits.colorSchemeDirty = true;
    d->m_dirtyBits.colorStyleDirty = true;
    d->m_dirtyBits.labelFontDirty = true;
    d->m_dirtyBits.gridVisibilityDirty = true;
    d->m_dirtyBits.gridDirty = true;
    d->m_dirtyBits.labelBackgroundColorDirty = true;
    d->m_dirtyBits.labelBackgroundVisibilityDirty = true;
    d->m_dirtyBits.labelBorderVisibilityDirty = true;
    d->m_dirtyBits.labelTextColorDirty = true;
    d->m_dirtyBits.axisXDirty = true;
    d->m_dirtyBits.axisYDirty = true;
    d->m_dirtyBits.axisZDirty = true;
    d->m_dirtyBits.labelsVisibilityDirty = true;
    d->m_dirtyBits.multiHighlightColorDirty = true;
    d->m_dirtyBits.multiHighlightGradientDirty = true;
    d->m_dirtyBits.singleHighlightColorDirty = true;
    d->m_dirtyBits.singleHighlightGradientDirty = true;
    d->m_dirtyBits.themeDirty = true;
    d->m_dirtyBits.backgroundColorDirty = true;
    d->m_dirtyBits.backgroundVisibilityDirty = true;
}

/*!
 * \property QGraphsTheme::colorScheme
 *
 * The color scheme of the graph in use.
 *
 * \sa Qt::ColorScheme
 */
QGraphsTheme::ColorScheme QGraphsTheme::colorScheme() const
{
    Q_D(const QGraphsTheme);
    return d->m_colorScheme;
}

void QGraphsTheme::setColorScheme(QGraphsTheme::ColorScheme newColorScheme)
{
    Q_D(QGraphsTheme);
    d->m_dirtyBits.colorSchemeDirty = true;
    d->m_colorScheme = newColorScheme;
    setColorSchemePalette();
    d->m_themeDirty = true;
    Q_EMIT colorSchemeChanged();
    Q_EMIT update();

    if (d->m_colorScheme == QGraphsTheme::ColorScheme::Automatic) {
        if (!d->m_autoColorConnection) {
            d->m_autoColorConnection = QObject::connect(QGuiApplication::styleHints(),
                                                        &QStyleHints::colorSchemeChanged,
                                                        this,
                                                        [this]() {
                                                            this->updateAutomaticColorScheme();
                                                        });
        }
    } else {
        QObject::disconnect(d->m_autoColorConnection);
    }
}

/*!
 * \property QGraphsTheme::theme
 *
 * The type of the theme. If no type is set, the type is
 * \l{QGraphsTheme::Theme::QtGreen}{GraphsTheme.Theme.QtGreen}.
 * Changing the theme type after the item has been constructed will change all
 * other properties of the theme to what the predefined theme specifies.
 * Changing the theme type of the active theme of the graph will also reset all
 * attached series to use the new theme.
 */
QGraphsTheme::Theme QGraphsTheme::theme() const
{
    Q_D(const QGraphsTheme);
    return d->m_theme;
}

void QGraphsTheme::setTheme(Theme newTheme, ForceTheme force)
{
    Q_D(QGraphsTheme);
    if ((force == ForceTheme::No && d->m_theme == newTheme)
        || newTheme < QGraphsTheme::Theme::QtGreen || newTheme > QGraphsTheme::Theme::UserDefined) {
        return;
    }
    d->m_dirtyBits.themeDirty = true;
    d->m_theme = newTheme;
    d->m_themeDirty = true;
    setThemePalette();
    Q_EMIT themeChanged(d->m_theme);
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::colorStyle
 *
 * The style of the graph colors. One of QGraphsTheme::ColorStyle enum values.
 *
 * This value can be overridden by setting \l{Abstract3DSeries::colorStyle}
 * {Abstract3DSeries.colorStyle} explicitly in the series.
 * \note This property does not have an effect in Qt Graphs for 2D.
 *
 * \sa QGraphsTheme::ColorStyle
 */
QGraphsTheme::ColorStyle QGraphsTheme::colorStyle() const
{
    Q_D(const QGraphsTheme);
    return d->m_colorStyle;
}

void QGraphsTheme::setColorStyle(ColorStyle newColorStyle)
{
    Q_D(QGraphsTheme);
    if (d->m_colorStyle == newColorStyle)
        return;
    d->m_dirtyBits.colorStyleDirty = true;
    d->m_colorStyle = newColorStyle;
    Q_EMIT colorStyleChanged(newColorStyle);
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::axisXLabelFont
 *
 * \brief The font to be used for labels on x axis.
 */
QFont QGraphsTheme::axisXLabelFont() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisXLabelFont;
}

void QGraphsTheme::setAxisXLabelFont(const QFont &newAxisXLabelFont)
{
    Q_D(QGraphsTheme);
    d->m_customBits.axisXLabelFontCustom = true;
    if (d->m_axisXLabelFont == newAxisXLabelFont)
        return;
    d->m_axisXLabelFont = newAxisXLabelFont;
    Q_EMIT axisXLabelFontChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::axisYLabelFont
 *
 * \brief The font to be used for labels on y axis.
 */
QFont QGraphsTheme::axisYLabelFont() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisYLabelFont;
}

void QGraphsTheme::setAxisYLabelFont(const QFont &newAxisYLabelFont)
{
    Q_D(QGraphsTheme);
    d->m_customBits.axisYLabelFontCustom = true;
    if (d->m_axisYLabelFont == newAxisYLabelFont)
        return;
    d->m_axisYLabelFont = newAxisYLabelFont;
    Q_EMIT axisYLabelFontChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::axisZLabelFont
 *
 * \brief The font to be used for labels on z axis.
 */
QFont QGraphsTheme::axisZLabelFont() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisZLabelFont;
}

void QGraphsTheme::setAxisZLabelFont(const QFont &newAxisZLabelFont)
{
    Q_D(QGraphsTheme);
    d->m_customBits.axisZLabelFontCustom = true;
    if (d->m_axisZLabelFont == newAxisZLabelFont)
        return;
    d->m_axisZLabelFont = newAxisZLabelFont;
    Q_EMIT axisZLabelFontChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::plotAreaBackgroundColor
 *
 * \brief The color of the graph plot area background.
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::plotAreaBackgroundColor() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.plotAreaBackgroundColorCustom)
        return d->m_plotAreaBackgroundColor;
    return d->m_plotAreaBackgroundThemeColor;
}

void QGraphsTheme::setPlotAreaBackgroundColor(QColor newBackgroundColor)
{
    Q_D(QGraphsTheme);
    d->m_customBits.plotAreaBackgroundColorCustom = true;
    if (d->m_plotAreaBackgroundColor == newBackgroundColor)
        return;
    d->m_dirtyBits.plotAreaBackgroundColorDirty = true;
    d->m_plotAreaBackgroundColor = newBackgroundColor;
    Q_EMIT plotAreaBackgroundColorChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::plotAreaBackgroundVisible
 *
 * \brief Whether the plot area background is visible.
 *
 * The background is drawn by using the value of plotAreaBackgroundColor.
 * The default value is \c true.
 */
bool QGraphsTheme::isPlotAreaBackgroundVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_plotAreaBackgroundVisibility;
}

void QGraphsTheme::setPlotAreaBackgroundVisible(bool newBackgroundVisibility)
{
    Q_D(QGraphsTheme);
    if (d->m_plotAreaBackgroundVisibility == newBackgroundVisibility)
        return;
    d->m_dirtyBits.plotAreaBackgroundVisibilityDirty = true;
    d->m_plotAreaBackgroundVisibility = newBackgroundVisibility;
    Q_EMIT plotAreaBackgroundVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::backgroundVisible
 *
 * \brief Whether the background is visible.
 *
 * The background is drawn by using the value of backgroundColor.
 * The default value is \c true.
 */
bool QGraphsTheme::isBackgroundVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_backgroundVisibility;
}

void QGraphsTheme::setBackgroundVisible(bool newBackgroundVisible)
{
    Q_D(QGraphsTheme);
    if (d->m_backgroundVisibility == newBackgroundVisible)
        return;
    d->m_dirtyBits.backgroundVisibilityDirty = true;
    d->m_backgroundVisibility = newBackgroundVisible;
    Q_EMIT backgroundVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::gridVisible
 *
 * \brief Whether the grid lines are drawn.
 *
 * This value affects all grid lines.
 * The default value is \c true.
 */
bool QGraphsTheme::isGridVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_gridVisibility;
}

void QGraphsTheme::setGridVisible(bool newGridVisibility)
{
    Q_D(QGraphsTheme);
    if (d->m_gridVisibility == newGridVisibility)
        return;
    d->m_dirtyBits.gridVisibilityDirty = true;
    d->m_gridVisibility = newGridVisibility;
    Q_EMIT gridVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::backgroundColor
 *
 * \brief The color of the view the graph is drawn into.
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::backgroundColor() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.backgroundColorCustom)
        return d->m_backgroundColor;
    return d->m_backgroundThemeColor;
}

void QGraphsTheme::setBackgroundColor(QColor newBackgroundColor)
{
    Q_D(QGraphsTheme);
    d->m_customBits.backgroundColorCustom = true;
    if (d->m_backgroundColor == newBackgroundColor)
        return;
    d->m_dirtyBits.backgroundColorDirty = true;
    d->m_backgroundColor = newBackgroundColor;
    Q_EMIT backgroundColorChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::labelsVisible
 *
 * \brief Whether labels are drawn at all.
 *
 * If this is \c{false}, all other label properties have no effect.
 * The default value is \c true.
 */
bool QGraphsTheme::labelsVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_labelsVisibility;
}

void QGraphsTheme::setLabelsVisible(bool newLabelsVisibility)
{
    Q_D(QGraphsTheme);
    if (d->m_labelsVisibility == newLabelsVisibility)
        return;
    d->m_dirtyBits.labelsVisibilityDirty = true;
    d->m_labelsVisibility = newLabelsVisibility;
    Q_EMIT labelsVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::labelBackgroundColor
 *
 * \brief The color of the label backgrounds.
 *
 * Has no effect if labelBackgroundVisible is \c false.
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::labelBackgroundColor() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.labelBackgroundColorCustom)
        return d->m_labelBackgroundColor;
    return d->m_labelBackgroundThemeColor;
}

void QGraphsTheme::setLabelBackgroundColor(QColor newLabelBackgroundColor)
{
    Q_D(QGraphsTheme);
    d->m_customBits.labelBackgroundColorCustom = true;
    if (d->m_labelBackgroundColor == newLabelBackgroundColor)
        return;
    d->m_dirtyBits.labelBackgroundColorDirty = true;
    d->m_labelBackgroundColor = newLabelBackgroundColor;
    Q_EMIT labelBackgroundColorChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::labelTextColor
 *
 * \brief The color of the font used for labels.
 *
 * If an axis has specified \l{QGraphsLine::labelTextColor}{labelTextColor} explicitly,
 * this has no effect.
 *
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::labelTextColor() const
{
    Q_D(const QGraphsTheme);
    return d->m_labelTextThemeColor;
}

void QGraphsTheme::setLabelTextColor(QColor newLabelTextColor)
{
    Q_D(QGraphsTheme);
    if (d->m_labelTextThemeColor == newLabelTextColor)
        return;
    d->m_customBits.labelTextColorCustom = true;
    d->m_dirtyBits.labelTextColorDirty = true;
    d->m_labelTextThemeColor = newLabelTextColor;
    // Do not override explicitly set axis colors
    if (!d->m_axisX.d->m_bits.labelTextColorCustom) {
        axisX().d->m_labelTextThemeColor = newLabelTextColor;
        d->m_dirtyBits.axisXDirty = true;
        Q_EMIT axisXChanged();
    }
    if (!d->m_axisY.d->m_bits.labelTextColorCustom) {
        axisY().d->m_labelTextThemeColor = newLabelTextColor;
        d->m_dirtyBits.axisYDirty = true;
        Q_EMIT axisYChanged();
    }
    if (!d->m_axisZ.d->m_bits.labelTextColorCustom) {
        axisZ().d->m_labelTextThemeColor = newLabelTextColor;
        d->m_dirtyBits.axisZDirty = true;
        Q_EMIT axisZChanged();
    }
    Q_EMIT labelTextColorChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::singleHighlightColor
 *
 * \brief The highlight color for a selected object.
 *
 * Used if \l{Q3DGraphsWidgetItem::selectionMode}{selectionMode} has the
 * \c QtGraphs3D::SelectionFlag::Item flag set.
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::singleHighlightColor() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.singleHighlightColorCustom)
        return d->m_singleHighlightColor;
    return d->m_singleHighlightThemeColor;
}

void QGraphsTheme::setSingleHighlightColor(QColor newSingleHighlightColor)
{
    Q_D(QGraphsTheme);
    d->m_customBits.singleHighlightColorCustom = true;
    if (d->m_singleHighlightColor == newSingleHighlightColor)
        return;
    d->m_dirtyBits.singleHighlightColorDirty = true;
    d->m_singleHighlightColor = newSingleHighlightColor;
    Q_EMIT singleHighlightColorChanged(d->m_singleHighlightColor);
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::multiHighlightColor
 *
 * \brief The highlight color for selected objects.
 *
 * Used if \l{Q3DGraphsWidgetItem::selectionMode}{selectionMode} has the
 * \c QtGraphs3D::SelectionFlag::Row or \c QtGraphs3D::SelectionFlag::Column
 * flag set.
 * The default value depends on \l colorScheme.
 */
QColor QGraphsTheme::multiHighlightColor() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.multiHighlightColorCustom)
        return d->m_multiHighlightColor;
    return d->m_multiHighlightThemeColor;
}

void QGraphsTheme::setMultiHighlightColor(QColor newMultiHighlightColor)
{
    Q_D(QGraphsTheme);
    d->m_customBits.multiHighlightColorCustom = true;
    if (d->m_multiHighlightColor == newMultiHighlightColor)
        return;
    d->m_dirtyBits.multiHighlightColorDirty = true;
    d->m_multiHighlightColor = newMultiHighlightColor;
    Q_EMIT multiHighlightColorChanged(d->m_multiHighlightColor);
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::singleHighlightGradient
 *
 * \brief The highlight gradient for a selected object.
 *
 * Used if \l{Q3DGraphsWidgetItem::selectionMode}{selectionMode}
 * has the \c QtGraphs3D::SelectionFlag::Item flag set.
 * The default value depends on \l colorScheme.
 */
void QGraphsTheme::setSingleHighlightGradient(const QLinearGradient &gradient)
{
    Q_D(QGraphsTheme);
    d->m_customBits.singleHighlightGradientCustom = true;
    if (d->m_singleHighlightGradient == gradient)
        return;

    d->m_dirtyBits.singleHighlightGradientDirty = true;
    d->m_singleHighlightGradient = gradient;
    Q_EMIT singleHighlightGradientChanged(d->m_singleHighlightGradient);
    Q_EMIT update();
}

QLinearGradient QGraphsTheme::singleHighlightGradient() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.singleHighlightGradientCustom)
        return d->m_singleHighlightGradient;
    return d->m_singleHighlightThemeGradient;
}

/*!
 * \property QGraphsTheme::multiHighlightGradient
 *
 * \brief The highlight gradient for selected objects.
 *
 * Used if \l{Q3DGraphsWidgetItem::selectionMode}{selectionMode}
 * has the \c QtGraphs3D::SelectionFlag::Row or
 * \c QtGraphs3D::SelectionFlag::Column flag set.
 * The default value depends on \l colorScheme.
 */
void QGraphsTheme::setMultiHighlightGradient(const QLinearGradient &gradient)
{
    Q_D(QGraphsTheme);
    d->m_customBits.multiHighlightGradientCustom = true;
    if (d->m_multiHighlightGradient == gradient)
        return;

    d->m_dirtyBits.multiHighlightGradientDirty = true;
    d->m_multiHighlightGradient = gradient;
    Q_EMIT multiHighlightGradientChanged(d->m_multiHighlightGradient);
    Q_EMIT update();
}

QLinearGradient QGraphsTheme::multiHighlightGradient() const
{
    Q_D(const QGraphsTheme);
    ;
    if (d->m_customBits.multiHighlightGradientCustom)
        return d->m_multiHighlightGradient;
    return d->m_multiHighlightThemeGradient;
}

/*!
 * \property QGraphsTheme::labelFont
 *
 * \brief The font to be used for labels.
 */
QFont QGraphsTheme::labelFont() const
{
    Q_D(const QGraphsTheme);
    return d->m_labelFont;
}

void QGraphsTheme::setLabelFont(const QFont &newFont)
{
    Q_D(QGraphsTheme);
    if (d->m_labelFont == newFont)
        return;
    d->m_dirtyBits.labelFontDirty = true;
    d->m_labelFont = newFont;
    if (!d->m_customBits.axisXLabelFontCustom)
        d->m_axisXLabelFont = newFont;
    if (!d->m_customBits.axisYLabelFontCustom)
        d->m_axisYLabelFont = newFont;
    if (!d->m_customBits.axisZLabelFontCustom)
        d->m_axisZLabelFont = newFont;
    Q_EMIT labelFontChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::labelBackgroundVisible
 *
 *\brief Whether the label is drawn with a color background or with a fully
 * transparent background.
 *
 * The labelBackgroundColor value (including alpha) is used for drawing the
 * background.
 *
 * Labels with a background are drawn to equal sizes per axis based
 * on the longest label, and the text is centered in them. Labels without a
 * background are drawn as is and are left or right aligned based on their
 * position in the graph.
 * The default value is \c true.
 */
bool QGraphsTheme::isLabelBackgroundVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_labelBackgroundVisibility;
}

void QGraphsTheme::setLabelBackgroundVisible(bool newLabelBackgroundVisibility)
{
    Q_D(QGraphsTheme);
    if (d->m_labelBackgroundVisibility == newLabelBackgroundVisibility)
        return;
    d->m_dirtyBits.labelBackgroundVisibilityDirty = true;
    d->m_labelBackgroundVisibility = newLabelBackgroundVisibility;
    Q_EMIT labelBackgroundVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::labelBorderVisible
 *
 * \brief Whether label borders are drawn for labels that have a background.
 *
 * Has no effect if labelBackgroundVisible is \c false.
 * The default value is \c true.
 */
bool QGraphsTheme::isLabelBorderVisible() const
{
    Q_D(const QGraphsTheme);
    return d->m_labelBorderVisibility;
}

void QGraphsTheme::setLabelBorderVisible(bool newLabelBorderVisibility)
{
    Q_D(QGraphsTheme);
    if (d->m_labelBorderVisibility == newLabelBorderVisibility)
        return;
    d->m_dirtyBits.labelBorderVisibilityDirty = true;
    d->m_labelBorderVisibility = newLabelBorderVisibility;
    Q_EMIT labelBorderVisibleChanged();
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::seriesColors
 *
 * \brief The list of base colors to be used for all the objects in the graph,
 * series by series.
 *
 * If there are more series than colors, the color list wraps and starts again
 * with the first color in the list.
 *
 * Has no immediate effect if colorStyle is not Uniform.
 *
 * This value can be overridden by setting the \l{QAbstract3DSeries::baseColor}
 * {baseColor} explicitly in the series.
 */
QList<QColor> QGraphsTheme::seriesColors() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.seriesColorsCustom && !d->m_seriesColors.isEmpty())
        return d->m_seriesColors;
    return d->m_seriesThemeColors;
}

void QGraphsTheme::setSeriesColors(const QList<QColor> &newSeriesColors)
{
    Q_D(QGraphsTheme);
    d->m_customBits.seriesColorsCustom = true;
    if (d->m_seriesColors == newSeriesColors)
        return;
    d->m_dirtyBits.seriesColorsDirty = true;
    d->m_seriesColors = newSeriesColors;
    Q_EMIT seriesColorsChanged(d->m_seriesColors);
    Q_EMIT update();
}

/*!
 * \property QGraphsTheme::borderColors
 *
 * \brief The list of border colors to be used for all the objects in the graph,
 * series by series.
 *
 * If there are more series than colors, the color list wraps and starts again
 * with the first color in the list.
 *
 * Has no immediate effect if colorStyle is not Uniform.
 */
QList<QColor> QGraphsTheme::borderColors() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.borderColorsCustom && !d->m_borderColors.isEmpty())
        return d->m_borderColors;
    return d->m_borderThemeColors;
}

void QGraphsTheme::setBorderColors(const QList<QColor> &newBorderColors)
{
    Q_D(QGraphsTheme);
    d->m_customBits.borderColorsCustom = true;
    if (d->m_borderColors == newBorderColors)
        return;
    d->m_borderColors = newBorderColors;
    Q_EMIT borderColorsChanged();
    Q_EMIT update();
}

/*!
    Returns the list of series gradients used by theme.
    \sa setSeriesGradients()
 */
QList<QLinearGradient> QGraphsTheme::seriesGradients() const
{
    Q_D(const QGraphsTheme);
    if (d->m_customBits.seriesGradientCustom && !d->m_seriesGradients.isEmpty())
        return d->m_seriesGradients;
    return d->m_seriesThemeGradients;
}

/*!
    Sets \a newSeriesGradients as the series gradients for the theme.
    \sa seriesGradients()
 */
void QGraphsTheme::setSeriesGradients(const QList<QLinearGradient> &newSeriesGradients)
{
    Q_D(QGraphsTheme);
    d->m_customBits.seriesGradientCustom = true;
    if (newSeriesGradients.size()) {
        d->m_dirtyBits.seriesGradientDirty = true;
        if (d->m_seriesGradients != newSeriesGradients) {
            d->m_seriesGradients.clear();
            d->m_seriesGradients = newSeriesGradients;
            Q_EMIT seriesGradientsChanged(newSeriesGradients);
            Q_EMIT update();
        }
    } else {
        d->m_seriesGradients.clear();
        Q_EMIT update();
    }
}

/*!
 * \property QGraphsTheme::borderWidth
 *
 * \brief The width of borders in graph if any
 * The default value is \c 1.0.
 */
qreal QGraphsTheme::borderWidth() const
{
    Q_D(const QGraphsTheme);
    return d->m_borderWidth;
}

void QGraphsTheme::setBorderWidth(qreal newBorderWidth)
{
    Q_D(QGraphsTheme);
    if (QtPrivate::fuzzyCompare(d->m_borderWidth, newBorderWidth))
        return;
    d->m_borderWidth = newBorderWidth;
    Q_EMIT borderWidthChanged();
    Q_EMIT update();
}

void QGraphsTheme::handleBaseColorUpdate()
{
    Q_D(QGraphsTheme);
    qsizetype colorCount = d->m_colors.size();
    int changed = 0;
    // Check which one changed
    QQuickGraphsColor *color = qobject_cast<QQuickGraphsColor *>(QObject::sender());
    for (int i = 0; i < colorCount; i++) {
        if (color == d->m_colors.at(i)) {
            changed = i;
            break;
        }
    }
    // Update the changed one from the list
    QList<QColor> list = seriesColors();
    list[changed] = d->m_colors.at(changed)->color();
    // Set the changed list
    setSeriesColors(list);
}

void QGraphsTheme::handleBaseGradientUpdate()
{
    Q_D(QGraphsTheme);
    // Find out which gradient has changed, and update the list with it
    qsizetype gradientCount = d->m_gradients.size();
    int changed = 0;

    // Check which one changed
    QQuickGradient *newGradient = qobject_cast<QQuickGradient *>(QObject::sender());

    for (int i = 0; i < gradientCount; ++i) {
        if (newGradient == d->m_gradients.at(i)) {
            changed = i;
            break;
        }
    }

    // Update the changed one from the list
    QList<QLinearGradient> list = seriesGradients();
    list[changed] = convertGradient(newGradient);

    // Set the changed list
    setSeriesGradients(list);
}

void QGraphsTheme::classBegin()
{
}

void QGraphsTheme::componentComplete()
{
    Q_D(QGraphsTheme);
    d->m_componentComplete = true;
}

void QGraphsTheme::updateAutomaticColorScheme()
{
    setColorSchemePalette();
    Q_EMIT update();
}

void QGraphsTheme::setColorSchemePalette()
{
    Q_D(QGraphsTheme);
    float defaultColorLevel = 0.5f;

    Qt::ColorScheme colorScheme = Qt::ColorScheme::Unknown;

    if (d->m_colorScheme == QGraphsTheme::ColorScheme::Automatic) {
        colorScheme = QGuiApplication::styleHints()->colorScheme();
        if (colorScheme == Qt::ColorScheme::Unknown)
            colorScheme = Qt::ColorScheme::Light;
    } else if (d->m_colorScheme == QGraphsTheme::ColorScheme::Dark) {
        colorScheme = Qt::ColorScheme::Dark;
    } else if (d->m_colorScheme == QGraphsTheme::ColorScheme::Light) {
        colorScheme = Qt::ColorScheme::Light;
    }

    if (colorScheme == Qt::ColorScheme::Unknown)
        return;

    if (d->m_componentComplete) {
        // Reset all customizations which colorScheme changes
        d->m_customBits.backgroundColorCustom = false;
        d->m_customBits.plotAreaBackgroundColorCustom = false;
        d->m_customBits.labelBackgroundColorCustom = false;
        d->m_customBits.labelTextColorCustom = false;
        d->m_customBits.multiHighlightColorCustom = false;
        d->m_customBits.multiHighlightGradientCustom = false;
        d->m_customBits.singleHighlightColorCustom = false;
        d->m_customBits.singleHighlightGradientCustom = false;
        d->m_grid.d->resetCustomBits();
        d->m_axisX.d->resetCustomBits();
        d->m_axisY.d->resetCustomBits();
        d->m_axisZ.d->resetCustomBits();
    }

    if (colorScheme == Qt::ColorScheme::Dark) {
        d->m_backgroundThemeColor = QColor(QRgb(0x262626));
        d->m_plotAreaBackgroundThemeColor = QColor(QRgb(0x1F1F1F));
        d->m_labelBackgroundThemeColor = QColor(QRgb(0x2E2E2E));

        d->m_grid.d->m_mainThemeColor = QColor(QRgb(0xAEABAB));
        d->m_grid.d->m_subThemeColor = QColor(QRgb(0x6A6A6A));
        d->m_axisX.d->m_mainThemeColor = QColor(QRgb(0xAEABAB));
        d->m_axisX.d->m_subThemeColor = QColor(QRgb(0x6A6A6A));
        d->m_axisY.d->m_mainThemeColor = QColor(QRgb(0xAEABAB));
        d->m_axisY.d->m_subThemeColor = QColor(QRgb(0x6A6A6A));
        d->m_axisZ.d->m_mainThemeColor = QColor(QRgb(0xAEABAB));
        d->m_axisZ.d->m_subThemeColor = QColor(QRgb(0x6A6A6A));

        d->m_singleHighlightThemeColor = QColor(QRgb(0xDBEB00));
        d->m_multiHighlightThemeColor = QColor(QRgb(0x22D489));
        d->m_singleHighlightThemeGradient = createGradient(QColor(QRgb(0xDBEB00)),
                                                           defaultColorLevel);
        d->m_multiHighlightThemeGradient = createGradient(QColor(QRgb(0x22D489)), defaultColorLevel);

        // If a label text color has been overridden already, do not change it back
        if (!d->m_labelTextThemeColor.isValid() || !d->m_customBits.labelTextColorCustom)
            d->m_labelTextThemeColor = QColor(QRgb(0xAEAEAE));
        if (!d->m_customBits.labelTextColorCustom) {
            if (!d->m_axisX.d->m_labelTextThemeColor.isValid()
                || !d->m_axisX.d->m_bits.labelTextColorCustom) {
                d->m_axisX.d->m_labelTextThemeColor = QColor(QRgb(0xAEAEAE));
            }
            if (!d->m_axisY.d->m_labelTextThemeColor.isValid()
                || !d->m_axisY.d->m_bits.labelTextColorCustom) {
                d->m_axisY.d->m_labelTextThemeColor = QColor(QRgb(0xAEAEAE));
            }
            if (!d->m_axisZ.d->m_labelTextThemeColor.isValid()
                || !d->m_axisZ.d->m_bits.labelTextColorCustom) {
                d->m_axisZ.d->m_labelTextThemeColor = QColor(QRgb(0xAEAEAE));
            }
        }
    } else {
        d->m_backgroundThemeColor = QColor(QRgb(0xF2F2F2));
        d->m_plotAreaBackgroundThemeColor = QColor(QRgb(0xFCFCFC));
        d->m_labelBackgroundThemeColor = QColor(QRgb(0xE7E7E7));

        d->m_grid.d->m_mainThemeColor = QColor(QRgb(0x545151));
        d->m_grid.d->m_subThemeColor = QColor(QRgb(0xAFAFAF));
        d->m_axisX.d->m_mainThemeColor = QColor(QRgb(0x545151));
        d->m_axisX.d->m_subThemeColor = QColor(QRgb(0xAFAFAF));
        d->m_axisY.d->m_mainThemeColor = QColor(QRgb(0x545151));
        d->m_axisY.d->m_subThemeColor = QColor(QRgb(0xAFAFAF));
        d->m_axisZ.d->m_mainThemeColor = QColor(QRgb(0x545151));
        d->m_axisZ.d->m_subThemeColor = QColor(QRgb(0xAFAFAF));

        d->m_singleHighlightThemeColor = QColor(QRgb(0xCCDC00));
        d->m_multiHighlightThemeColor = QColor(QRgb(0x22D47B));
        d->m_singleHighlightThemeGradient = createGradient(QColor(QRgb(0xCCDC00)),
                                                           defaultColorLevel);
        d->m_multiHighlightThemeGradient = createGradient(QColor(QRgb(0x22D47B)), defaultColorLevel);

        // If a label text color has been overridden already, do not change it back
        if (!d->m_labelTextThemeColor.isValid() || !d->m_customBits.labelTextColorCustom)
            d->m_labelTextThemeColor = QColor(QRgb(0x6A6A6A));
        if (!d->m_customBits.labelTextColorCustom) {
            if (!d->m_axisX.d->m_labelTextThemeColor.isValid()
                || !d->m_axisX.d->m_bits.labelTextColorCustom) {
                d->m_axisX.d->m_labelTextThemeColor = QColor(QRgb(0x6A6A6A));
            }
            if (!d->m_axisY.d->m_labelTextThemeColor.isValid()
                || !d->m_axisY.d->m_bits.labelTextColorCustom) {
                d->m_axisY.d->m_labelTextThemeColor = QColor(QRgb(0x6A6A6A));
            }
            if (!d->m_axisZ.d->m_labelTextThemeColor.isValid()
                || !d->m_axisZ.d->m_bits.labelTextColorCustom) {
                d->m_axisZ.d->m_labelTextThemeColor = QColor(QRgb(0x6A6A6A));
            }
        }
    }

    d->m_dirtyBits.backgroundColorDirty = true;
    d->m_dirtyBits.plotAreaBackgroundColorDirty = true;
    d->m_dirtyBits.labelBackgroundColorDirty = true;
    d->m_dirtyBits.gridDirty = true;
    d->m_dirtyBits.axisXDirty = true;
    d->m_dirtyBits.axisYDirty = true;
    d->m_dirtyBits.axisZDirty = true;
    d->m_dirtyBits.singleHighlightColorDirty = true;
    d->m_dirtyBits.singleHighlightGradientDirty = true;
    d->m_dirtyBits.multiHighlightColorDirty = true;
    d->m_dirtyBits.multiHighlightGradientDirty = true;
    d->m_dirtyBits.labelTextColorDirty = true;
    Q_EMIT gridChanged();
    Q_EMIT axisXChanged();
    Q_EMIT axisYChanged();
    Q_EMIT axisZChanged();
}

void QGraphsTheme::setThemePalette()
{
    Q_D(QGraphsTheme);
    float defaultColorLevel = 0.5f;
    d->m_seriesThemeColors.clear();
    switch (d->m_theme) {
    case Theme::QtGreen:
        d->m_seriesThemeColors.append(QColor(QRgb(0xD5F8E7)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xABF2CE)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x7BE6B1)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x51E098)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x22D478)));
        break;
    case Theme::QtGreenNeon:
        d->m_seriesThemeColors.append(QColor(QRgb(0x22D478)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x00AF80)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x00897B)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x006468)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x00414A)));
        break;
    case Theme::MixSeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0xFFA615)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x5E45DF)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x759F1C)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xF92759)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x0128F8)));
        break;
    case Theme::OrangeSeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0xFFC290)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xFF9C4D)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xFF7200)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xD86000)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xA24900)));
        break;
    case Theme::YellowSeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0xFFE380)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xFFC500)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xE2B000)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xB88F00)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x8C6D02)));
        break;
    case Theme::BlueSeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0x86AFFF)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x4A86FC)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x2B6EF1)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x0750E9)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x0023DB)));
        break;
    case Theme::PurpleSeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0xE682E7)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xB646B7)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x9035B4)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x6C2BA0)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x3D2582)));
        break;
    case Theme::GreySeries:
        d->m_seriesThemeColors.append(QColor(QRgb(0xCCD0D6)));
        d->m_seriesThemeColors.append(QColor(QRgb(0xA7AEBB)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x7A869A)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x566070)));
        d->m_seriesThemeColors.append(QColor(QRgb(0x3E4654)));
        break;
    default: // UserDefined
        d->m_seriesThemeColors.append(QColor(Qt::black));
        break;
    }

    d->m_borderThemeColors = d->m_seriesThemeColors;

    d->m_seriesThemeGradients.clear();
    for (QColor color : std::as_const(d->m_seriesThemeColors))
        d->m_seriesThemeGradients.append(createGradient(color, defaultColorLevel));

    d->m_dirtyBits.seriesColorsDirty = true;
    d->m_dirtyBits.seriesGradientDirty = true;
}

QLinearGradient QGraphsTheme::createGradient(QColor color, float colorLevel)
{
    QColor startColor;
    QLinearGradient gradient = QLinearGradient(CommonUtils::maxTextureSize(),
                                               gradientTextureHeight,
                                               0.0,
                                               0.0);
    startColor.setRed(color.red() * colorLevel);
    startColor.setGreen(color.green() * colorLevel);
    startColor.setBlue(color.blue() * colorLevel);
    gradient.setColorAt(0.0, startColor);
    gradient.setColorAt(1.0, color);
    return gradient;
}

void QGraphsTheme::setThemeGradient(QQuickGradient *gradient, GradientQMLStyle type)
{
    QLinearGradient linearGradient = convertGradient(gradient);

    switch (type) {
    case GradientQMLStyle::SingleHL:
        setSingleHighlightGradient(linearGradient);
        break;
    case GradientQMLStyle::MultiHL:
        setMultiHighlightGradient(linearGradient);
        break;
    default:
        qCWarning(lcGraphsCommonTheme, "Incorrect usage. Type may be GradientQMLStyle::SingleHL or "
                 "GradientQMLStyle::MultiHL.");
        break;
    }
}

QLinearGradient QGraphsTheme::convertGradient(QQuickGradient *gradient)
{
    // Create QLinearGradient out of QQuickGradient
    QLinearGradient newGradient;
    newGradient.setStops(gradient->gradientStops());
    return newGradient;
}

/*!
    \property QGraphsTheme::baseColors

    The list of base colors to be used for the series when using the ColorStyle::Uniform
    color style.
*/
QQmlListProperty<QQuickGraphsColor> QGraphsTheme::baseColorsQML()
{
    return QQmlListProperty<QQuickGraphsColor>(this,
                                               this,
                                               &QGraphsTheme::appendBaseColorsFunc,
                                               &QGraphsTheme::countBaseColorsFunc,
                                               &QGraphsTheme::atBaseColorsFunc,
                                               &QGraphsTheme::clearBaseColorsFunc);
}

void QGraphsTheme::appendBaseColorsFunc(QQmlListProperty<QQuickGraphsColor> *list,
                                    QQuickGraphsColor *color)
{
    reinterpret_cast<QGraphsTheme *>(list->data)->addColor(color);
}

qsizetype QGraphsTheme::countBaseColorsFunc(QQmlListProperty<QQuickGraphsColor> *list)
{
    return reinterpret_cast<QGraphsTheme *>(list->data)->colorList().size();
}

QQuickGraphsColor *QGraphsTheme::atBaseColorsFunc(QQmlListProperty<QQuickGraphsColor> *list,
                                              qsizetype index)
{
    return reinterpret_cast<QGraphsTheme *>(list->data)->colorList().at(index);
}

void QGraphsTheme::clearBaseColorsFunc(QQmlListProperty<QQuickGraphsColor> *list)
{
    reinterpret_cast<QGraphsTheme *>(list->data)->clearColors();
}

/*!
    \property QGraphsTheme::baseGradients

    The list of base gradients to be used for the series when using the ColorStyle::RangeGradient
    color style.
*/
QQmlListProperty<QQuickGradient> QGraphsTheme::baseGradientsQML()
{
    return QQmlListProperty<QQuickGradient>(this,
                                            this,
                                            &QGraphsTheme::appendBaseGradientsFunc,
                                            &QGraphsTheme::countBaseGradientsFunc,
                                            &QGraphsTheme::atBaseGradientsFunc,
                                            &QGraphsTheme::clearBaseGradientsFunc);
}

void QGraphsTheme::appendBaseGradientsFunc(QQmlListProperty<QQuickGradient> *list,
                                           QQuickGradient *gradient)
{
    reinterpret_cast<QGraphsTheme *>(list->data)->addGradient(gradient);
}

qsizetype QGraphsTheme::countBaseGradientsFunc(QQmlListProperty<QQuickGradient> *list)
{
    return reinterpret_cast<QGraphsTheme *>(list->data)->gradientList().size();
}

QQuickGradient *QGraphsTheme::atBaseGradientsFunc(QQmlListProperty<QQuickGradient> *list,
                                                  qsizetype index)
{
    return reinterpret_cast<QGraphsTheme *>(list->data)->gradientList().at(index);
}

void QGraphsTheme::clearBaseGradientsFunc(QQmlListProperty<QQuickGradient> *list)
{
    reinterpret_cast<QGraphsTheme *>(list->data)->clearGradients();
}

/*!
    \property QGraphsTheme::themeChildren

    The list of theme child objects. This is a default property that allows child objects to be
    specified within the Theme element in QML without explicitly using the children property name.
*/
QQmlListProperty<QObject> QGraphsTheme::themeChildren()
{
    return QQmlListProperty<QObject>(this, this, &QGraphsTheme::appendThemeChildren, 0, 0, 0);
}

void QGraphsTheme::appendThemeChildren(QQmlListProperty<QObject> *list, QObject *element)
{
    Q_UNUSED(list);
    Q_UNUSED(element);
}

void QGraphsTheme::addColor(QQuickGraphsColor *color)
{
    if (!color) {
        qCWarning(lcGraphsCommonTheme, "Color is invalid, use Color");
        return;
    }
    clearDummyColors();
    Q_D(QGraphsTheme);
    d->m_colors.append(color);
    connect(color, &QQuickGraphsColor::colorChanged, this, &QGraphsTheme::handleBaseColorUpdate);
    QList<QColor> list = d->m_seriesColors;
    list.append(color->color());
    setSeriesColors(list);
}

QList<QQuickGraphsColor *> QGraphsTheme::colorList()
{
    Q_D(QGraphsTheme);
    if (d->m_colors.isEmpty()) {
        // Create dummy Colors from theme's colors
        d->m_dummyColors = true;
        QList<QColor> list = seriesColors();
        for (const QColor &item : std::as_const(list)) {
            QQuickGraphsColor *color = new QQuickGraphsColor(this);
            color->setColor(item);
            d->m_colors.append(color);
            connect(color, &QQuickGraphsColor::colorChanged, this, &QGraphsTheme::handleBaseColorUpdate);
        }
    }
    return d->m_colors;
}

void QGraphsTheme::clearColors()
{
    clearDummyColors();
    Q_D(QGraphsTheme);
    for (QQuickGraphsColor *item : std::as_const(d->m_colors))
        disconnect(item, 0, this, 0);
    d->m_colors.clear();
    setSeriesColors(QList<QColor>());
}

void QGraphsTheme::clearDummyColors()
{
    Q_D(QGraphsTheme);
    if (d->m_dummyColors) {
        for (QQuickGraphsColor *item : std::as_const(d->m_colors))
            delete item;
        d->m_colors.clear();
        d->m_dummyColors = false;
    }
}

void QGraphsTheme::addGradient(QQuickGradient *gradient)
{
    Q_D(QGraphsTheme);
    d->m_gradients.append(gradient);

    connect(gradient, &QQuickGradient::updated, this, &QGraphsTheme::handleBaseGradientUpdate);

    QList<QLinearGradient> list = d->m_seriesGradients;
    list.append(convertGradient(gradient));
    setSeriesGradients(list);
}

QQuickGradient *QGraphsTheme::singleHighlightGradientQML() const
{
    const Q_D(QGraphsTheme);
    return d->m_singleHLQuickGradient;
}

void QGraphsTheme::setSingleHighlightGradientQML(QQuickGradient *gradient)
{
    Q_D(QGraphsTheme);
    // connect new / disconnect old
    if (gradient != d->m_singleHLQuickGradient) {
        if (d->m_singleHLQuickGradient)
            QObject::disconnect(d->m_singleHLQuickGradient, 0, this, 0);

        d->m_singleHLQuickGradient = gradient;

        QObject::connect(d->m_singleHLQuickGradient,
                         &QQuickGradient::updated,
                         this,
                         &QGraphsTheme::update);

        Q_EMIT singleHighlightGradientQMLChanged();
    }

    if (d->m_singleHLQuickGradient != nullptr)
        setThemeGradient(d->m_singleHLQuickGradient, QGraphsTheme::GradientQMLStyle::SingleHL);
}

void QGraphsTheme::setMultiHighlightGradientQML(QQuickGradient *gradient)
{
    Q_D(QGraphsTheme);
    // connect new / disconnect old
    if (gradient != nullptr) {
        if (d->m_multiHLQuickGradient)
            QObject::disconnect(d->m_multiHLQuickGradient, 0, this, 0);

        d->m_multiHLQuickGradient = gradient;

        QObject::connect(d->m_multiHLQuickGradient,
                         &QQuickGradient::updated,
                         this,
                         &QGraphsTheme::update);

        Q_EMIT multiHighlightGradientQMLChanged();
    }

    if (d->m_multiHLQuickGradient != nullptr)
        setThemeGradient(d->m_multiHLQuickGradient, QGraphsTheme::GradientQMLStyle::MultiHL);
}

QQuickGradient *QGraphsTheme::multiHighlightGradientQML() const
{
    const Q_D(QGraphsTheme);
    return d->m_multiHLQuickGradient;
}

QList<QQuickGradient *> QGraphsTheme::gradientList()
{
    Q_D(QGraphsTheme);
    return d->m_gradients;
}

void QGraphsTheme::clearGradients()
{
    Q_D(QGraphsTheme);
    d->m_gradients.clear();
    setSeriesGradients(QList<QLinearGradient>());
}

/*!
    \property QGraphsTheme::grid

    The grid line styling properties for the graph.
*/
QGraphsLine QGraphsTheme::grid() const
{
    Q_D(const QGraphsTheme);
    return d->m_grid;
}

void QGraphsTheme::setGrid(const QGraphsLine &newGrid)
{
    Q_D(QGraphsTheme);
    if (d->m_grid == newGrid)
        return;
    d->m_grid = newGrid;
    d->m_dirtyBits.gridDirty = true;
    Q_EMIT gridChanged();
    Q_EMIT update();
}

/*!
    \property QGraphsTheme::axisX

    The X-axis line and label styling properties for the graph.
*/
QGraphsLine QGraphsTheme::axisX() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisX;
}

void QGraphsTheme::setAxisX(const QGraphsLine &newAxisX)
{
    Q_D(QGraphsTheme);
    if (d->m_axisX == newAxisX)
        return;
    d->m_axisX = newAxisX;
    d->m_dirtyBits.axisXDirty = true;
    Q_EMIT axisXChanged();
    Q_EMIT update();
}

/*!
    \property QGraphsTheme::axisY

    The Y-axis line and label styling properties for the graph.
*/
QGraphsLine QGraphsTheme::axisY() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisY;
}

void QGraphsTheme::setAxisY(const QGraphsLine &newAxisY)
{
    Q_D(QGraphsTheme);
    if (d->m_axisY == newAxisY)
        return;
    d->m_axisY = newAxisY;
    d->m_dirtyBits.axisYDirty = true;
    Q_EMIT axisYChanged();
    Q_EMIT update();
}

/*!
    \property QGraphsTheme::axisZ

    The Z-axis line and label styling properties for the graph.
*/
QGraphsLine QGraphsTheme::axisZ() const
{
    Q_D(const QGraphsTheme);
    return d->m_axisZ;
}

void QGraphsTheme::setAxisZ(const QGraphsLine &newAxisZ)
{
    Q_D(QGraphsTheme);
    if (d->m_axisZ == newAxisZ)
        return;
    d->m_axisZ = newAxisZ;
    d->m_dirtyBits.axisZDirty = true;
    Q_EMIT axisZChanged();
    Q_EMIT update();
}

QVariant QGraphsLine::create(const QJSValue &params)
{
    if (!params.isObject())
        return QVariant();

    QGraphsLine line;

    const QJSValue mainColor = params.property(QStringLiteral("mainColor"));
    if (mainColor.isString())
        line.setMainColor(QColor::fromString(mainColor.toString()));
    const QJSValue subColor = params.property(QStringLiteral("subColor"));
    if (subColor.isString())
        line.setSubColor(QColor::fromString(subColor.toString()));
    const QJSValue mainWidth = params.property(QStringLiteral("mainWidth"));
    if (mainWidth.isNumber())
        line.setMainWidth(mainWidth.toNumber());
    const QJSValue subWidth = params.property(QStringLiteral("subWidth"));
    if (subWidth.isNumber())
        line.setSubWidth(subWidth.toNumber());
    const QJSValue labelTextColor = params.property(QStringLiteral("labelTextColor"));
    if (labelTextColor.isString())
        line.setLabelTextColor(QColor::fromString(labelTextColor.toString()));

    return line;
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGraphsLinePrivate)

QGraphsLine::QGraphsLine()
    : d(new QGraphsLinePrivate)
{
}

QGraphsLine::QGraphsLine(const QGraphsLine &other)
    = default;

QGraphsLine::~QGraphsLine()
    = default;

/*!
    \property QGraphsLine::mainColor

    The color of the main grid or axis lines. If not set explicitly, the value is determined by the
    theme.
*/
QColor QGraphsLine::mainColor() const
{
    if (d->m_bits.mainColorCustom)
        return d->m_mainColor;
    return d->m_mainThemeColor;
}

void QGraphsLine::setMainColor(QColor newColor)
{
    d->m_bits.mainColorCustom = true;
    if (d->m_mainColor == newColor)
        return;
    detach();
    d->m_mainColor = newColor;
}

/*!
    \property QGraphsLine::subColor

    The color of the sub grid or axis lines. If not set explicitly, the value is determined by the
    theme.
*/
QColor QGraphsLine::subColor() const
{
    if (d->m_bits.subColorCustom)
        return d->m_subColor;
    return d->m_subThemeColor;
}

void QGraphsLine::setSubColor(QColor newColor)
{
    d->m_bits.subColorCustom = true;
    if (d->m_subColor == newColor)
        return;
    detach();
    d->m_subColor = newColor;
}

/*!
    \property QGraphsLine::mainWidth

    The width of the main grid or axis lines in pixels.
*/
qreal QGraphsLine::mainWidth() const
{
    return d->m_mainWidth;
}

void QGraphsLine::setMainWidth(qreal newWidth)
{
    if (QtPrivate::fuzzyCompare(d->m_mainWidth, newWidth))
        return;
    detach();
    d->m_mainWidth = newWidth;
}

/*!
    \property QGraphsLine::subWidth

    The width of the sub grid or axis lines in pixels.
*/
qreal QGraphsLine::subWidth() const
{
    return d->m_subWidth;
}

void QGraphsLine::setSubWidth(qreal newWidth)
{
    if (QtPrivate::fuzzyCompare(d->m_subWidth, newWidth))
        return;
    detach();
    d->m_subWidth = newWidth;
}

/*!
    \property QGraphsLine::labelTextColor

    The color of the axis label text. If not set explicitly, the value is determined by the theme.
*/
QColor QGraphsLine::labelTextColor() const
{
    if (d->m_bits.labelTextColorCustom)
        return d->m_labelTextColor;
    return d->m_labelTextThemeColor;
}

void QGraphsLine::setLabelTextColor(QColor newColor)
{
    d->m_bits.labelTextColorCustom = true;
    if (d->m_labelTextColor == newColor)
        return;
    detach();
    d->m_labelTextColor = newColor;
}

void QGraphsLine::detach()
{
    d.detach();
}

QGraphsLine &QGraphsLine::operator=(const QGraphsLine &other)
{
    QGraphsLine temp(other);
    swap(temp);
    return *this;
}

QGraphsLine::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

///////////////////////////////////////////////////////////////////////////

QGraphsLinePrivate::QGraphsLinePrivate()
    : QSharedData()
    , m_mainColor(QColor())
    , m_subColor(QColor())
    , m_mainWidth(2.0f)
    , m_subWidth(1.0f)
    , m_labelTextColor(QColor())
    , m_mainThemeColor(QColor())
    , m_subThemeColor(QColor())
    , m_labelTextThemeColor(QColor())
    , m_bits(QGraphsLineCustomField())
{}

QGraphsLinePrivate::QGraphsLinePrivate(const QGraphsLinePrivate &other)
    : QSharedData(other)
    , m_mainColor(other.m_mainColor)
    , m_subColor(other.m_subColor)
    , m_mainWidth(other.m_mainWidth)
    , m_subWidth(other.m_subWidth)
    , m_labelTextColor(other.m_labelTextColor)
    , m_mainThemeColor(other.m_mainThemeColor)
    , m_subThemeColor(other.m_subThemeColor)
    , m_labelTextThemeColor(other.m_labelTextThemeColor)
    , m_bits(other.m_bits)
{}

QGraphsLinePrivate::~QGraphsLinePrivate()
    = default;

void QGraphsLinePrivate::resetCustomBits()
{
    m_bits.mainColorCustom = false;
    m_bits.subColorCustom = false;
    m_bits.labelTextColorCustom = false;
}

QGraphsThemePrivate::QGraphsThemePrivate() {}

QGraphsThemePrivate::~QGraphsThemePrivate() {}
bool comparesEqual(const QGraphsLinePrivate &lhs, const QGraphsLinePrivate &rhs) noexcept
{
    bool ret = true;
    ret = ret && (lhs.m_bits.mainColorCustom == rhs.m_bits.mainColorCustom);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_bits.subColorCustom == rhs.m_bits.subColorCustom);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_bits.labelTextColorCustom == rhs.m_bits.labelTextColorCustom);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_mainColor == rhs.m_mainColor);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_subColor == rhs.m_subColor);
    if (!ret)
        return ret;
    ret = ret && QtPrivate::fuzzyCompare(lhs.m_mainWidth, rhs.m_mainWidth);
    if (!ret)
        return ret;
    ret = ret && QtPrivate::fuzzyCompare(lhs.m_subWidth, rhs.m_subWidth);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_labelTextColor == rhs.m_labelTextColor);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_mainThemeColor == rhs.m_mainThemeColor);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_subThemeColor == rhs.m_subThemeColor);
    if (!ret)
        return ret;
    ret = ret && (lhs.m_labelTextThemeColor == rhs.m_labelTextThemeColor);
    if (!ret)
        return ret;

    return ret;
}

QT_END_NAMESPACE

#include "moc_qgraphstheme.cpp"
