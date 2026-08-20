# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_dynamicmetaobject Test:
#####################################################################

qt_internal_add_test(tst_dynamicmetaobject
    SOURCES
        tst_dynamicmetaobject.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Scxml
)

# Resources:
set(tst_dynamicmetaobject_resource_files
    "mediaplayer.scxml"
    "test1.scxml"
)

qt_internal_add_resource(tst_dynamicmetaobject "tst_dynamicmetaobject"
    PREFIX
        "/tst_dynamicmetaobject"
    FILES
        ${tst_dynamicmetaobject_resource_files}
)


#### Keys ignored in scope 1:.:.:dynamicmetaobject.pro:<TRUE>:
# TEMPLATE = "app"
