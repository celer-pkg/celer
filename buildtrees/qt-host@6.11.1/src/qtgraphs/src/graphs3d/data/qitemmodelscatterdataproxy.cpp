// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qitemmodelscatterdataproxy_p.h"
#include "scatteritemmodelhandler_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QItemModelScatterDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief Proxy class for presenting data in item models with Q3DScatterWidgetItem.
 *
 * QItemModelScatterDataProxy allows you to use QAbstractItemModel derived
 * models as a data source for Q3DScatterWidgetItem. It maps roles of QAbstractItemModel
 * to the XYZ-values of Q3DScatterWidgetItem points.
 *
 * The data is resolved asynchronously whenever the mapping or the model
 * changes. QScatterDataProxy::arrayReset() is emitted when the data has been
 * resolved. However, inserts, removes, and single data item changes after the
 * model initialization are resolved synchronously, unless the same frame also
 * contains a change that causes the whole model to be resolved.
 *
 * Mapping ignores rows and columns of the QAbstractItemModel and treats
 * all items equally. It requires the model to provide roles for the data items
 * that can be mapped to X, Y, and Z-values for the scatter points.
 *
 * For example, assume that you have a custom QAbstractItemModel for storing
 * various measurements done on material samples, providing data for roles such
 * as "density", "hardness", and "conductivity". You could visualize these
 * properties on a scatter graph using this proxy:
 *
 * \snippet doc_src_qtgraphs.cpp scattermodelproxy
 *
 * If the fields of the model do not contain the data in the exact format you
 * need, you can specify a search pattern regular expression and a replace rule
 * for each role to get the value in a format you need. For more information on
 * how the replacement using regular expressions works, see the
 * QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation. Note that using regular expressions has an impact on
 * performance, so it's more efficient to utilize item models where doing search
 * and replace is not necessary to get the desired values.
 *
 * For example about using the search patterns in conjunction with the roles,
 * see ItemModelBarDataProxy usage in \l{Simple Bar Graph}.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmltype ItemModelScatterDataProxy
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QItemModelScatterDataProxy
 * \inherits ScatterDataProxy
 * \brief Proxy class for presenting data in item models with Scatter3D.
 *
 * This type allows you to use AbstractItemModel derived models as a data source
 * for Scatter3D.
 *
 * The data is resolved asynchronously whenever the mapping or the model
 * changes. QScatterDataProxy::arrayReset() is emitted when the data has been
 * resolved.
 *
 * For more details, see QItemModelScatterDataProxy documentation.
 *
 * Usage example:
 *
 * \snippet doc_src_qmlgraphs.cpp 8
 *
 * \sa ScatterDataProxy, {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty model ItemModelScatterDataProxy::itemModel
 * The item model to use as a data source for Scatter3D.
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::xPosRole
 * The item model role to map into the X position.
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::yPosRole
 * The item model role to map into the Y position.
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::zPosRole
 * The item model role to map into the Z position.
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::rotationRole
 *
 * The item model role to map into item rotation.
 * The model may supply the value for rotation as either variant that is
 * directly convertible to \l [QtQuick] quaternion, or as one of the string
 * representations: \c{"scalar,x,y,z"} or \c{"@angle,x,y,z"}. The first format
 * will construct the \l [QtQuick] quaternion directly with given values, and
 * the second one will construct the \l [QtQuick] quaternion using
 * QQuaternion::fromAxisAndAngle() method.
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::scaleRole
 *
 * The model may supply the value for scale as either a variant that is
 * \c{"x,y,z"}. The first will construct the
 * vector3d directly with the given values.
 */

/*!
 * \qmlproperty regExp ItemModelScatterDataProxy::xPosRolePattern
 *
 * When set, a search and replace is done on the value mapped by the x-position
 * role before it is used as
 * an item position value. This property specifies the regular expression to
 * find the portion of the mapped value to replace, and xPosRoleReplace property
 * contains the replacement string.
 *
 * \sa xPosRole, xPosRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelScatterDataProxy::yPosRolePattern
 *
 * When set, a search and replace is done on the value mapped by the y-position
 * role before it is used as
 * an item position value. This property specifies the regular expression to
 * find the portion of the mapped value to replace, and yPosRoleReplace property
 * contains the replacement string.
 *
 * \sa yPosRole, yPosRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelScatterDataProxy::zPosRolePattern
 *
 * When set, a search and replace is done on the value mapped by the z-position
 * role before it is used as
 * an item position value. This property specifies the regular expression to
 * find the portion of the mapped value to replace, and zPosRoleReplace property
 * contains the replacement string.
 *
 * \sa zPosRole, zPosRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelScatterDataProxy::rotationRolePattern
 * When set, a search and replace is done on the value mapped by the rotation
 * role before it is used
 * as item rotation. This property specifies the regular expression to find the
 * portion of the mapped value to replace, and rotationRoleReplace property
 * contains the replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelScatterDataProxy::scaleRolePattern
 * When set, a search and replace is done on the value mapped by the scale
 * role before it is used
 * as item scale. This property specifies the regular expression to find the
 * portion of the mapped value to replace, and scaleRoleReplace property
 * contains the replacement string.
 *
 * \sa scaleRole, scaleRoleReplace
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::xPosRoleReplace
 *
 * This property defines the replacement content to be used in conjunction with
 * xPosRolePattern. Defaults to an empty string. For more information on how the
 * search and replace using regular expressions works, see
 * the QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation.
 *
 * \sa xPosRole, xPosRolePattern
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::yPosRoleReplace
 *
 * This property defines the replacement content to be used in conjunction with
 * yPosRolePattern. Defaults to an empty string. For more information on how the
 * search and replace using regular expressions works, see
 * the QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation.
 *
 * \sa yPosRole, yPosRolePattern
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::zPosRoleReplace
 *
 * This property defines the replacement content to be used in conjunction with
 * zPosRolePattern. Defaults to an empty string. For more information on how the
 * search and replace using regular expressions works, see
 * the QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation.
 *
 * \sa zPosRole, zPosRolePattern
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::rotationRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * rotationRolePattern. Defaults to an empty string. For more information on how
 * the search and replace using regular expressions works, see
 * the QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */

/*!
 * \qmlproperty string ItemModelScatterDataProxy::scaleRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * scaleRolePattern. Defaults to an empty string. For more information on how
 * the search and replace using regular expressions works, see
 * the QString::replace(const QRegularExpression &rx, const QString &after)
 * function documentation.
 *
 * \sa scaleRole, scaleRolePattern
 */

/*!
    \qmlsignal ItemModelScatterDataProxy::itemModelChanged(model itemModel)

    This signal is emitted when itemModel changes to \a itemModel.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::xPosRoleChanged(string role)

    This signal is emitted when xPosRole changes to \a role.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::yPosRoleChanged(string role)

    This signal is emitted when yPosRole changes to \a role.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::zPosRoleChanged(string role)

    This signal is emitted when zPosRole changes to \a role.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::rotationRoleChanged(string role)

    This signal is emitted when rotationRole changes to \a role.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::scaleRoleChanged(string role)

    This signal is emitted when scaleRole changes to \a role.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::xPosRolePatternChanged(regExp pattern)

    This signal is emitted when xPosRolePattern changes to \a pattern.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::yPosRolePatternChanged(regExp pattern)

    This signal is emitted when yPosRolePattern changes to \a pattern.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::zPosRolePatternChanged(regExp pattern)

    This signal is emitted when zPosRolePattern changes to \a pattern.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::rotationRolePatternChanged(regExp pattern)

    This signal is emitted when rotationRolePattern changes to \a pattern.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::scaleRolePatternChanged(regExp pattern)

    This signal is emitted when scaleRolePattern changes to \a pattern.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::rotationRoleReplaceChanged(string replace)

    This signal is emitted when rotationRoleReplace changes to \a replace.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::scaleRoleReplaceChanged(string replace)

    This signal is emitted when scaleRoleReplace changes to \a replace.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::xPosRoleReplaceChanged(string replace)

    This signal is emitted when xPosRoleReplace changes to \a replace.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::yPosRoleReplaceChanged(string replace)

    This signal is emitted when yPosRoleReplace changes to \a replace.
*/

/*!
    \qmlsignal ItemModelScatterDataProxy::zPosRoleReplaceChanged(string replace)

    This signal is emitted when zPosRoleReplace changes to \a replace.
*/

/*!
 * Constructs QItemModelScatterDataProxy with optional \a parent.
 */
QItemModelScatterDataProxy::QItemModelScatterDataProxy(QObject *parent)
    : QScatterDataProxy(*(new QItemModelScatterDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelScatterDataProxy);
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelScatterDataProxy with \a itemModel and an optional \a
 * parent. Proxy doesn't take ownership of the \a itemModel, as typically item
 * models are owned by other controls.
 */
QItemModelScatterDataProxy::QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                                       QObject *parent)
    : QScatterDataProxy(*(new QItemModelScatterDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelScatterDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelScatterDataProxy with \a itemModel and an optional
 * \a parent. The proxy doesn't take ownership of the \a itemModel, as item
 * models are typically owned by other controls. The xPosRole property is set
 * to \a xPosRole, the yPosRole property to \a yPosRole, and the zPosRole
 * property to \a zPosRole.
 */
QItemModelScatterDataProxy::QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                                       const QString &xPosRole,
                                                       const QString &yPosRole,
                                                       const QString &zPosRole,
                                                       QObject *parent)
    : QScatterDataProxy(*(new QItemModelScatterDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelScatterDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_xPosRole = xPosRole;
    d->m_yPosRole = yPosRole;
    d->m_zPosRole = zPosRole;
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelScatterDataProxy with \a itemModel and an optional \a
 * parent. The proxy doesn't take ownership of the \a itemModel, as item models
 * are typically owned by other controls. The xPosRole property is set to \a
 * xPosRole, the yPosRole property to \a yPosRole, the zPosRole property to \a
 * zPosRole, and the rotationRole property to \a rotationRole.
 */
QItemModelScatterDataProxy::QItemModelScatterDataProxy(QAbstractItemModel *itemModel,
                                                       const QString &xPosRole,
                                                       const QString &yPosRole,
                                                       const QString &zPosRole,
                                                       const QString &rotationRole,
                                                       QObject *parent)
    : QScatterDataProxy(*(new QItemModelScatterDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelScatterDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_xPosRole = xPosRole;
    d->m_yPosRole = yPosRole;
    d->m_zPosRole = zPosRole;
    d->m_rotationRole = rotationRole;
    d->connectItemModelHandler();
}

/*!
 * Destroys QItemModelScatterDataProxy.
 */
QItemModelScatterDataProxy::~QItemModelScatterDataProxy() {}

/*!
 * \property QItemModelScatterDataProxy::itemModel
 *
 * \brief The item model to use as a data source for a 3D scatter series.
 */

/*!
 * Sets \a itemModel as the item model for Q3DScatterWidgetItem. Does not take
 * ownership of the model, but does connect to it to listen for changes.
 */
void QItemModelScatterDataProxy::setItemModel(QAbstractItemModel *itemModel)
{
    Q_D(QItemModelScatterDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
}

QAbstractItemModel *QItemModelScatterDataProxy::itemModel() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_itemModelHandler->itemModel();
}

/*!
 * \property QItemModelScatterDataProxy::xPosRole
 *
 * \brief The item model role to map into the X position.
 */
void QItemModelScatterDataProxy::setXPosRole(const QString &role)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_xPosRole == role) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(role));
        return;
    }
    d->m_xPosRole = role;
    emit xPosRoleChanged(role);
}

QString QItemModelScatterDataProxy::xPosRole() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_xPosRole;
}

/*!
 * \property QItemModelScatterDataProxy::yPosRole
 *
 * \brief The item model role to map into the Y position.
 */
void QItemModelScatterDataProxy::setYPosRole(const QString &role)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_yPosRole == role) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(role));
        return;
    }
    d->m_yPosRole = role;
    emit yPosRoleChanged(role);
}

QString QItemModelScatterDataProxy::yPosRole() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_yPosRole;
}

/*!
 * \property QItemModelScatterDataProxy::zPosRole
 *
 * \brief The item model role to map into the Z position.
 */
void QItemModelScatterDataProxy::setZPosRole(const QString &role)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_zPosRole == role) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(role));
        return;
    }
    d->m_zPosRole = role;
    emit zPosRoleChanged(role);
}

QString QItemModelScatterDataProxy::zPosRole() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_zPosRole;
}

/*!
 * \property QItemModelScatterDataProxy::rotationRole
 *
 * \brief The item model role to map into item rotation.
 *
 * The model may supply the value for rotation as either a variant that is
 * directly convertible to QQuaternion or as one of the string representations:
 * \c{"scalar,x,y,z"} or \c{"@angle,x,y,z"}. The first will construct the
 * quaternion directly with the given values, and the second one will construct
 * the quaternion using the QQuaternion::fromAxisAndAngle() method.
 */
void QItemModelScatterDataProxy::setRotationRole(const QString &role)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_rotationRole == role) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(role));
        return;
    }
    d->m_rotationRole = role;
    emit rotationRoleChanged(role);
}

QString QItemModelScatterDataProxy::rotationRole() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_rotationRole;
}

/*!
 * \property QItemModelScatterDataProxy::scaleRole
 *
 * \brief The item model role to map into item scale.
 *
 * The model may supply the value for scale as either a variant that is
 * \c{"x,y,z"}. The first will construct the
 * vector3d directly with the given values.
 */
void QItemModelScatterDataProxy::setScaleRole(const QString &role)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_scaleRole != role) {
        d->m_scaleRole = role;
        emit scaleRoleChanged(role);
    }
}

QString QItemModelScatterDataProxy::scaleRole() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_scaleRole;
}

/*!
 * \property QItemModelScatterDataProxy::xPosRolePattern
 *
 * \brief Whether search and replace is done on the value mapped by the x
 * position role before it is used as an item position value.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and xPosRoleReplace property contains the replacement
 * string.
 *
 * \sa xPosRole, xPosRoleReplace
 */
void QItemModelScatterDataProxy::setXPosRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_xPosRolePattern == pattern) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << pattern;
        return;
    }
    d->m_xPosRolePattern = pattern;
    emit xPosRolePatternChanged(pattern);
}

QRegularExpression QItemModelScatterDataProxy::xPosRolePattern() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_xPosRolePattern;
}

/*!
 * \property QItemModelScatterDataProxy::yPosRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by the
 * y position role before it is used as an item position value.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and yPosRoleReplace property contains the replacement
 * string.
 *
 * \sa yPosRole, yPosRoleReplace
 */
void QItemModelScatterDataProxy::setYPosRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_yPosRolePattern == pattern) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << pattern;
        return;
    }
    d->m_yPosRolePattern = pattern;
    emit yPosRolePatternChanged(pattern);
}

QRegularExpression QItemModelScatterDataProxy::yPosRolePattern() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_yPosRolePattern;
}

/*!
 * \property QItemModelScatterDataProxy::zPosRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by the z
 * position role before it is used as an item position value.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and zPosRoleReplace property contains the replacement
 * string.
 *
 * \sa zPosRole, zPosRoleReplace
 */
void QItemModelScatterDataProxy::setZPosRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_zPosRolePattern == pattern) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << pattern;
        return;
    }
    d->m_zPosRolePattern = pattern;
    emit zPosRolePatternChanged(pattern);
}

QRegularExpression QItemModelScatterDataProxy::zPosRolePattern() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_zPosRolePattern;
}

/*!
 * \property QItemModelScatterDataProxy::rotationRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by the
 * rotation role before it is used as item rotation.
 *
 * This property specifies the regular expression to find the portion
 * of the mapped value to replace and rotationRoleReplace property contains the
 * replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */
void QItemModelScatterDataProxy::setRotationRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_rotationRolePattern == pattern) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << pattern;
        return;
    }
    d->m_rotationRolePattern = pattern;
    emit rotationRolePatternChanged(pattern);
}

QRegularExpression QItemModelScatterDataProxy::rotationRolePattern() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_rotationRolePattern;
}

/*!
 * \property QItemModelScatterDataProxy::scaleRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by the
 * scale role before it is used as item scale.
 *
 * This property specifies the regular expression to find the portion
 * of the mapped value to replace and scaleRoleReplace property contains the
 * replacement string.
 *
 * \sa scaleRole, scaleRoleReplace
 */
void QItemModelScatterDataProxy::setScaleRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_scaleRolePattern != pattern) {
        d->m_scaleRolePattern = pattern;
        emit scaleRolePatternChanged(pattern);
    }
}

QRegularExpression QItemModelScatterDataProxy::scaleRolePattern() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_scaleRolePattern;
}

/*!
 * \property QItemModelScatterDataProxy::xPosRoleReplace
 *
 * \brief The replace content to be used in conjunction with the x position role
 * pattern.
 *
 * Defaults to an empty string. For more information on how the search and
 * replace using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa xPosRole, xPosRolePattern
 */
void QItemModelScatterDataProxy::setXPosRoleReplace(const QString &replace)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_xPosRoleReplace == replace) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(replace));
        return;
    }
    d->m_xPosRoleReplace = replace;
    emit xPosRoleReplaceChanged(replace);
}

QString QItemModelScatterDataProxy::xPosRoleReplace() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_xPosRoleReplace;
}

/*!
 * \property QItemModelScatterDataProxy::yPosRoleReplace
 *
 * \brief The replace content to be used in conjunction with the y position role
 * pattern.
 *
 * Defaults to an empty string. For more information on how the search and
 * replace using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa yPosRole, yPosRolePattern
 */
void QItemModelScatterDataProxy::setYPosRoleReplace(const QString &replace)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_yPosRoleReplace == replace) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(replace));
        return;
    }
    d->m_yPosRoleReplace = replace;
    emit yPosRoleReplaceChanged(replace);
}

QString QItemModelScatterDataProxy::yPosRoleReplace() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_yPosRoleReplace;
}

/*!
 * \property QItemModelScatterDataProxy::zPosRoleReplace
 *
 * \brief The replace content to be used in conjunction with the z position role
 * pattern.
 *
 * Defaults to an empty string. For more information on how the search and
 * replace using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa zPosRole, zPosRolePattern
 */
void QItemModelScatterDataProxy::setZPosRoleReplace(const QString &replace)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_zPosRoleReplace == replace) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(replace));
        return;
    }
    d->m_zPosRoleReplace = replace;
    emit zPosRoleReplaceChanged(replace);
}

QString QItemModelScatterDataProxy::zPosRoleReplace() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_zPosRoleReplace;
}

/*!
 * \property QItemModelScatterDataProxy::rotationRoleReplace
 *
 * \brief The replace content to be used in conjunction with the rotation role
 * pattern.
 *
 * Defaults to an empty string. For more information on how the search and
 * replace using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */
void QItemModelScatterDataProxy::setRotationRoleReplace(const QString &replace)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_rotationRoleReplace == replace) {
        qCDebug(lcProperties3D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(replace));
        return;
    }
    d->m_rotationRoleReplace = replace;
    emit rotationRoleReplaceChanged(replace);
}

QString QItemModelScatterDataProxy::rotationRoleReplace() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_rotationRoleReplace;
}

/*!
 * \property QItemModelScatterDataProxy::scaleRoleReplace
 *
 * \brief The replace content to be used in conjunction with the scale role
 * pattern.
 *
 * Defaults to an empty string. For more information on how the search and
 * replace using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa scaleRole, scaleRolePattern
 */
void QItemModelScatterDataProxy::setScaleRoleReplace(const QString &replace)
{
    Q_D(QItemModelScatterDataProxy);
    if (d->m_scaleRoleReplace != replace) {
        d->m_scaleRoleReplace = replace;
        emit scaleRoleReplaceChanged(replace);
    }
}

QString QItemModelScatterDataProxy::scaleRoleReplace() const
{
    Q_D(const QItemModelScatterDataProxy);
    return d->m_scaleRoleReplace;
}

/*!
 * Changes \a xPosRole, \a yPosRole, \a zPosRole, and \a rotationRole mapping.
 */
void QItemModelScatterDataProxy::remap(const QString &xPosRole,
                                       const QString &yPosRole,
                                       const QString &zPosRole,
                                       const QString &rotationRole)
{
    setXPosRole(xPosRole);
    setYPosRole(yPosRole);
    setZPosRole(zPosRole);
    setRotationRole(rotationRole);
}

/*!
 * Changes \a xPosRole, \a yPosRole, \a zPosRole, \a rotationRole, and \a scaleRole mapping.
 */
void QItemModelScatterDataProxy::remap(const QString &xPosRole,
                                       const QString &yPosRole,
                                       const QString &zPosRole,
                                       const QString &rotationRole,
                                       const QString &scaleRole)
{
    setXPosRole(xPosRole);
    setYPosRole(yPosRole);
    setZPosRole(zPosRole);
    setRotationRole(rotationRole);
    setScaleRole(scaleRole);
}

// QItemModelScatterDataProxyPrivate

QItemModelScatterDataProxyPrivate::QItemModelScatterDataProxyPrivate(QItemModelScatterDataProxy *q)
    : m_itemModelHandler(new ScatterItemModelHandler(q))
{}

QItemModelScatterDataProxyPrivate::~QItemModelScatterDataProxyPrivate()
{
    delete m_itemModelHandler;
}

void QItemModelScatterDataProxyPrivate::connectItemModelHandler()
{
    Q_Q(QItemModelScatterDataProxy);
    QObject::connect(m_itemModelHandler,
                     &ScatterItemModelHandler::itemModelChanged,
                     q,
                     &QItemModelScatterDataProxy::itemModelChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::xPosRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::yPosRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::zPosRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::rotationRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::scaleRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::xPosRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::yPosRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::zPosRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::rotationRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::scaleRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::xPosRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::yPosRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::zPosRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::rotationRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelScatterDataProxy::scaleRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
}

QT_END_NAMESPACE

#include "moc_qitemmodelscatterdataproxy.cpp"
