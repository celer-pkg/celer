// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qqmlformatsettings_p.h"

#include <QCommandLineParser>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::StringLiterals;

QQmlFormatSettings::QQmlFormatSettings(const QString &toolName) : QQmlToolingSettings(toolName)
{
    addOption(s_useTabsSetting, false);
    addOption(s_indentWidthSetting, 4);
    addOption(s_maxColumnWidthSetting, -1);
    addOption(s_normalizeSetting, false);
    addOption(s_newlineSetting, u"native"_s,
              QStringList { u"unix"_s, u"windows"_s, u"macos"_s, u"native"_s });
    addOption(s_objectsSpacingSetting, false);
    addOption(s_functionsSpacingSetting, false);
    addOption(s_groupAttributesTogetherSetting, false);
    addOption(s_sortImportsSetting, false);
    addOption(s_singleLineEmptyObjectsSetting, false);
    addOption(s_semiColonRuleSetting, u"always"_s, QStringList{ u"always"_s, u"essential"_s });
}

void QQmlFormatSettings::addOption(const QString &name, const QVariant &defaultValue, const QStringList &allowedValues)
{
    QQmlToolingSettings::addOption(name, defaultValue);
    if (defaultValue.typeId() == QMetaType::QString) {
        Q_ASSERT(!allowedValues.isEmpty());
        m_allowedValues[name] = allowedValues;
    }
}

bool QQmlFormatSettings::outputOptions() const
{
    QJsonObject root;
    QJsonArray optionsArray;
    for (auto it = m_values.constBegin(); it != m_values.constEnd(); ++it) {
        QJsonObject option;
        option[u"name"] = it.key();
        option[u"value"] = QJsonValue::fromVariant(it.value());
        option[u"hint"] = QString::fromUtf8(it.value().typeName());

        if (it.value().typeId() == QMetaType::QString)
            option[u"hint"] = m_allowedValues[it.key()].join(u',');

        optionsArray.append(option);
    }
    root[u"options"] = optionsArray;

    QCommandLineParser::showMessageAndExit(
            QCommandLineParser::MessageType::Information,
            QString::fromUtf8(QJsonDocument(root).toJson()), EXIT_SUCCESS);
    return true;
}
