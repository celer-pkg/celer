// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qspimatchrulematcher_p.h"

#if QT_CONFIG(accessibility)

#include "atspiadaptor_p.h"
#include "qspi_constant_mappings_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcAccessibilityAtspi, "qt.accessibility.atspi")

QSpiMatchRuleMatcher::QSpiMatchRuleMatcher(const QSpiMatchRule &matchRule)
    : m_states(spiStatesFromSpiStateSet(matchRule.states)),
      m_stateMatchType(matchRule.stateMatchType),
      m_attributes(matchRule.attributes),
      m_attributeMatchType(matchRule.attributeMatchType),
      m_roleMatchType(matchRule.roleMatchType),
      m_interfaceMatchType(matchRule.interfaceMatchType)
{
    // extract roles encoded in bitset stored in multiple 32 bit integers
    for (qsizetype i = 0; i < matchRule.roles.size(); ++i) {
        const auto role = matchRule.roles.at(i);
        for (int j = 0; j < 32; j++) {
            if (role & (1 << j)) {
                const auto atspiRole = i * 32 + j;
                if (atspiRole < ATSPI_ROLE_LAST_DEFINED)
                    m_roles.insert(AtspiRole(atspiRole));
                else
                    qCWarning(lcAccessibilityAtspi)
                            << "Ignoring invalid AT-SPI role value" << atspiRole;
            }
        }
    }

    // use qualified interface names to match what AtSpiAdaptor::accessibleInterfaces returns
    m_interfaces.reserve(matchRule.interfaces.size());
    for (const QString &ifaceName : matchRule.interfaces)
        m_interfaces.push_back("org.a11y.atspi."_L1 + ifaceName);
}

bool QSpiMatchRuleMatcher::matchAttributes(QAccessibleInterface &iface) const
{
    switch (m_attributeMatchType) {
    case ATSPI_Collection_MATCH_EMPTY:
        if (m_attributes.empty())
            return AtSpiAdaptor::getAttributes(&iface).isEmpty();
        [[fallthrough]];
    case ATSPI_Collection_MATCH_ALL: {
        if (m_attributes.empty())
            return true;
        const QSpiAttributeSet attributes = AtSpiAdaptor::getAttributes(&iface);
        for (const auto &[key, value] : m_attributes.asKeyValueRange()) {
            if (!attributes.contains(key) || attributes[key] != value)
                return false;
        }
        return true;
    }
    case ATSPI_Collection_MATCH_ANY: {
        const QSpiAttributeSet attributes = AtSpiAdaptor::getAttributes(&iface);
        for (const auto &[key, value] : m_attributes.asKeyValueRange()) {
            if (attributes.contains(key) && attributes[key] == value)
                return true;
        }
        return false;
    }
    case ATSPI_Collection_MATCH_NONE: {
        const QSpiAttributeSet attributes = AtSpiAdaptor::getAttributes(&iface);
        for (const auto &[key, value] : m_attributes.asKeyValueRange()) {
            if (attributes.contains(key) && attributes[key] == value)
                return false;
        }
        return true;
    }
    default:
        qCWarning(lcAccessibilityAtspi)
                << "QSpiMatchRuleMatcher::matchAttributes called with invalid match type "
                << m_attributeMatchType;
        return false;
    }
}

bool QSpiMatchRuleMatcher::matchInterfaces(QAccessibleInterface &iface) const
{
    switch (m_interfaceMatchType) {
    case ATSPI_Collection_MATCH_EMPTY:
        if (m_interfaces.empty())
            return AtSpiAdaptor::accessibleInterfaces(&iface).isEmpty();
        [[fallthrough]];
    case ATSPI_Collection_MATCH_ALL: {
        if (m_interfaces.empty())
            return true;
        const QStringList interfaces = AtSpiAdaptor::accessibleInterfaces(&iface);
        for (const QString &atSpiInterface : m_interfaces) {
            if (!interfaces.contains(atSpiInterface))
                return false;
        }
        return true;
    }
    case ATSPI_Collection_MATCH_ANY: {
        const QStringList interfaces = AtSpiAdaptor::accessibleInterfaces(&iface);
        for (const QString &atSpiInterface : m_interfaces) {
            if (interfaces.contains(atSpiInterface))
                return true;
        }
        return false;
    }
    case ATSPI_Collection_MATCH_NONE: {
        const QStringList interfaces = AtSpiAdaptor::accessibleInterfaces(&iface);
        for (const QString &atSpiInterface : m_interfaces) {
            if (interfaces.contains(atSpiInterface))
                return false;
        }
        return true;
    }
    default:
        qCWarning(lcAccessibilityAtspi)
                << "QSpiMatchRuleMatcher::matchInterfaces called with invalid match type "
                << m_interfaceMatchType;
        return false;
    }
}

bool QSpiMatchRuleMatcher::matchRoles(QAccessibleInterface &iface) const
{
    switch (m_roleMatchType) {
    case ATSPI_Collection_MATCH_EMPTY:
        if (m_roles.empty())
            // accessible always has exactly one role, i.e. can't have no roles
            return false;
        [[fallthrough]];
    case ATSPI_Collection_MATCH_ALL:
        if (m_roles.empty())
            return true;
        if (m_roles.size() > 1)
            // accessible only has a single role
            return false;
        [[fallthrough]];
    case ATSPI_Collection_MATCH_ANY:
        return m_roles.find(AtSpiAdaptor::getRole(&iface)) != m_roles.end();
    case ATSPI_Collection_MATCH_NONE:
        return m_roles.find(AtSpiAdaptor::getRole(&iface)) == m_roles.end();
    default:
        qCWarning(lcAccessibilityAtspi)
                << "QSpiMatchRuleMatcher::matchRoles called with invalid match type "
                << m_roleMatchType;
        return false;
    }
}

bool QSpiMatchRuleMatcher::matchStates(QAccessibleInterface &iface) const
{
    switch (m_stateMatchType) {
    case ATSPI_Collection_MATCH_EMPTY:
        if (m_states == 0)
            return spiStatesFromQState(iface.state()) == 0;
        [[fallthrough]];
    case ATSPI_Collection_MATCH_ALL:
        return (spiStatesFromQState(iface.state()) & m_states) == m_states;
    case ATSPI_Collection_MATCH_ANY:
        return (spiStatesFromQState(iface.state()) & m_states) != 0;
    case ATSPI_Collection_MATCH_NONE:
        return (spiStatesFromQState(iface.state()) & m_states) == 0;
    default:
        qCWarning(lcAccessibilityAtspi)
                << "QSpiMatchRuleMatcher::matchStates called with invalid match type "
                << m_stateMatchType;
        return false;
    }
}

bool QSpiMatchRuleMatcher::match(QAccessibleInterface &iface) const
{
    return matchRoles(iface) && matchStates(iface) && matchInterfaces(iface)
        && matchAttributes(iface);
}

QT_END_NAMESPACE

#endif // QT_CONFIG(accessibility)
