// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtLocation

MenuBar {
    property var providerMenu: providerMenu
    property var settingsMenu: settingsMenu

    signal selectProvider(providerName: string)
    signal selectSetting(settings: string);

    function getMenuItem(menuText, parentItem) {
        var menuItem = Qt.createQmlObject('import QtQuick; import QtQuick.Controls; MenuItem {}', parent)
        menuItem.text = qsTr(menuText)
        return menuItem
    }

    function clearMenu(menu) {
        while (menu.contentData.length > 0)
            menu.removeItem(menu.itemAt(0))
    }

    Menu {
        id: providerMenu
        title: qsTr("Provider")

        function create(plugins) {
            clearMenu(providerMenu)
            for (var i = 0; i < plugins.length; i++) {
                createProvider(plugins[i])
            }
        }

        function createProvider(itemVal) {
            var item = getMenuItem(itemVal, providerMenu)
            addItem(item)
            item.triggered.connect(function(){selectProvider(itemVal)})
        }
    }

    Menu {
        id: settingsMenu
        title: qsTr("Settings")

        function create(settings) {
            clearMenu(settingsMenu)
            for (var i = 0; i < settings.length; i++) {
                createSetting(settings[i])
            }
        }

        function createSetting(itemVal) {
            var item = getMenuItem(itemVal, providerMenu)
            addItem(item)
            item.triggered.connect(function(){selectSetting(itemVal)})
        }
    }
}
