// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/* Raw Geometry Data:
 *
 * source:
 * https://gist.github.com/hrbrmstr/91ea5cc9474286c72838
 *
 * license: MIT Licensed
 */

import QtQuick
import QtLocation
import QtPositioning

MapPolygon {
    color: "green"
    autoFadeIn: false
    geoShape: QtPositioning.polygon(
        [
            QtPositioning.coordinate(-82.081680297851562, 163.76611328125),
            QtPositioning.coordinate(-84.30224609375, 180.0),
            QtPositioning.coordinate(-90.0, 180.0),
            QtPositioning.coordinate(-90.0, -180.0),
            QtPositioning.coordinate(-84.305343627929688, -180.0),
            QtPositioning.coordinate(-84.984725952148438, -138.589447021484375),
            QtPositioning.coordinate(-84.012786865234375, -153.053070068359375),
            QtPositioning.coordinate(-82.577789306640625, -151.794189453125),
            QtPositioning.coordinate(-81.252296447753906, -156.9560546875),
            QtPositioning.coordinate(-80.460556030273438, -145.524169921875),
            QtPositioning.coordinate(-78.7197265625, -155.906402587890625),
            QtPositioning.coordinate(-78.311668395996094, -153.750442504882812),
            QtPositioning.coordinate(-77.108062744140625, -157.753082275390625),
            QtPositioning.coordinate(-77.761123657226562, -149.6622314453125),
            QtPositioning.coordinate(-77.466400146484375, -146.26861572265625),
            QtPositioning.coordinate(-76.759033203125, -145.461334228515625),
            QtPositioning.coordinate(-76.451950073242188, -146.9322509765625),
            QtPositioning.coordinate(-76.382156372070312, -149.503341674804688),
            QtPositioning.coordinate(-76.443344116210938, -145.479461669921875),
            QtPositioning.coordinate(-74.532791137695312, -134.30389404296875),
            QtPositioning.coordinate(-75.096260070800781, -99.513893127441406),
            QtPositioning.coordinate(-73.637924194335938, -103.013343811035156),
            QtPositioning.coordinate(-73.610565185546875, -99.195556640625),
            QtPositioning.coordinate(-73.326400756835938, -103.036117553710938),
            QtPositioning.coordinate(-72.73333740234375, -103.176116943359375),
            QtPositioning.coordinate(-73.943344116210938, -82.134170532226562),
            QtPositioning.coordinate(-73.050567626953125, -80.695556640625),
            QtPositioning.coordinate(-73.873062133789062, -76.962783813476562),
            QtPositioning.coordinate(-73.197235107421875, -69.4283447265625),
            QtPositioning.coordinate(-72.40618896484375, -66.800277709960938),
            QtPositioning.coordinate(-69.413543701171875, -68.832778930664062),
            QtPositioning.coordinate(-69.021263122558594, -66.659873962402344),
            QtPositioning.coordinate(-67.527091979980469, -66.433761596679688),
            QtPositioning.coordinate(-67.171043395996094, -67.602455139160156),
            QtPositioning.coordinate(-63.205421447753906, -57.197780609130859),
            QtPositioning.coordinate(-67.562850952148438, -65.611808776855469),
            QtPositioning.coordinate(-68.707923889160156, -65.319450378417969),
            QtPositioning.coordinate(-68.835769653320312, -63.861354827880859),
            QtPositioning.coordinate(-68.51611328125, -64.39154052734375),
            QtPositioning.coordinate(-68.413475036621094, -62.738128662109375),
            QtPositioning.coordinate(-68.531394958496094, -63.966804504394531),
            QtPositioning.coordinate(-71.742500305175781, -60.905696868896484),
            QtPositioning.coordinate(-72.04937744140625, -62.546043395996094),
            QtPositioning.coordinate(-72.897369384765625, -59.777362823486328),
            QtPositioning.coordinate(-73.133346557617188, -61.903060913085938),
            QtPositioning.coordinate(-73.709381103515625, -60.591533660888672),
            QtPositioning.coordinate(-74.832229614257812, -61.887504577636719),
            QtPositioning.coordinate(-76.707778930664062, -70.476394653320312),
            QtPositioning.coordinate(-75.918899536132812, -77.77166748046875),
            QtPositioning.coordinate(-77.656257629394531, -72.84820556640625),
            QtPositioning.coordinate(-78.360145568847656, -84.101882934570312),
            QtPositioning.coordinate(-83.033340454101562, -58.210838317871094),
            QtPositioning.coordinate(-80.276947021484375, -28.319446563720703),
            QtPositioning.coordinate(-79.664237976074219, -30.203750610351562),
            QtPositioning.coordinate(-79.017715454101562, -27.090139389038086),
            QtPositioning.coordinate(-78.771949768066406, -36.299724578857422),
            QtPositioning.coordinate(-75.733901977539062, -17.778614044189453),
            QtPositioning.coordinate(-73.94451904296875, -13.711459159851074),
            QtPositioning.coordinate(-73.770004272460938, -16.893474578857422),
            QtPositioning.coordinate(-70.90020751953125, -9.869357109069824),
            QtPositioning.coordinate(-71.825836181640625, -8.366945266723633),
            QtPositioning.coordinate(-70.074462890625, 8.682855606079102),
            QtPositioning.coordinate(-70.546417236328125, 23.246753692626953),
            QtPositioning.coordinate(-68.4827880859375, 34.140357971191406),
            QtPositioning.coordinate(-69.988632202148438, 38.650367736816406),
            QtPositioning.coordinate(-68.5220947265625, 41.111885070800781),
            QtPositioning.coordinate(-65.84002685546875, 53.778961181640625),
            QtPositioning.coordinate(-67.75390625, 69.644790649414062),
            QtPositioning.coordinate(-69.359054565429688, 69.746170043945312),
            QtPositioning.coordinate(-70.390571594238281, 67.646987915039062),
            QtPositioning.coordinate(-70.662788391113281, 69.2471923828125),
            QtPositioning.coordinate(-72.066192626953125, 67.340263366699219),
            QtPositioning.coordinate(-72.425003051757812, 68.936416625976562),
            QtPositioning.coordinate(-71.942230224609375, 70.839797973632812),
            QtPositioning.coordinate(-70.01251220703125, 72.930068969726562),
            QtPositioning.coordinate(-66.894744873046875, 87.502517700195312),
            QtPositioning.coordinate(-66.036262512207031, 88.23529052734375),
            QtPositioning.coordinate(-66.831130981445312, 108.82421875),
            QtPositioning.coordinate(-65.713348388671875, 113.315338134765625),
            QtPositioning.coordinate(-67.142242431640625, 128.844345092773438),
            QtPositioning.coordinate(-66.198066711425781, 134.25225830078125),
            QtPositioning.coordinate(-64.925308227539062, 134.407257080078125),
            QtPositioning.coordinate(-66.093063354492188, 134.947784423828125),
            QtPositioning.coordinate(-71.351669311523438, 170.453033447265625),
            QtPositioning.coordinate(-73.52752685546875, 169.07916259765625),
            QtPositioning.coordinate(-75.098007202148438, 162.547500610351562),
            QtPositioning.coordinate(-76.948768615722656, 162.34210205078125),
            QtPositioning.coordinate(-78.65716552734375, 167.263671875),
            QtPositioning.coordinate(-80.288619995117188, 158.066757202148438),
            QtPositioning.coordinate(-82.081680297851562, 163.76611328125)
        ]
    )
}
