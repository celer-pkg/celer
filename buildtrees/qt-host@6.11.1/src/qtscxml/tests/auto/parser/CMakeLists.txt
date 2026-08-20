# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_scxml_parser Test:
#####################################################################

qt_internal_add_test(tst_scxml_parser
    SOURCES
        tst_parser.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Scxml
)

# Resources:
set(tst_parser_resource_files
    "data/badInitial.scxml"
    "data/badInitial.scxml.errors"
    "data/commentInScript.scxml"
    "data/commentInScript.scxml.errors"
    "data/empty.scxml"
    "data/empty.scxml.errors"
    "data/eventnames.scxml"
    "data/eventnames.scxml.errors"
    "data/ids1.scxml"
    "data/ids1.scxml.errors"
    "data/ids2.scxml"
    "data/ids2.scxml.errors"
    "data/invalidContent.scxml"
    "data/invalidContent.scxml.errors"
    "data/invalidRoot1.scxml"
    "data/invalidRoot1.scxml.errors"
    "data/invalidRoot2.scxml"
    "data/invalidRoot2.scxml.errors"
    "data/invalidRoot3.scxml"
    "data/invalidRoot3.scxml.errors"
    "data/invalidRoot6.scxml"
    "data/invalidRoot6.scxml.errors"
    "data/invalidUnicode1.scxml"
    "data/invalidUnicode1.scxml.errors"
    "data/invalidXmlHeader1.scxml"
    "data/invalidXmlHeader1.scxml.errors"
    "data/invalidXmlHeader2.scxml"
    "data/invalidXmlHeader2.scxml.errors"
    "data/invalidstatemachinename.scxml"
    "data/invalidstatemachinename.scxml.errors"
    "data/misplacedinvoke.scxml"
    "data/misplacedinvoke.scxml.errors"
    "data/namespaces1.scxml"
    "data/namespaces1.scxml.errors"
    "data/nestedScxml.scxml"
    "data/nestedScxml.scxml.errors"
    "data/noContentInInvoke1.scxml"
    "data/noContentInInvoke1.scxml.errors"
    "data/noContentInInvoke2.scxml"
    "data/noContentInInvoke2.scxml.errors"
    "data/noContentInInvoke3.scxml"
    "data/noContentInInvoke3.scxml.errors"
    "data/noContentInInvoke4.scxml"
    "data/noContentInInvoke4.scxml.errors"
    "data/prematureEndOfDocument1.scxml"
    "data/prematureEndOfDocument1.scxml.errors"
    "data/prematureEndOfDocument2.scxml"
    "data/prematureEndOfDocument2.scxml.errors"
    "data/scxml1.scxml"
    "data/scxml1.scxml.errors"
    "data/scxml2.scxml"
    "data/scxml2.scxml.errors"
    "data/syntaxErrors1.scxml"
    "data/syntaxErrors1.scxml.errors"
    "data/syntaxErrors10.scxml"
    "data/syntaxErrors10.scxml.errors"
    "data/syntaxErrors11.scxml"
    "data/syntaxErrors11.scxml.errors"
    "data/syntaxErrors12.scxml"
    "data/syntaxErrors12.scxml.errors"
    "data/syntaxErrors13.scxml"
    "data/syntaxErrors13.scxml.errors"
    "data/syntaxErrors14.scxml"
    "data/syntaxErrors14.scxml.errors"
    "data/syntaxErrors15.scxml"
    "data/syntaxErrors15.scxml.errors"
    "data/syntaxErrors2.scxml"
    "data/syntaxErrors2.scxml.errors"
    "data/syntaxErrors3.scxml"
    "data/syntaxErrors3.scxml.errors"
    "data/syntaxErrors4.scxml"
    "data/syntaxErrors4.scxml.errors"
    "data/syntaxErrors5.scxml"
    "data/syntaxErrors5.scxml.errors"
    "data/syntaxErrors7.scxml"
    "data/syntaxErrors7.scxml.errors"
    "data/syntaxErrors9.scxml"
    "data/syntaxErrors9.scxml.errors"
    "data/test1.scxml"
    "data/test1.scxml.errors"
    "data/emptyScript.scxml"
    "data/emptyScript.scxml.errors"
)

qt_internal_add_resource(tst_scxml_parser "tst_parser"
    PREFIX
        "/tst_parser"
    FILES
        ${tst_parser_resource_files}
)


#### Keys ignored in scope 1:.:.:parser.pro:<TRUE>:
# TEMPLATE = "app"
