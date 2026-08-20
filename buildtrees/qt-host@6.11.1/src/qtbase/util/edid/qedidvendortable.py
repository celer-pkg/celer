#!/usr/bin/env python3
# Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import urllib.request
import sys

# The original source for this data used to be
# 'https://git.fedorahosted.org/cgit/hwdata.git/plain/pnp.ids'
# which is discontinued. For now there seems to be a fork at:
url = 'https://github.com/vcrhonek/hwdata/raw/master/pnp.ids'
# REUSE-IgnoreStart
copyright = """// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
"""
# REUSE-IgnoreEnd
notice = """/*
 * This lookup table was generated from {}
 *
 * Do not change this file directly, instead edit the
 * qtbase/util/edid/qedidvendortable.py script and regenerate this file.
 */""".format(url)

header = """
#ifndef QEDIDVENDORTABLE_P_H
#define QEDIDVENDORTABLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qglobal_p.h>
#include <QtCore/qtypes.h>

#include <iterator>

QT_BEGIN_NAMESPACE
"""

vendorIdHeader = """struct QEdidVendorId {
    const char id[4];
};

static constexpr QEdidVendorId q_edidVendorIds[] = {"""

vendorIdFooter = """};
"""

vendorNameHeader = """static constexpr char q_edidVendorNames[] = {"""

vendorNameFooter = """};
"""

vendorNameOffsetHeader = """static constexpr %s q_edidVendorNamesOffsets[] = {"""

vendorNameOffsetFooter = """};
"""

footer = """static_assert(std::size(q_edidVendorIds) == std::size(q_edidVendorNamesOffsets));

QT_END_NAMESPACE

#endif // QEDIDVENDORTABLE_P_H"""


# Actual script begins here

vendors = {}

vendorNameTotalLength = 0
response = urllib.request.urlopen(url)
data = response.read().decode('utf-8')
for line in data.split('\n'):
    if line.startswith('#'):
        continue
    elif len(line) == 0:
        continue

    l = line.split('\t', 1)
    if len(l) == 0:
        continue

    pnp_id = l[0].upper()
    if len(pnp_id) != 3:
        sys.exit("Id '%s' is non-conforming" % pnp_id)
    vendors[pnp_id] = l[1]
    vendorNameTotalLength += len(l[1]) + 1

sortedVendorKeys = sorted(vendors.keys())

print(copyright)
print(notice)
print(header)

print(vendorIdHeader)
print(*[('    { "%s" }' % pnp_id) for pnp_id in sortedVendorKeys], sep=",\n")
print(vendorIdFooter)

print(vendorNameHeader)
print(*[('    "%s\\0"' % vendors[pnp_id]) for pnp_id in sortedVendorKeys], sep="\n")
print(vendorNameFooter)

if vendorNameTotalLength < 2**16:
    print(vendorNameOffsetHeader % "quint16")
elif vendorNameTotalLength < 2**32:
    print(vendorNameOffsetHeader % "quint32")
else:
    sys.exit("Vendor name table is too big")

currentOffset = 0
for pnp_id in sortedVendorKeys:
    vendor = vendors[pnp_id]
    print('    %d,' % currentOffset)
    currentOffset += len(vendor) + 1

print(vendorNameOffsetFooter)

print(footer)
