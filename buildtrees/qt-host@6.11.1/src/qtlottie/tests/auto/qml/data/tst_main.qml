// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.9
import QtQuick.Window 2.2
import QtTest 1.0
import Qt.labs.lottieqt 1.0

Item {
    id: animContainer
    width: childrenRect.width
    height: childrenRect.height

    LottieAnimation {
        id: bmAnim

        x: 0
        y: 0
        quality: LottieAnimation.HighQuality
        source: "qrc:/data/rec_pos_col_opa.json"

        onFinished: {
            bmAnim.start();
        }


    }

    TestCase{
        id: testID
        name: "testAnimation"

        function test_1_loadingStatus(){
            compare(bmAnim.status, 2);      /* status: Null(0), Loading(1), Ready(2), Error(3)
                                            if the source has been loaded successfully, the status
                                            must be "Ready"
                                            */
        }

        function test_2_initialFrameRate(){
            compare(bmAnim.frameRate, 60);  /* frame rate of the source used in this test is 60 */
        }

        function test_3_changeFrameRate(){
            bmAnim.frameRate = 30;
            compare(bmAnim.frameRate, 30);
        }

        function test_4_initialQuality(){
            compare(bmAnim.quality, 2);     /* quality: LowQuality(0), MediumQuality(1), HighQuality(2)
                                            lottieanimation's initial quality is set to HighQuality
                                            */
        }

        function test_5_changeQuality(){
            bmAnim.quality = 1;             /* change quality to MediumQuality */
            compare(bmAnim.quality, 1);
        }

        function test_6_setEmptySource() {
            compare(bmAnim.status, 2)
            bmAnim.source = ""
            compare(bmAnim.status, 0)
        }
    }
}
