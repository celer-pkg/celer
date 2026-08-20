// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include "ieframe.h" // generated header
#include "msxml6.h"  // generated header
#include <QApplication>


struct XmlFixture
{
    MSXML2::DOMDocument60 doc;
    MSXML2::IXMLDOMNodeList *root;

    const QString xml{
        R"(
            <root prop="The root property">
                The value
            </root>
        )" };
};

class tst_dumpcpp : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void toggleAddressBar();
    void propertyGetter_ReturnsValue_WhenValueIsInt();
    void propertyGetter_ReturnsValue_WhenValueIsString();
    void invokeGetter_ReturnsValue_WhenValueInheritsIDispatch();
    void propertyGetter_ReturnsValue_WhenValueInheritsIDispatch();

    void propertySetter_SetsValue_WhenValueIsVariantInt();
    void propertySetter_SetsValue_WhenValueIsString();
    void invoke_SetsValue_WhenValueDerivesFromIDispatch();

private:
    XmlFixture m_xml;
};

void tst_dumpcpp::init()
{
    m_xml.doc.loadXML(m_xml.xml);
    m_xml.root = m_xml.doc.childNodes();
}

// A simple test to verify that an object can be instantiated and interacted with
void tst_dumpcpp::toggleAddressBar()
{
    SHDocVw::WebBrowser *webBrowser = new SHDocVw::WebBrowser;
    QVERIFY(webBrowser);
    bool addressBar = webBrowser->AddressBar();
    addressBar = !addressBar;
    webBrowser->SetAddressBar(addressBar);
    QVERIFY(webBrowser->AddressBar() == addressBar);
    delete webBrowser;
}

void tst_dumpcpp::propertyGetter_ReturnsValue_WhenValueIsInt()
{
    int length = m_xml.root->length();
    QVERIFY(length == 1);
}

void tst_dumpcpp::invokeGetter_ReturnsValue_WhenValueInheritsIDispatch()
{
    // item(...) takes an argument and is called as a function invocation
    MSXML2::IXMLDOMNode *firstChild = m_xml.root->item(0);
    QVERIFY(firstChild);
}

void tst_dumpcpp::propertyGetter_ReturnsValue_WhenValueInheritsIDispatch()
{
    // attributes() takes an argument and is called as property getter
    MSXML2::IXMLDOMNamedNodeMap *attributes = m_xml.root->item(0)->attributes();
    QVERIFY(attributes);
}

void tst_dumpcpp::propertyGetter_ReturnsValue_WhenValueIsString()
{
    MSXML2::IXMLDOMNamedNodeMap *attributes = m_xml.root->item(0)->attributes();

    // nodeValue is a property getter
    QVariant p = attributes->getNamedItem("prop")->nodeValue();
    QCOMPARE(p, "The root property");
}

void tst_dumpcpp::propertySetter_SetsValue_WhenValueIsVariantInt()
{
    MSXML2::IXMLDOMNamedNodeMap *attributes = m_xml.root->item(0)->attributes();
    MSXML2::IXMLDOMNode *attribNode = attributes->item(0);
    attribNode->setNodeValue(QVariant { 42 } );

    QVariant p = attributes->getNamedItem("prop")->nodeValue();
    QCOMPARE(p, 42);
}

void tst_dumpcpp::propertySetter_SetsValue_WhenValueIsString()
{
    m_xml.root->item(0)->setText("The new value");
    QCOMPARE(m_xml.root->item(0)->text(), "The new value");
}

void tst_dumpcpp::invoke_SetsValue_WhenValueDerivesFromIDispatch()
{
    MSXML2::IXMLDOMNode *node = m_xml.doc.createNode(MSXML2::NODE_ELEMENT, "sometag", "");
    node->setText("The new text");

    m_xml.root->item(0)->appendChild(node);

    QCOMPARE(m_xml.root->item(0)->childNodes()->item(1)->text(), "The new text");
}

QTEST_MAIN(tst_dumpcpp)
#include "tst_dumpcpp.moc"
