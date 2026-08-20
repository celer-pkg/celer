// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest 1.0
import QtPositioning 6.2

TestCase {
    id: testCase

    name: "Address"

    Address {
        id: address

        street: "Evergreen Tce"
        streetNumber: "742"
        district: "Pressboard Estates"
        city: "Springfield"
        state: "Oregon"
        postalCode: "8900"
        country: "United States"
        countryCode: "USA"
    }

    function test_qmlAddressText() {
        compare(address.isTextGenerated, true);
        compare(address.text, "742 Evergreen Tce<br/>Springfield, Oregon 8900<br/>United States");
        var textChangedSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        textChangedSpy.target = address;
        textChangedSpy.signalName = "textChanged"

        var isTextGeneratedSpy = Qt.createQmlObject('import QtTest 1.0; SignalSpy {}', testCase, "SignalSpy");
        isTextGeneratedSpy.target = address
        isTextGeneratedSpy.signalName = "isTextGeneratedChanged"

        address.countryCode = "FRA";
        compare(address.text, "742 Evergreen Tce<br/>8900 Springfield<br/>United States");
        compare(textChangedSpy.count, 1);
        textChangedSpy.clear();
        compare(isTextGeneratedSpy.count, 0);

        address.countryCode = "DEU"; // the street number should go after the street name
        compare(address.text, "Evergreen Tce 742<br/>8900 Springfield<br/>United States");
        compare(textChangedSpy.count, 1);
        textChangedSpy.clear();
        compare(isTextGeneratedSpy.count, 0);

        address.text = "address label";
        compare(address.isTextGenerated, false);
        compare(address.text, "address label");
        compare(textChangedSpy.count, 1);
        textChangedSpy.clear();
        compare(isTextGeneratedSpy.count, 1);
        isTextGeneratedSpy.clear();

        address.countryCode = "USA";
        compare(address.text, "address label");
        compare(textChangedSpy.count, 0);
        textChangedSpy.clear();
        compare(isTextGeneratedSpy.count, 0);

        address.text = "";
        compare(address.isTextGenerated, true);
        compare(address.text, "742 Evergreen Tce<br/>Springfield, Oregon 8900<br/>United States");
        compare(textChangedSpy.count, 1);
        textChangedSpy.clear();
        compare(isTextGeneratedSpy.count, 1);
        isTextGeneratedSpy.clear();
    }
}
