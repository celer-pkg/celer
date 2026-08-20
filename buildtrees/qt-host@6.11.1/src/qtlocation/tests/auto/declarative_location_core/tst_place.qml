// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation
import QtPositioning
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "Place"

    Plugin {
        id: testPlugin
        name: "qmlgeo.test.plugin"
        allowExperimental: true
    }

    Place {
        id: favoritePlace
        name: "Favorite Place"
    }

    Place { id: emptyPlace }

    Place { id: emptyPlace2 }

    Place { id: testPlace }

    Place {
        id: savePlace

        name: "Test place"

        visibility: Place.DeviceVisibility

        location: Location {
            address: Address {
                country: "country"
                countryCode: "cc"
                state: "state"
                county: "county"
                city: "city"
                district: "district"
                street: "123 Fake Street"
                postalCode: "1234"
            }

            coordinate {
                latitude: 10
                longitude: 10
                altitude: 100
            }

            boundingShape: QtPositioning.rectangle(QtPositioning.coordinate(10, 10, 100), 100, 100)
        }

        ratings: ({ average: 3.5, count: 10 })

        supplier: ({
            name: "Supplier 1",
            supplierId: "supplier-id-1",
            url: "http://www.example.com/supplier-id-1/",
            icon: ({ parameters : { singleUrl: "http://www.example.com/supplier-id-1/icon" }})
        })
        categories: [
            Category {
                name: "Category 1"
                categoryId: "category-id-1"
                plugin: testPlugin
            },
            Category {
                name: "Category 2"
                categoryId: "category-id-2"
                plugin: testPlugin
            }
        ]

        icon: ({ parameters: { singleUrl: "http://example.com/test-place.png" } })
    }

     Place {
         id: dummyPlace
         placeId: "487"
         name: "dummyPlace"
         visibility: Place.PublicVisibility
     }

    // compares two places property by property
    function compare_place(place1, place2) {
        // check simple properties
        var simpleProperties = ["name", "placeId", "primaryPhone", "primaryFax", "primaryEmail",
                                "primaryWebsite", "visibility"];
        for (x in simpleProperties) {
            if (place1[simpleProperties[x]] !== place2[simpleProperties[x]])
                return false;
        }

        // check categories
        if (place1.categories.length !== place2.categories.length)
            return false;
        for (var i = 0; i < place1.categories.length; ++i) {
            // fixme, what if the order of the two lists are not the same
            if (place1.categories[i].categoryId !== place2.categories[i].categoryId)
                return false;
            if (place1.categories[i].name !== place2.categories[i].name)
                return false;
        }

        // check supplier
        if (place1.supplier !== place2.supplier) {
            return false;
        }

        // check ratings
        if (place1.ratings !==  place2.ratings) {
            return false;
        }

        // check location
        if (place1.location === null && place2.location !== null)
            return false;
        if (place1.location !== null && place2.location === null)
            return false;
        if (place1.location !== null && place2.location !== null) {
            if (place1.location.address.country !== place2.location.address.country)
                return false;
            if (place1.location.address.countryCode !== place2.location.address.countryCode)
                return false;
            if (place1.location.address.state !== place2.location.address.state)
                return false;
            if (place1.location.address.county !== place2.location.address.county)
                return false;
            if (place1.location.address.city !== place2.location.address.city)
                return false;
            if (place1.location.address.district !== place2.location.address.district)
                return false;
            if (place1.location.address.street !== place2.location.address.street)
                return false;
            if (place1.location.address.postalCode !== place2.location.address.postalCode)
                return false;

            if (place1.location.coordinate !== place2.location.coordinate)
                return false;
            if (place1.location.boundingBox !== place2.location.boundingBox)
                return false;
        }

        // check icon
        if (place1.icon !== place2.icon) {
            return false;
        }

        // check extended attributes

        return true;
    }

    function test_emptyPlace() {
        // basic properties
        compare(emptyPlace.plugin, null);
        compare(emptyPlace.categories.length, 0);
        compare(emptyPlace.name, "");
        compare(emptyPlace.placeId, "");
        compare(emptyPlace.detailsFetched, false);
        compare(emptyPlace.status, Place.Ready);
        compare(emptyPlace.primaryPhone, "");
        compare(emptyPlace.primaryFax, "");
        compare(emptyPlace.primaryEmail, "");
        compare(emptyPlace.primaryWebsite, "");
        compare(emptyPlace.visibility, Place.UnspecifiedVisibility);
        compare(emptyPlace.attribution, "");

        // complex properties
        compare(emptyPlace.ratings.average, 0);
        compare(emptyPlace.location.address.street, '');
        compare(emptyPlace.location.address.district, '');
        compare(emptyPlace.location.address.city, '');
        compare(emptyPlace.location.address.county, '');
        compare(emptyPlace.location.address.state, '');
        compare(emptyPlace.location.address.country, '');

        compare(emptyPlace.supplier.name, '');
        compare(emptyPlace.supplier.supplierId, '');
        compare(emptyPlace.supplier.url, '');

        compare(emptyPlace.reviewModel.totalCount, -1);
        compare(emptyPlace.imageModel.totalCount, -1);
        compare(emptyPlace.editorialModel.totalCount, -1);
        compare(emptyPlace.categories.length, 0);

        verify(compare_place(emptyPlace, emptyPlace));
        verify(compare_place(emptyPlace, emptyPlace2));
    }

    function test_setAndGet_data() {
        return [
            { tag: "name", property: "name", signal: "nameChanged", value: "Test Place", reset: "" },
            { tag: "placeId", property: "placeId", signal: "placeIdChanged", value: "test-place-id-1", reset: "" },
            { tag: "visibility", property: "visibility", signal: "visibilityChanged", value: Place.PublicVisibility, reset: Place.UnspecifiedVisibility },
            { tag: "attribution", property: "attribution", signal: "attributionChanged", value: "Place data from...", reset: "" },
            { tag: "favorite", property: "favorite", signal: "favoriteChanged", value: favoritePlace }
        ];
    }

    function test_setAndGet(data) {
        Utils.testObjectProperties(testCase, testPlace, data);
    }

    function test_categories() {
        var categories = new Array(2);
        categories[0] = Qt.createQmlObject('import QtLocation 5.3; Category { categoryId: "cat-id-1"; name: "Category 1" }', testCase, "Category1");
        categories[1] = Qt.createQmlObject('import QtLocation 5.3; Category { categoryId: "cat-id-2"; name: "Category 2" }', testCase, "Category2");

        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = testPlace;
        signalSpy.signalName = "categoriesChanged";

        // set categories to something new
        testPlace.categories = categories;
        compare(testPlace.categories.length, categories.length);

        for (var i = 0; i < categories.length; ++i) {
            compare(testPlace.categories[i].categoryId, categories[i].categoryId);
            compare(testPlace.categories[i].name, categories[i].name);
        }

        compare(signalSpy.count, 2);

        // set categories to the same. Signal spy should not increase since nothing changes.
        testPlace.categories = categories;
        compare(signalSpy.count, 2);

        compare(testPlace.categories.length, categories.length);

        for (var i = 0; i < categories.length; ++i) {
            compare(testPlace.categories[i].categoryId, categories[i].categoryId);
            compare(testPlace.categories[i].name, categories[i].name);
        }

        // All the comparing should not change anything about the list.
        compare(signalSpy.count, 2);

        // reset by assignment
        testPlace.categories = new Array(0);
        compare(testPlace.categories.length, 0);
        compare(signalSpy.count, 3);

        signalSpy.destroy();
    }

    function test_supplier() {
        var supplier = savePlace.supplier;

        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = testPlace;
        signalSpy.signalName = "supplierChanged";

        // set supplier to something new
        testPlace.supplier = supplier;
        compare(testPlace.supplier, supplier);

        compare(testPlace.supplier.supplierId, supplier.supplierId);
        compare(testPlace.supplier.name, supplier.name);

        compare(signalSpy.count, 1);

        // set supplier to the same
        testPlace.supplier = supplier;
        compare(testPlace.supplier, supplier);

        compare(testPlace.supplier.supplierId, supplier.supplierId);
        compare(testPlace.supplier.name, supplier.name);

        compare(signalSpy.count, 1);

        // reset by assignment
        testPlace.supplier = emptyPlace.supplier;
        compare(testPlace.supplier, emptyPlace.supplier);
        compare(signalSpy.count, 2);

        signalSpy.destroy();
    }

    function test_location() {
        var location = Qt.createQmlObject('import QtPositioning 5.2; Location { coordinate: QtPositioning.coordinate(10.0, 20.0) }', testCase, "Location1");

        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = testPlace;
        signalSpy.signalName = "locationChanged";

        testPlace.location = location;
        compare(testPlace.location.coordinate.latitude, 10.0);
        compare(signalSpy.count, 1);

        testPlace.location = location;
        compare(testPlace.location.coordinate.latitude, 10.0);
        compare(signalSpy.count, 1);

        testPlace.location = null;
        compare(testPlace.location, null);
        compare(signalSpy.count, 2);

        location.destroy();
        signalSpy.destroy();
    }

    function test_ratings() {
        var ratings1 = emptyPlace.ratings
        var ratings2 = savePlace.ratings

        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = testPlace;
        signalSpy.signalName = "ratingsChanged";

        testPlace.ratings = ratings1;
        compare(signalSpy.count, 0);

        testPlace.ratings = ratings2;
        compare(testPlace.ratings.count, ratings2.count);
        compare(signalSpy.count, 1);

        testPlace.ratings = ratings1;
        compare(testPlace.ratings.count, ratings1.count);
        compare(signalSpy.count, 2);

        signalSpy.destroy();
    }

    property placeAttribute fooAttribute
    fooAttribute {
        label: "Foo label"
        text: "Foo"
    }
    function test_extendedAttributes() {
        verify(testPlace.extendedAttributes);

        testPlace.extendedAttributes["foo"] = fooAttribute;

        verify(testPlace.extendedAttributes.foo);
        compare(testPlace.extendedAttributes.foo.text, "Foo");
        compare(testPlace.extendedAttributes.foo.label, "Foo label");

        testPlace.extendedAttributes["foo"] = null;
        verify(!testPlace.extendedAttributes.foo);
    }

    property contactDetail testDetail
    testDetail {
        label: "Test Label"
        value: "Detail Value"
    }
    function test_contactDetailsProperty() {
        verify(testPlace.contactDetails);

        testPlace.contactDetails["phone"] = testDetail;

        verify(testPlace.contactDetails.phone);
        compare(testPlace.contactDetails.phone[0].label, "Test Label");
        compare(testPlace.contactDetails.phone[0].value, "Detail Value");

        testPlace.contactDetails["phone"] = null;
        verify(!testPlace.contactDetails.phone);
    }

    function test_saveload() {
        // Save a place
        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = savePlace;
        signalSpy.signalName = "statusChanged";

        savePlace.plugin = testPlugin;
        savePlace.icon.plugin = testPlugin;
        savePlace.placeId = "invalid-place-id";

        savePlace.save();

        compare(savePlace.status, Place.Saving);

        tryCompare(savePlace, "status", Place.Error);

        // try again without an invalid placeId
        savePlace.placeId = "";
        savePlace.save();

        compare(savePlace.status, Place.Saving);

        tryCompare(savePlace, "status", Place.Ready);

        verify(savePlace.placeId !== "");

        signalSpy.destroy();


        // Read a place
        var readPlace = Qt.createQmlObject('import QtLocation 5.3; Place { }', testCase, "test_saveload");

        signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = readPlace;
        signalSpy.signalName = "statusChanged";

        readPlace.plugin = testPlugin;

        readPlace.getDetails();

        compare(readPlace.status, Place.Fetching);
        tryCompare(readPlace, "status", Place.Error);

        readPlace.placeId = "invalid-id";

        readPlace.getDetails();

        compare(readPlace.status, Place.Fetching);
        tryCompare(readPlace, "status", Place.Error);

        readPlace.placeId = savePlace.placeId;

        // verify that read place is not currently the same as what we saved
        verify(!compare_place(readPlace, savePlace));

        readPlace.getDetails();

        compare(readPlace.status, Place.Fetching);
        tryCompare(readPlace, "status", Place.Ready);

        // verify that read place is the same as what we saved
        verify(compare_place(readPlace, savePlace));

        signalSpy.destroy();


        // Remove a place
        var removePlace = Qt.createQmlObject('import QtLocation 5.3; Place { }', testCase, "test_saveload");

        signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = removePlace;
        signalSpy.signalName = "statusChanged";

        removePlace.plugin = testPlugin;

        removePlace.remove();

        compare(removePlace.status, Place.Removing);
        tryCompare(removePlace, "status", Place.Error);

        removePlace.placeId = "invalid-id";

        removePlace.remove();

        compare(removePlace.status, Place.Removing);
        tryCompare(removePlace, "status", Place.Error);

        removePlace.placeId = savePlace.placeId;

        removePlace.remove();

        compare(removePlace.status, Place.Removing);
        tryCompare(removePlace, "status", Place.Ready);

        removePlace.getDetails();

        compare(removePlace.status, Place.Fetching);
        tryCompare(removePlace, "status", Place.Error);

        signalSpy.destroy();
    }

    function test_copy() {
        var place = Qt.createQmlObject('import QtLocation 5.3; Place { }', this);
        place.plugin = testPlugin;
        place.copyFrom(dummyPlace);
        compare(place.placeId, "");
        compare(place.name, "dummyPlace");
        compare(place.visibility, Place.UnspecifiedVisibility);
    }

    property contactDetail detail1
    detail1 {
        label: "Detail1"
        value: "555-detail1"
    }
    property contactDetail detail2
    detail2 {
        label: "Detail2"
        value: "555-detail2"
    }

    function test_contactDetails(data) {
        var place = Qt.createQmlObject('import QtLocation 5.3; Place {}', this);

        var signalSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        signalSpy.target = place;
        signalSpy.signalName = data.signalName;

        place.contactDetails[data.contactType] = detail1;
        compare(place.contactDetails[data.contactType].length, 1);
        compare(place.contactDetails[data.contactType][0].label, detail1.label);
        compare(place.contactDetails[data.contactType][0].value, detail1.value);

        compare(place[data.primaryValue], "555-detail1");
        compare(signalSpy.count, 1);
        signalSpy.clear();

        var listView = Qt.createQmlObject('import QtQuick 2.0; ListView { delegate:Text{text:modelData.label + ":" + modelData.value } }', this);
        listView.model = place.contactDetails[data.contactType];
        compare(listView.count, 1);

        var details = new Array();
        details.push(detail2);
        details.push(detail1);

        place.contactDetails[data.contactType] = details;
        compare(place.contactDetails[data.contactType].length, 2);
        compare(place.contactDetails[data.contactType][0].label, detail2.label);
        compare(place.contactDetails[data.contactType][0].value, detail2.value);
        compare(place.contactDetails[data.contactType][1].label, detail1.label);
        compare(place.contactDetails[data.contactType][1].value, detail1.value);

        compare(place[data.primaryValue], detail2.value);
        compare(signalSpy.count, 1);
        signalSpy.clear();
        listView.model = place.contactDetails[data.contactType];
        compare(listView.count, 2);
    }

    function test_contactDetails_data() {
        return [
                    { tag: "phone", contactType: "phone", signalName: "primaryPhoneChanged", primaryValue: "primaryPhone"},
                    { tag: "fax", contactType: "fax", signalName: "primaryFaxChanged", primaryValue: "primaryFax"},
                    { tag: "email", contactType: "email", signalName: "primaryEmailChanged", primaryValue: "primaryEmail"},
                    { tag: "website", contactType: "website", signalName: "primaryWebsiteChanged", primaryValue: "primaryWebsite"}
        ];
    }
}
