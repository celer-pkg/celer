// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcustom3dlabel_p.h"
#include "utils_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QCustom3DLabel
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QCustom3DLabel class adds a custom label to a graph.
 *
 * The text, font, position, scaling, rotation, and colors of a custom label can
 * be set. In addition, the visibility of the borders and background of the
 * label can be toggled. Colors, borders, and background are determined by the
 * active theme unless set explicitly.
 *
 * \note In scaling, the z-coordinate has no effect. Setting the same x- and
 * y-coordinates retains the original font dimensions.
 *
 * \sa Q3DGraphsWidgetItem::addCustomItem()
 */

/*!
 * \qmltype Custom3DLabel
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QCustom3DLabel
 * \inherits Custom3DItem
 * \brief Adds a custom label to a graph.
 *
 * The text, font, position, scaling, rotation, and colors of a custom label can
 * be set. In addition, the visibility of the borders and background of the
 * label can be toggled. Colors, borders, and background are determined by the
 * active theme unless set explicitly.
 *
 * \note In scaling, the z-coordinate has no effect. Setting the same x- and
 * y-coordinates retains the original font dimensions.
 */

/*! \qmlproperty string Custom3DLabel::text
 *
 * The text for the label. Rich text is not supported.
 */

/*! \qmlproperty font Custom3DLabel::font
 *
 * The font to be used for the label. Defaults to \c{Font {family: "Arial";
 * pointSize: 20}}. Special formatting (for example, outlined) is not supported.
 */

/*! \qmlproperty color Custom3DLabel::textColor
 *
 * The color for the label text. Also affects label border, if enabled. Defaults
 * to \c{"white"}.
 *
 * \sa borderVisible
 */

/*! \qmlproperty color Custom3DLabel::backgroundColor
 *
 * The color for the label background, if enabled. Defaults to \c{"gray"}.
 *
 * \sa backgroundVisible
 */

/*! \qmlproperty bool Custom3DLabel::backgroundVisible
 *
 * Defines whether the label background is visible. If set to \c{false},
 * backgroundColor has no effect. Defaults to \c{true}.
 */

/*! \qmlproperty bool Custom3DLabel::borderVisible
 *
 * Defines whether label borders are visible. Defaults to \c{true}.
 */

/*! \qmlproperty bool Custom3DLabel::facingCamera
 *
 * Defines whether the label will always face the camera. Defaults to \c{false}.
 * If set to \c{true}, \l {QCustom3DItem::}{rotation} has no effect.
 */

/*!
    \qmlsignal Custom3DLabel::textChanged(string text)

    This signal is emitted when \l text changes to \a text.
*/

/*!
    \qmlsignal Custom3DLabel::fontChanged(font font)

    This signal is emitted when \l font changes to \a font.
*/

/*!
    \qmlsignal Custom3DLabel::textColorChanged(color color)

    This signal is emitted when textColor changes to \a color.
*/

/*!
    \qmlsignal Custom3DLabel::backgroundColorChanged(color color)

    This signal is emitted when backgroundColor changes to \a color.
*/

/*!
    \qmlsignal Custom3DLabel::borderEnabledChanged(bool enabled)

    This signal is emitted when borderEnabled changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DLabel::backgroundEnabledChanged(bool enabled)

    This signal is emitted when backgroundEnabled changes to \a enabled.
*/

/*!
    \qmlsignal Custom3DLabel::facingCameraChanged(bool enabled)

    This signal is emitted when facingCamera changes to \a enabled.
*/

/*!
 * Constructs a custom 3D label with the given \a parent.
 */
QCustom3DLabel::QCustom3DLabel(QObject *parent)
    : QCustom3DItem(*(new QCustom3DLabelPrivate()), parent)
{}

/*!
 * Constructs a custom 3D label with the given \a text, \a font, \a position, \a
 * scaling, \a rotation, and optional \a parent.
 *
 * \note Setting the same x- and y-coordinates for \a scaling retains the
 * original font dimensions.
 */
QCustom3DLabel::QCustom3DLabel(const QString &text,
                               const QFont &font,
                               QVector3D position,
                               QVector3D scaling,
                               const QQuaternion &rotation,
                               QObject *parent)
    : QCustom3DItem(*(new QCustom3DLabelPrivate(text, font, position, scaling, rotation)), parent)
{}

/*!
 * Deletes the custom 3D label.
 */
QCustom3DLabel::~QCustom3DLabel() {}

/*! \property QCustom3DLabel::text
 *
 * \brief The text for the label.
 *
 * Rich text is not supported.
 */
void QCustom3DLabel::setText(const QString &text)
{
    Q_D(QCustom3DLabel);
    if (d->m_text == text) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(text));
        return;
    }
    d->m_text = text;
    emit textChanged(text);
    emit needUpdate();
}

QString QCustom3DLabel::text() const
{
    Q_D(const QCustom3DLabel);
    return d->m_text;
}

/*! \property QCustom3DLabel::font
 *
 * \brief The font to be used for the label.
 *
 * Defaults to \c{QFont("Arial", 20)}. Special formatting
 * (for example, outlined) is not supported.
 */
void QCustom3DLabel::setFont(const QFont &font)
{
    Q_D(QCustom3DLabel);
    if (d->m_font == font) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << font;
        return;
    }
    d->m_font = font;
    emit fontChanged(font);
    emit needUpdate();
}

QFont QCustom3DLabel::font() const
{
    Q_D(const QCustom3DLabel);
    return d->m_font;
}

/*! \property QCustom3DLabel::textColor
 *
 * \brief The color for the label text.
 *
 * Also affects the label border, if enabled. Defaults to \c{Qt::white}.
 *
 * \sa borderVisible
 */
void QCustom3DLabel::setTextColor(QColor color)
{
    Q_D(QCustom3DLabel);
    if (d->m_txtColor == color) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(color.name()));
        return;
    }

    d->m_txtColor = color;
    d->m_customVisuals = true;
    emit textColorChanged(color);
    emit needUpdate();
}

QColor QCustom3DLabel::textColor() const
{
    Q_D(const QCustom3DLabel);
    return d->m_txtColor;
}

/*! \property QCustom3DLabel::backgroundColor
 *
 * \brief The color for the label background, if enabled.
 *
 * Defaults to \c{Qt::gray}.
 *
 * \sa backgroundVisible
 */
void QCustom3DLabel::setBackgroundColor(QColor color)
{
    Q_D(QCustom3DLabel);
    if (d->m_bgrColor == color) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(color.name()));
        return;
    }

    d->m_bgrColor = color;
    d->m_customVisuals = true;
    emit backgroundColorChanged(color);
    emit needUpdate();
}

QColor QCustom3DLabel::backgroundColor() const
{
    Q_D(const QCustom3DLabel);
    return d->m_bgrColor;
}

/*! \property QCustom3DLabel::borderVisible
 *
 * \brief Whether label borders are visible.
 *
 * Defaults to \c{true}.
 */
void QCustom3DLabel::setBorderVisible(bool visible)
{
    Q_D(QCustom3DLabel);
    if (d->m_borders == visible) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << visible;
        return;
    }

    d->m_borders = visible;
    d->m_customVisuals = true;
    emit borderVisibleChanged(visible);
    emit needUpdate();
}

bool QCustom3DLabel::isBorderVisible() const
{
    Q_D(const QCustom3DLabel);
    return d->m_borders;
}

/*! \property QCustom3DLabel::backgroundVisible
 *
 * \brief Whether the label background is visible.
 *
 * If set to \c{false}, backgroundColor() has no effect. Defaults
 * to \c{true}.
 */
void QCustom3DLabel::setBackgroundVisible(bool visible)
{
    Q_D(QCustom3DLabel);
    if (d->m_background == visible) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << visible;
        return;
    }

    d->m_background = visible;
    d->m_customVisuals = true;
    emit backgroundVisibleChanged(visible);
    emit needUpdate();
}

bool QCustom3DLabel::isBackgroundVisible() const
{
    Q_D(const QCustom3DLabel);
    return d->m_background;
}

/*! \property QCustom3DLabel::facingCamera
 *
 * \brief Whether the label will always face the camera.
 *
 * Defaults to \c{false}. If set to \c{true}, rotation()
 * has no effect.
 */
void QCustom3DLabel::setFacingCamera(bool enabled)
{
    Q_D(QCustom3DLabel);
    if (d->m_facingCamera == enabled) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enabled;
        return;
    }
    d->m_facingCamera = enabled;
    d->m_facingCameraDirty = true;
    emit facingCameraChanged(enabled);
    emit needUpdate();
}

bool QCustom3DLabel::isFacingCamera() const
{
    Q_D(const QCustom3DLabel);
    return d->m_facingCamera;
}

QCustom3DLabelPrivate::QCustom3DLabelPrivate()
    : m_font(QFont(QStringLiteral("Arial"), 20))
    , m_bgrColor(Qt::gray)
    , m_txtColor(Qt::white)
    , m_background(true)
    , m_borders(true)
    , m_facingCamera(false)
    , m_customVisuals(false)
    , m_facingCameraDirty(false)
{
    m_isLabelItem = true;
    m_shadowCasting = false;
    m_meshFile = QStringLiteral(":/defaultMeshes/plane");
}

QCustom3DLabelPrivate::QCustom3DLabelPrivate(const QString &text,
                                             const QFont &font,
                                             QVector3D position,
                                             QVector3D scaling,
                                             const QQuaternion &rotation)
    : QCustom3DItemPrivate(QStringLiteral(":/defaultMeshes/plane"), position, scaling, rotation)
    , m_text(text)
    , m_font(font)
    , m_bgrColor(Qt::gray)
    , m_txtColor(Qt::white)
    , m_background(true)
    , m_borders(true)
    , m_facingCamera(false)
    , m_customVisuals(false)
    , m_facingCameraDirty(false)
{
    m_isLabelItem = true;
    m_shadowCasting = false;
}

QCustom3DLabelPrivate::~QCustom3DLabelPrivate() {}

void QCustom3DLabelPrivate::resetDirtyBits()
{
    QCustom3DItemPrivate::resetDirtyBits();
    m_facingCameraDirty = false;
}

QT_END_NAMESPACE

#include "moc_qcustom3dlabel.cpp"
