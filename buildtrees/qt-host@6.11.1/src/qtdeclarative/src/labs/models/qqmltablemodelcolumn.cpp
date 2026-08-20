// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qqmltablemodelcolumn_p.h"

#include <QtQml/qqmlinfo.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TableModelColumn
//!  \nativetype QQmlTableModelColumn
    \inqmlmodule Qt.labs.qmlmodels
    \brief Represents a column in a model.
    \since 5.14

    The TableModelColumn class represents columns in TableModel.
    TableModel supports JavaScript/JSON data where each row is an object,
    a list of simple key-value pairs where the keys are unordered.

    \code
    {
        // Each property is one cell/column.
        checked: false,
        amount: 1,
        fruitType: "Apple",
        fruitName: "Granny Smith",
        fruitPrice: 1.50
    },
    // ...
    \endcode

    However, models in Qt are manipulated via row and column indices. Specifying
    the columns with TableModelColumn allows a mapping between Qt's built-in
    roles to any property in each row object.

    \snippet qml/tablemodel/fruit-example-simpledelegate.qml file

    TableModelColumn also has basic read-only support for complex rows. For more
    information, see \l {Supported Row Data Structures}.

    \note Most of the above concepts also apply to TreeModel, except in
    TreeModel each row represents a node of the tree.

    \section1 Supported Roles

    TableModelColumn supports all of \l {Qt::ItemDataRole}{Qt's roles},
    with the exception of \c Qt::InitialSortOrderRole.
    Roles can be accessed by as listed below, e.g.
    \code
    text: display

    required property string display
    \endcode

    \table
    \row \li Qt::DisplayRole \li display
    \row \li Qt::DecorationRole \li decoration
    \row \li Qt::EditRole \li edit
    \row \li Qt::ToolTipRole \li toolTip
    \row \li Qt::StatusTipRole \li statusTip
    \row \li Qt::WhatsThisRole \li whatsThis
    \row \li Qt::FontRole \li font
    \row \li Qt::TextAlignmentRole \li textAlignment
    \row \li Qt::BackgroundRole \li background
    \row \li Qt::ForegroundRole \li foreground
    \row \li Qt::CheckStateRole \li checkState
    \row \li Qt::AccessibleTextRole \li accessibleText
    \row \li Qt::AccessibleDescriptionRole \li accessibleDescription
    \row \li Qt::SizeHintRole \li sizeHintRoleNam
    \endtable

    \sa TableModel, TableView
*/

static constexpr QLatin1StringView displayRoleName("display");
static constexpr QLatin1StringView decorationRoleName("decoration");
static constexpr QLatin1StringView editRoleName("edit");
static constexpr QLatin1StringView toolTipRoleName("toolTip");
static constexpr QLatin1StringView statusTipRoleName("statusTip");
static constexpr QLatin1StringView whatsThisRoleName("whatsThis");

static constexpr QLatin1StringView fontRoleName("font");
static constexpr QLatin1StringView textAlignmentRoleName("textAlignment");
static constexpr QLatin1StringView backgroundRoleName("background");
static constexpr QLatin1StringView foregroundRoleName("foreground");
static constexpr QLatin1StringView checkStateRoleName("checkState");

static constexpr QLatin1StringView accessibleTextRoleName("accessibleText");
static constexpr QLatin1StringView accessibleDescriptionRoleName("accessibleDescription");

static constexpr QLatin1StringView sizeHintRoleName("sizeHint");


QQmlTableModelColumn::QQmlTableModelColumn(QObject *parent)
    : QObject(parent)
{
}

QQmlTableModelColumn::~QQmlTableModelColumn()
{
}

#define DEFINE_ROLE_PROPERTIES(getterGetterName, getterSetterName, getterSignal, roleName) \
QJSValue QQmlTableModelColumn::getterGetterName() const \
{ \
    return mGetters.value(roleName); \
} \
\
void QQmlTableModelColumn::getterSetterName(const QJSValue &stringOrFunction) \
{ \
    if (!stringOrFunction.isString() && !stringOrFunction.isCallable()) { \
        qmlWarning(this).quote() << "getter for " << roleName << " must be a function"; \
        return; \
    } \
    if (stringOrFunction.strictlyEquals(decoration())) \
        return; \
\
    mGetters[roleName] = stringOrFunction; \
    emit decorationChanged(); \
}

DEFINE_ROLE_PROPERTIES(display, setDisplay, displayChanged,
    displayRoleName)
DEFINE_ROLE_PROPERTIES(decoration, setDecoration, decorationChanged,
    decorationRoleName)
DEFINE_ROLE_PROPERTIES(edit, setEdit, editChanged,
    editRoleName)
DEFINE_ROLE_PROPERTIES(toolTip, setToolTip, toolTipChanged,
    toolTipRoleName)
DEFINE_ROLE_PROPERTIES(statusTip, setStatusTip, statusTipChanged,
    statusTipRoleName)
DEFINE_ROLE_PROPERTIES(whatsThis, setWhatsThis, whatsThisChanged,
    whatsThisRoleName)

DEFINE_ROLE_PROPERTIES(font, setFont, fontChanged,
    fontRoleName)
DEFINE_ROLE_PROPERTIES(textAlignment, setTextAlignment, textAlignmentChanged,
    textAlignmentRoleName)
DEFINE_ROLE_PROPERTIES(background, setBackground, backgroundChanged,
    backgroundRoleName)
DEFINE_ROLE_PROPERTIES(foreground, setForeground, foregroundChanged,
    foregroundRoleName)
DEFINE_ROLE_PROPERTIES(checkState, setCheckState, checkStateChanged,
    checkStateRoleName)

DEFINE_ROLE_PROPERTIES(accessibleText, setAccessibleText, accessibleTextChanged,
    accessibleTextRoleName)
DEFINE_ROLE_PROPERTIES(accessibleDescription, setAccessibleDescription, accessibleDescriptionChanged,
    accessibleDescriptionRoleName)

DEFINE_ROLE_PROPERTIES(sizeHint, setSizeHint, sizeHintChanged,
    sizeHintRoleName)

QJSValue QQmlTableModelColumn::getterAtRole(const QString &roleName)
{
    auto it = mGetters.find(roleName);
    if (it == mGetters.end())
        return QJSValue();
    return *it;
}

const QHash<QString, QJSValue> QQmlTableModelColumn::getters() const
{
    return mGetters;
}

const QHash<int, QString> QQmlTableModelColumn::supportedRoleNames()
{
    static const QHash<int, QString> names {
        {Qt::DisplayRole, displayRoleName},
        {Qt::DecorationRole, decorationRoleName},
        {Qt::EditRole, editRoleName},
        {Qt::ToolTipRole, toolTipRoleName},
        {Qt::StatusTipRole, statusTipRoleName},
        {Qt::WhatsThisRole, whatsThisRoleName},
        {Qt::FontRole, fontRoleName},
        {Qt::TextAlignmentRole, textAlignmentRoleName},
        {Qt::BackgroundRole, backgroundRoleName},
        {Qt::ForegroundRole, foregroundRoleName},
        {Qt::CheckStateRole, checkStateRoleName},
        {Qt::AccessibleTextRole, accessibleTextRoleName},
        {Qt::AccessibleDescriptionRole, accessibleDescriptionRoleName},
        {Qt::SizeHintRole, sizeHintRoleName}
    };
    return names;
}

QT_END_NAMESPACE

#include "moc_qqmltablemodelcolumn_p.cpp"
