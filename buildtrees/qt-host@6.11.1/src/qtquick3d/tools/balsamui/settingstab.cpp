// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "settingstab.h"

#include <QVBoxLayout>
#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>
#include <QtWidgets>

SettingsTab::SettingsTab(QWidget *parent) : QScrollArea(parent)
{
    QWidget *central = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    const auto allDescs = QSSGAssetImportManager().getImporterPluginInfos();

    for (const auto &plugin : allDescs) {
        auto options = plugin.importOptions;
        // Skip plugins without options
        if (options.isEmpty())
            continue;

        if (auto optionsIt = options.constFind(QLatin1String("options")); optionsIt != options.constEnd())
            options = optionsIt->toObject();

        QGroupBox *pluginGroup = new QGroupBox(plugin.typeDescription + " (" + plugin.name + ")");
        QFormLayout *extensionsLayout = new QFormLayout;
        extensionsLayout->addRow(new QLabel(tr("Supported extensions:")), new QLabel(plugin.inputExtensions.join(", ")));

        for (auto kv_it = options.constBegin(); kv_it != options.constEnd(); ++kv_it) {
            if (kv_it->isObject()) {
                auto map = kv_it->toObject();
                auto settingKeyName = kv_it.key();
                auto description = map.value("description");
                auto name = map.value(QLatin1String("name"));
                auto type = map.value("type");
                auto value = map.value("value");

                auto label = new QLabel(name.toString() + ":");
                label->setToolTip(description.toString());

                if (type == "Boolean") {
                    QCheckBox *checkBox = new QCheckBox();
                    checkBox->setChecked(value.toBool());
                    extensionsLayout->addRow(label, checkBox);
                    settings.push_back(Setting { checkBox, settingKeyName, value.toBool(), 0.0f });
                } else if (type == "Real") {
                    QSpinBox *spinBox = new QSpinBox();
                    spinBox->setMinimum(-9999);
                    spinBox->setMaximum(9999);
                    spinBox->setSingleStep(1);
                    spinBox->setValue(value.toDouble());
                    extensionsLayout->addRow(label, spinBox);
                    settings.push_back(Setting { spinBox, settingKeyName, false, value.toDouble() });
                } else {
                    qWarning() << "Unsupported setting " << name;
                }
            }
        }

        pluginGroup->setLayout(extensionsLayout);
        mainLayout->addWidget(pluginGroup);
    }

    mainLayout->addStretch(1);

    setWidget(central);
    setWidgetResizable(true);
}

QJsonObject SettingsTab::getOptions() const
{
    QJsonObject options;

    for (const Setting &setting : settings) {
        auto checkBox = dynamic_cast<QCheckBox *>(setting.uiELement);
        auto spinBox = dynamic_cast<QSpinBox *>(setting.uiELement);

        if (checkBox != nullptr && setting.defaultBool != checkBox->isChecked()) {
            options[setting.name] = QJsonValue(checkBox->isChecked());
        } else if (spinBox != nullptr && setting.defaultReal != spinBox->value()) {
            options[setting.name] = QJsonValue(double(spinBox->value()));
        }
    }

    return options;
}
