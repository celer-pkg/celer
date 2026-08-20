// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQml.Models
import QtTest
import QtLocation
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "Category"

    Category { id: emptyCategory }

    function test_empty() {
        compare(emptyCategory.categoryId, "");
        compare(emptyCategory.name, "");
        compare(emptyCategory.visibility, Category.UnspecifiedVisibility);
        compare(emptyCategory.status, Category.Ready);
        compare(emptyCategory.plugin, null);
        verify(emptyCategory.icon);
    }

    Category {
        id: qmlCategory

        plugin: testPlugin

        categoryId: "test-category-id"
        name: "Test Category"
        visibility: Category.DeviceVisibility

        icon: ({ parameters: { singleUrl: "http://example.com/icons/test-category.png" }})
    }

    function test_qmlConstructedCategory() {
        compare(qmlCategory.categoryId, "test-category-id");
        compare(qmlCategory.name, "Test Category");
        compare(qmlCategory.visibility, Category.DeviceVisibility);
        compare(qmlCategory.status, Category.Ready);
        compare(qmlCategory.plugin, testPlugin);
        verify(qmlCategory.icon);
        compare(qmlCategory.icon.url(), "http://example.com/icons/test-category.png");
        compare(qmlCategory.icon.parameters.singleUrl, "http://example.com/icons/test-category.png");
    }

    Category {
        id: testCategory
    }

    Plugin {
        id: testPlugin
        name: "qmlgeo.test.plugin"
        allowExperimental: true
    }

    Plugin {
        id: invalidPlugin
    }

    property icon testIcon: ({ parameters: { singleUrl: "example.com" } })

    Category {
        id: saveCategory

        name: "Test Category"
        visibility: Place.DeviceVisibility
    }

    DelegateModel {
        id: categoryModel

        model: CategoryModel {
            plugin: testPlugin
        }
        delegate: Item { }
    }

    function test_setAndGet_data() {
        return [
            { tag: "name", property: "name", signal: "nameChanged", value: "Test Category", reset: "" },
            { tag: "categoryId", property: "categoryId", signal: "categoryIdChanged", value: "test-category-id-1", reset: "" },
            { tag: "visibility", property: "visibility", signal: "visibilityChanged", value: Place.PublicVisibility, reset: Place.UnspecifiedVisibility },
            { tag: "plugin", property: "plugin", signal: "pluginChanged", value: testPlugin },
        ];
    }

    function test_setAndGet(data) {
        Utils.testObjectProperties(testCase, testCategory, data);
    }

    function test_save() {
        categoryModel.model.update();
        tryCompare(categoryModel.model, "status", CategoryModel.Ready);
        compare(categoryModel.count, 0);

        saveCategory.plugin = testPlugin;
        saveCategory.categoryId = "invalid-category-id";

        saveCategory.save();

        compare(saveCategory.status, Category.Saving);
        verify(saveCategory.errorString().length === 0);

        tryCompare(saveCategory, "status", Category.Error);
        verify(saveCategory.errorString().length > 0);

        // try again without an invalid categoryId
        saveCategory.categoryId = "";
        saveCategory.save();

        compare(saveCategory.status, Category.Saving);

        tryCompare(saveCategory, "status", Category.Ready);
        verify(saveCategory.errorString().length === 0);

        verify(saveCategory.categoryId !== "");


        // Verify that the category was added to the model
        categoryModel.model.update();
        compare(categoryModel.model.status, CategoryModel.Loading);

        tryCompare(categoryModel.model, "status", CategoryModel.Ready);

        compare(categoryModel.count, 1);
        var modelCategory = categoryModel.model.data(categoryModel.modelIndex(0),
                                                     CategoryModel.CategoryRole);
        compare(modelCategory.categoryId, saveCategory.categoryId);
        compare(modelCategory.name, saveCategory.name);


        // Remove a category
        saveCategory.remove();

        compare(saveCategory.status, Category.Removing);

        tryCompare(saveCategory, "status", Category.Ready);
        verify(saveCategory.errorString().length === 0);


        // Verify that the category was removed from the model
        categoryModel.model.update();
        compare(categoryModel.model.status, CategoryModel.Loading);

        tryCompare(categoryModel.model, "status", CategoryModel.Ready);

        compare(categoryModel.count, 0);


        // Try again, this time fail because category does not exist
        saveCategory.remove();

        compare(saveCategory.status, Category.Removing);

        tryCompare(saveCategory, "status", Category.Error);

        verify(saveCategory.errorString().length > 0);
    }

    function test_saveWithoutPlugin() {
        saveCategory.plugin = null;
        saveCategory.categoryId = "";

        saveCategory.save();

        tryCompare(saveCategory, "status", Category.Error);

        verify(saveCategory.errorString().length > 0);
        compare(saveCategory.categoryId, "");

        saveCategory.plugin = invalidPlugin;

        saveCategory.save();

        compare(saveCategory.status, Category.Error);

        verify(saveCategory.errorString().length > 0);
        compare(saveCategory.categoryId, "");
    }

    function test_removeWithoutPlugin() {
        saveCategory.plugin = null;
        saveCategory.categoryId = "test-category-id";

        saveCategory.remove();

        compare(saveCategory.status, Category.Error);

        verify(saveCategory.errorString().length > 0);
        compare(saveCategory.categoryId, "test-category-id");

        saveCategory.plugin = invalidPlugin;

        saveCategory.remove();

        compare(saveCategory.status, Category.Error);

        verify(saveCategory.errorString().length > 0);
        compare(saveCategory.categoryId, "test-category-id");
    }
}
