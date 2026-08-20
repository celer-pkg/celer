// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_propertydefinition.h"

#include <QtQmlDom/private/qqmldomoutwriter_p.h>
#include <QtQmlDom/private/qqmldomlinewriterfactory_p.h>

using namespace QQmlJS::Dom;

void TestPropertyDefinition::writeOut_data()
{
    QTest::addColumn<PropertyDefinition>("property");
    QTest::addColumn<QString>("expected");

    QTest::newRow("default") << PropertyDefinition{} << "";

    const QLatin1StringView name("p");
    const QLatin1StringView typeName("var");
    const QString property_typeName_name =
            QLatin1StringView("property") + ' ' + typeName + ' ' + name;

    {
        PropertyDefinition property;
        property.name = name;
        QTest::newRow("name") << property << name.toString();
    }
    {
        PropertyDefinition property;
        property.name = name;
        property.typeName = typeName;
        QTest::newRow("named and type") << property << property_typeName_name;
    }

    // TODO add more variations

    {
        PropertyDefinition property;
        property.isOverride = true;
        QTest::newRow("override") << property << "override";
    }
    {
        PropertyDefinition property;
        property.isVirtual = true;
        QTest::newRow("virtual") << property << "virtual";
    }

    {
        PropertyDefinition property;
        property.name = name;
        property.typeName = typeName;
        property.isOverride = true;
        QTest::newRow("override name and type")
                << property << QLatin1StringView("override") + ' ' + property_typeName_name;
    }
    {
        PropertyDefinition property;
        property.name = name;
        property.typeName = typeName;
        property.isVirtual = true;
        QTest::newRow("virtual name and type")
                << property << QLatin1StringView("virtual") + ' ' + property_typeName_name;
    }
}

void TestPropertyDefinition::writeOut()
{
    // TODO add test for the first ensureNewLine in the writeOut,
    // for that LineWriter should commit smth before writing out property

    QFETCH(PropertyDefinition, property);
    QFETCH(QString, expected);

    QString resultStr;
    auto lw = createLineWriter([&resultStr](QStringView s) { resultStr.append(s); }, {}, {});
    OutWriter ow(*lw);
    ow.skipComments = true;

    // PropertyDefinition is a simple object wrap
    auto propDefItem = DomItem().wrap({}, property);
    property.writeOut(propDefItem, ow);

    lw->eof(false);
    QCOMPARE(resultStr, expected);
}

#ifndef NO_QTEST_MAIN
QTEST_MAIN(TestPropertyDefinition)
#endif
