// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [8]
DESTDIR = /path/to/Qt/plugandpaint/plugins
#! [8]


#! [21]
CONFIG -= embed_manifest_exe
#! [21]


#! [26]
CONFIG-=app_bundle
#! [26]


#! [51]
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
#! [51]

#! [53]
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
CONFIG+=x86 ppc
#! [53]

#! [56]
vendorinfo = \
    "%{\"Example Localized Vendor\"}" \
    ":\"Example Vendor\""

my_deployment.pkg_prerules = vendorinfo
DEPLOYMENT += my_deployment
#! [56]

#! [57]
supported_platforms = \
    "; This demo only supports S60 5.0" \
    "[0x1028315F],0,0,0,{\"S60ProductID\"}"

default_deployment.pkg_prerules -= pkg_platform_dependencies
my_deployment.pkg_prerules += supported_platforms
DEPLOYMENT += my_deployment
#! [57]
